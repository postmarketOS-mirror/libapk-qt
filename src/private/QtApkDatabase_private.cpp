// SPDX-License-Identifier: GPL-2.0-or-later

#include "QtApkDatabase_private.h"

#include <QDebug>
#include <QLoggingCategory>
#include <QFile>

#include <unistd.h>

#include "private/libapk_c_wrappers.h"

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(LOG_QTAPK, "qtapk", QtDebugMsg)
#else
Q_LOGGING_CATEGORY(LOG_QTAPK, "qtapk", QtInfoMsg)
#endif

namespace QtApk {

// forwards for some internals
static Package apk_package_to_QtApkPackage(struct apk_package *pkg);
static int cb_append_package_to_vector(void *hash_item, void *ctx);
static void cb_enum_installed(struct apk_package *pkg, void *pv);

// predefined sets of libapk database open flags
static const unsigned long DBOPENF_READONLY = APK_OPENF_READ
        | APK_OPENF_NO_AUTOUPDATE;
static const unsigned long DBOPENF_READWRITE = APK_OPENF_READ
        | APK_OPENF_WRITE | APK_OPENF_CACHE_WRITE | APK_OPENF_CREATE
        | APK_OPENF_NO_AUTOUPDATE;


DatabasePrivate::DatabasePrivate(Database *q)
    : q_ptr(q)
{
    progress_fd[0] = 0; // read end
    progress_fd[1] = 0; // write end
}

DatabasePrivate::~DatabasePrivate()
{
    close();
    w_set_apk_progress_fd(0);
    if (progress_fd[0] != 0) ::close(progress_fd[0]);
    if (progress_fd[1] != 0) ::close(progress_fd[1]);
    progress_fd[0] = 0;
    progress_fd[1] = 0;
}

bool DatabasePrivate::open(DbOpenFlags flags)
{
    // map flags from DbOpenFlags enum to libapk defines
    unsigned long open_flags = 0;

    if (flags & QTAPK_OPENF_READWRITE) {
        open_flags = DBOPENF_READWRITE;
    } else if (flags & QTAPK_OPENF_READONLY) {
        open_flags = DBOPENF_READONLY;
    }

    wdb = w_db_open(open_flags, fakeRoot.toUtf8().constData());
    if (!wdb) {
        return false;
    }

    if (flags & QTAPK_OPENF_ENABLE_PROGRESSFD) {
        if (::pipe(progress_fd) == 0) {
            w_set_apk_progress_fd(progress_fd[1]); // write end
        }
    }
    return true;
}

void DatabasePrivate::close()
{
    w_db_close(wdb);
    wdb = nullptr;
}

bool DatabasePrivate::isOpen() const
{
    if (!wdb) return false;
    return w_db_is_open_complete(wdb->db);
}

bool DatabasePrivate::update(DbUpdateFlags flags)
{
    if (!isOpen()) {
        qCWarning(LOG_QTAPK) << "update: Database is not open!";
        return false;
    }

    bool res = true;
    char progress_buf[64] = {0}; // enough for petabytes...

    // report 0%
    int buflen;
    if (w_get_apk_progress_fd() != 0) {
        buflen = ::snprintf(progress_buf, sizeof(progress_buf), "0/%zu\n",
                            static_cast<size_t>(w_db_get_num_repos(wdb->db)));
        ::write(w_get_apk_progress_fd(), progress_buf, buflen);
    }

    // update each repo
    for (unsigned int iRepo = APK_REPOSITORY_FIRST_CONFIGURED;
         iRepo < w_db_get_num_repos(wdb->db); iRepo++)
    {
        // skip always-configured cache repo
        if (iRepo == APK_REPOSITORY_CACHED) {
            continue;
        }

        qCDebug(LOG_QTAPK) << "Updating: [" << w_db_get_repo_url(wdb->db, iRepo) << "]"
                           << w_db_get_repo_desc(wdb->db, iRepo);

        int r = w_db_repository_update(wdb->db, iRepo, flags & QTAPK_UPDATE_ALLOW_UNTRUSTED ? true : false);
        res = (res && (r == 0));
        if (r != 0) {
            qCWarning(LOG_QTAPK) << "Fetch failed [" << w_db_get_repo_url(wdb->db, iRepo) << "]: "
                                 << w_apk_error_str(r);
        }

        // report progress to the same progress_fd that libapk uses
        //     in exactly the same format as libapk does
        if (w_get_apk_progress_fd() != 0) {
            buflen = ::snprintf(progress_buf, sizeof(progress_buf), "%zu/%zu\n",
                                static_cast<size_t>(iRepo + 1),
                                static_cast<size_t>(w_db_get_num_repos(wdb->db)));
            ::write(w_get_apk_progress_fd(), progress_buf, buflen);
        }
    }
    qCDebug(LOG_QTAPK) << "Updated: " << w_db_get_repo_update_counter(wdb->db)
                       << "; Update errors: " << w_db_get_repo_update_errors(wdb->db);
    qCDebug(LOG_QTAPK) << w_db_get_get_available_packages_count(wdb->db)
                       << " distinct packages available";
    return res;
}

bool DatabasePrivate::upgrade(DbUpgradeFlags flags, Changeset *changes)
{
    if (!isOpen()) {
        qCWarning(LOG_QTAPK) << "upgrade: Database is not open!";
        return false;
    }

    struct apk_changeset *changeset = w_create_apk_changeset();
    int r = 0;
    bool ret = false;
    bool only_simulate = false;

    if (flags & QTAPK_UPGRADE_SIMULATE) only_simulate = true;

    if (w_db_check_world(wdb->db) != 0) {
        qCWarning(LOG_QTAPK) << "upgrade: Missing repository tags. Use "
                                "--force-broken-world to override.";
        w_delete_apk_changeset(changeset);
        return false;
    }

    unsigned short solver_flags = APK_SOLVERF_UPGRADE;

    if (flags & QTAPK_UPGRADE_AVAILABLE) solver_flags |= APK_SOLVERF_AVAILABLE;
    if (flags & QTAPK_UPGRADE_LATEST) solver_flags |= APK_SOLVERF_LATEST;

    // Calculate what will be done
    r = w_apk_solver_solve(wdb->db, solver_flags, changeset);
    if (r == 0) {
        ret = true;

        // fill changeset
        if (changes) {
            changes->changes().clear();
            changes->setNumInstall(w_apk_changeset_get_num_install(changeset));
            changes->setNumRemove(w_apk_changeset_get_num_remove(changeset));
            changes->setNumAdjust(w_apk_changeset_get_num_adjust(changeset));

            // packages:
            for (unsigned iChange = 0; iChange < w_apk_changeset_get_num_changes(changeset); iChange++) {
                struct apk_change *achange = w_apk_changeset_get_change(changeset, iChange);
                ChangesetItem item;
                item.reinstall = w_apk_change_is_reinstall(achange) ? true : false;
                item.oldPackage = apk_package_to_QtApkPackage(w_apk_change_get_old_pkg(achange));
                item.newPackage = apk_package_to_QtApkPackage(w_apk_change_get_new_pkg(achange));
                if (item.newPackage.version != item.oldPackage.version) {
                    changes->changes().append(std::move(item));
                }
            }
        }

        qCDebug(LOG_QTAPK) << "To install:" << (w_apk_changeset_get_num_install(changeset))
                           << "; To remove:" << (w_apk_changeset_get_num_remove(changeset))
                           << "; To adjust:" << (w_apk_changeset_get_num_adjust(changeset));

        // if we are not simulating upgrade, actually install packages
        if (!only_simulate) {
            qCDebug(LOG_QTAPK) << "Installing...";
            r = w_apk_solver_commit_changeset(wdb->db, changeset);
            if (r != 0) {
                ret = false;
                qCWarning(LOG_QTAPK) << "upgrade failed:"
                                     << w_apk_error_str(r);
            }
        }
    } else {
        // solver could not solve a world upgrade.
        // there is an apk_solver_print_errors() function
        // for that case, but we will not use it here.
        qCWarning(LOG_QTAPK) << "upgrade: Failed to resolve world:"
                             << w_apk_error_str(r);
    }

    w_delete_apk_changeset(changeset);
    return ret;
}

/**
 * @brief add
 * @param pkgNameSpec  - package name spec, in format: "name(@tag)([<>~=]version)"
 * @param solver_flags - solver flags, like APK_SOLVERF_UPGRADE | APK_SOLVERF_LATEST,
 *                       optional, default 0
 * @return true on OK
 */
bool DatabasePrivate::add(const QString &pkgNameSpec, unsigned short solver_flags)
{
    if (!isOpen()) {
        qCWarning(LOG_QTAPK) << "add: Database is not open!";
        return false;
    }

    struct w_resolved_apk_dependency resolved_dep;

    const char *const pkgNameSpecC = pkgNameSpec.toUtf8().constData();
    int r = w_apk_add(wdb->db, pkgNameSpecC, solver_flags, &resolved_dep);

    if (r != 0) {
        qCWarning(LOG_QTAPK) << "add: Failed to install package: "
                             << resolved_dep.name << "-" << resolved_dep.version
                             << ": " << w_apk_error_str(r);
    }

    w_resolved_dep_free_strings(&resolved_dep);

    return (r == 0);
}

/**
 * @brief del
 * @param pkgNameSpec
 * @param delete_rdepends - Recursively delete all top-level
 *                          reverse dependencies too
 * @return true on OK
 */
bool DatabasePrivate::del(const QString &pkgNameSpec, DbDelFlags flags)
{
    if (!isOpen()) {
        qCWarning(LOG_QTAPK) << "del: Database is not open!";
        return false;
    }

    const char *const pkgname = pkgNameSpec.toUtf8().constData();
    int r = w_apk_del(wdb->db, pkgname, flags & QTAPK_DEL_RDEPENDS ? true : false);
    if (r) {
        qCWarning(LOG_QTAPK) << "del: failed to delete package:" << pkgNameSpec
                             << ": " << w_apk_error_str(r);
    }

    return (r == 0);
}

QVector<Package> DatabasePrivate::get_installed_packages() const
{
    QVector<Package> ret;

    if (!w_db_has_installed(wdb->db)) {
        return ret;
    }
    w_db_enumerate_installed(wdb->db, cb_enum_installed, reinterpret_cast<void *>(&ret));
    return ret;
}

QVector<Package> DatabasePrivate::get_available_packages() const
{
    QVector<Package> ret;
    ret.reserve(w_db_get_get_available_packages_count(wdb->db));
    int r = w_db_enumerate_available(wdb->db, cb_append_package_to_vector, static_cast<void *>(&ret));
    if (r < 0) {
        qCWarning(LOG_QTAPK) << "Failed to enumerate available packages!";
    }
    return ret;
}


static void cb_enum_installed(struct apk_package *pkg, void *pv)
{
    QVector<Package> *ret = reinterpret_cast<QVector<Package> *>(pv);
    ret->push_back(apk_package_to_QtApkPackage(pkg));
}

static int cb_append_package_to_vector(void *hash_item, void *ctx)
{
    QVector<Package> *vec = static_cast<QVector<Package> *>(ctx);
    struct apk_package *pkg = (struct apk_package *)hash_item;
    vec->append(apk_package_to_QtApkPackage(pkg));
    return 0;
}

static Package apk_package_to_QtApkPackage(struct apk_package *pkg)
{
    Package qpkg;

    if (!pkg) {
        return qpkg;
    }

    qpkg.name = QString::fromUtf8(w_apk_package_get_pkg_name(pkg));
    qpkg.version = QString::fromUtf8(w_apk_package_get_version(pkg));
    qpkg.arch = QString::fromUtf8(w_apk_package_get_arch(pkg));
    qpkg.license = QString::fromUtf8(w_apk_package_get_license(pkg));
    qpkg.origin = QString::fromUtf8(w_apk_package_get_origin(pkg));
    qpkg.maintainer = QString::fromUtf8(w_apk_package_get_maintainer(pkg));
    qpkg.url = QString::fromUtf8(w_apk_package_get_url(pkg));
    qpkg.description = QString::fromUtf8(w_apk_package_get_description(pkg));
    qpkg.commit = QString::fromUtf8(w_apk_package_get_commit(pkg));
    qpkg.filename = QString::fromUtf8(w_apk_package_get_filename(pkg));
    qpkg.buildTime = QDateTime::fromSecsSinceEpoch(w_apk_package_get_buildTime(pkg), Qt::UTC);
    qpkg.installedSize = w_apk_package_get_installedSize(pkg);
    qpkg.size = w_apk_package_get_size(pkg);
    return qpkg;
}

} // namespace QtApk
