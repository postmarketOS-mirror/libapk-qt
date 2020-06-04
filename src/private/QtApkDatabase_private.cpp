// SPDX-License-Identifier: GPL-2.0-or-later

#include "QtApkDatabase_private.h"

#include <QDebug>
#include <QLoggingCategory>
#include <QFile>

extern "C" {
#include "apk_blob.h"
#include "apk_database.h"
#include "apk_defines.h"
#include "apk_version.h"
#include "apk_solver.h"
#include "apk_package.h"
#include "apk_print.h"
}
#include <unistd.h>

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(LOG_QTAPK, "qtapk", QtDebugMsg)
#else
Q_LOGGING_CATEGORY(LOG_QTAPK, "qtapk", QtInfoMsg)
#endif

namespace QtApk {

// internal externals
extern const char *qtapk_error_str(int error);

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

    int r = dbopen(open_flags);
    if (r != 0) {
        dbclose();
        return false;
    }
    if (flags & QTAPK_OPENF_ENABLE_PROGRESSFD) {
        if (::pipe(progress_fd) == 0) {
            ::apk_progress_fd = progress_fd[1]; // write end
        }
    }
    return true;
}

void DatabasePrivate::close()
{
    dbclose();
}

bool DatabasePrivate::isOpen() const
{
    if (!db) return false;
    return (db->open_complete != 0);
}

bool DatabasePrivate::update(DbUpdateFlags flags)
{
    if (!isOpen()) {
        qCWarning(LOG_QTAPK) << "update: Database is not open!";
        return false;
    }

    // apk_repository_update() // is not public?? why, libapk??
    bool res = true;
    char progress_buf[64] = {0}; // enough for petabytes...

    // report 0%
    int buflen = ::snprintf(progress_buf, sizeof(progress_buf), "0/%zu\n",
                            static_cast<size_t>(db->num_repos));
    ::write(::apk_progress_fd, progress_buf, buflen);

    // update each repo
    for (unsigned int i = APK_REPOSITORY_FIRST_CONFIGURED;
         i < db->num_repos; i++)
    {
        // skip always-configured cache repo
        if (i == APK_REPOSITORY_CACHED) {
            continue;
        }
        res = (res && repository_update(&db->repos[i], flags));

        // report progress to the same progress_fd that libapk uses
        //     in exactly the same format as libapk does
        if (::apk_progress_fd != 0) {
            buflen = ::snprintf(progress_buf, sizeof(progress_buf), "%zu/%zu\n",
                                    static_cast<size_t>(i + 1),
                                    static_cast<size_t>(db->num_repos));
            ::write(::apk_progress_fd, progress_buf, buflen);
        }
    }
    qCDebug(LOG_QTAPK) << "Updated: " << db->repo_update_counter
                       << "; Update errors: " << db->repo_update_errors;
    qCDebug(LOG_QTAPK) << db->available.packages.num_items
                       << " distinct packages available";
    return res;
}

bool DatabasePrivate::upgrade(DbUpgradeFlags flags, Changeset *changes)
{
    if (!isOpen()) {
        qCWarning(LOG_QTAPK) << "upgrade: Database is not open!";
        return false;
    }

    struct apk_changeset changeset = {};
    int r = 0;
    bool ret = false;
    bool only_simulate = false;

    if (flags & QTAPK_UPGRADE_SIMULATE) only_simulate = true;

    if (apk_db_check_world(db, db->world) != 0) {
        qCWarning(LOG_QTAPK) << "upgrade: Missing repository tags. Use "
                                "--force-broken-world to override.";
        return false;
    }

    // Calculate what will be done
    unsigned short solver_flags = APK_SOLVERF_UPGRADE;
    r = apk_solver_solve(db, solver_flags, db->world, &changeset);
    if (r == 0) {
        ret = true;

        // fill changeset
        if (changes) {
            changes->changes().clear();
            changes->setNumInstall(changeset.num_install);
            changes->setNumRemove(changeset.num_remove);
            changes->setNumAdjust(changeset.num_adjust);

            // packages:
            for (size_t iChange = 0; iChange < changeset.changes->num; iChange++) {
                struct apk_change *achange = &changeset.changes->item[iChange];
                ChangesetItem item;
                item.reinstall = achange->reinstall ? true : false;
                item.oldPackage = apk_package_to_QtApkPackage(achange->old_pkg);
                item.newPackage = apk_package_to_QtApkPackage(achange->new_pkg);
                if (item.newPackage.version != item.oldPackage.version) {
                    changes->changes().append(std::move(item));
                }
            }
        }

        qCDebug(LOG_QTAPK) << "To install:" << (changeset.num_install)
                           << "; To remove:" << (changeset.num_remove)
                           << "; To adjust:" << (changeset.num_adjust);

        // if we are not simulating upgrade, actually install packages
        if (!only_simulate) {
            qCDebug(LOG_QTAPK) << "Installing...";
            r = apk_solver_commit_changeset(db, &changeset, db->world);
            if (r != 0) {
                ret = false;
                qCWarning(LOG_QTAPK) << "upgrade failed:"
                                     << qtapk_error_str(r);
            }
        }
    } else {
        // solver could not solve a world upgrade.
        // there is an apk_solver_print_errors() function
        // for that case, but we will not use it here.
        qCWarning(LOG_QTAPK) << "upgrade: Failed to resolve world:"
                             << qtapk_error_str(r);
    }

    apk_change_array_free(&changeset.changes);
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

    const char *const pkgname = pkgNameSpec.toUtf8().constData();
    struct apk_dependency dep;
    if (!package_name_to_apk_dependency(pkgname, &dep)) {
        return false;
    }

    int r;
    struct apk_dependency_array *world_copy = NULL;
    apk_dependency_array_copy(&world_copy, db->world);
    apk_deps_add(&world_copy, &dep);
    apk_solver_set_name_flags(dep.name, solver_flags, solver_flags);
    r = apk_solver_commit(db, 0, world_copy);
    // ^^ this may be split into:
    //    - apk_solver_solve
    //    - apk_solver_commit_changeset
    // (see above upgrade() function),
    // if more detailed information is required about
    //    what is happening behind the scenes
    apk_dependency_array_free(&world_copy);

    if (r != 0) {
        qCWarning(LOG_QTAPK) << "add: Failed to install package: "
                             << dep.name->name << "-" << apk_blob_cstr(*dep.version)
                             << ": " << qtapk_error_str(r);
    }

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

    int r = 0;
    const char *const pkgname = pkgNameSpec.toUtf8().constData();
    const apk_blob_t pkgname_blob = APK_BLOB_STR(pkgname);
    struct apk_dependency_array *world_copy = NULL;
    struct apk_changeset changeset = {};
    // struct apk_change *change;
    struct apk_name *name = NULL;
    struct apk_package *pkg = NULL;

    apk_dependency_array_copy(&world_copy, db->world);

    // fill in deletion context
    struct del_ctx dctx = {
        .recursive_delete = (flags & QTAPK_DEL_RDEPENDS) ? 1 : 0,
        .world = world_copy,
        .errors = 0
    };

    // find package name as apk_name in installed packages
    name = static_cast<struct apk_name *>(
                apk_hash_get(&db->available.names, pkgname_blob));

    if (!name) {
        qCWarning(LOG_QTAPK) << "del: No such package: " << pkgname;
        dctx.errors++;
        return false;
    }

    // This time find package (not only name!)
    // This function find first provider of the name
    //      that is actually installed
    pkg = apk_pkg_get_installed(name);
    if (pkg != NULL) {
        // cb_delete_pkg() can call itself recursively to
        //  delete reverse depends, if requested
        cb_delete_pkg(pkg, NULL, NULL, &dctx);
    } else {
        apk_deps_del(&dctx.world, name);
    }

    // solve world
    r = apk_solver_solve(db, 0, world_copy, &changeset);
    if (r == 0) {
        r = apk_solver_commit_changeset(db, &changeset, world_copy);
    }

    // cleanup
    apk_change_array_free(&changeset.changes);
    apk_dependency_array_free(&world_copy);
    return (r == 0);
}

QVector<Package> DatabasePrivate::get_installed_packages() const
{
    QVector<Package> ret;
    struct apk_installed_package *ipkg;
    ipkg = list_entry((&db->installed.packages)->next,
                      struct apk_installed_package,
                      installed_pkgs_list);
    if (!ipkg) {
        qCDebug(LOG_QTAPK) << "No installed packages!";
        return ret;
    }

    if (ipkg->installed_pkgs_list.next == &db->installed.packages) {
        qCDebug(LOG_QTAPK) << "No installed packages!";
        return ret;
    }

    while (&ipkg->installed_pkgs_list != &db->installed.packages) {
        ret.append(apk_package_to_QtApkPackage(ipkg->pkg));
        ipkg = list_entry(ipkg->installed_pkgs_list.next,
                          typeof(*ipkg), installed_pkgs_list);
    }
    return ret;
}

QVector<Package> DatabasePrivate::get_available_packages() const
{
    QVector<Package> ret;
    int r;
    ret.reserve(db->available.packages.num_items);
    r = apk_hash_foreach(&db->available.packages,
                         cb_append_package_to_vector,
                         static_cast<void *>(&ret));
    if (r < 0) {
        qCWarning(LOG_QTAPK) << "Failed to enumerate available packages!";
    }
    return ret;
}

int DatabasePrivate::dbopen(unsigned long open_flags)
{
    struct apk_db_options db_opts;
    memset(&db_opts, 0, sizeof(db_opts));
    db_opts.open_flags = open_flags;
    // apply "fake" root, if set
    if (!fakeRoot.isEmpty()) {
        db_opts.root = strdup(fakeRoot.toUtf8().constData());
        // Do not load scripts when db is opened for writing
        // in fake root mode. Scripts don't like it and always fail
        if (open_flags & APK_OPENF_WRITE) {
            db_opts.open_flags |= APK_OPENF_NO_SCRIPTS;
        }
    }
    list_init(&db_opts.repository_list);
    apk_atom_init();
    db = static_cast<struct apk_database *>(
                malloc(sizeof(struct apk_database)));
    memset(db, 0, sizeof(struct apk_database));
    apk_db_init(db);
    return apk_db_open(db, &db_opts);
}

void DatabasePrivate::dbclose()
{
    if (db) {
        if (db->open_complete) {
            apk_db_close(db);
        }
        free(db);
    }
    db = nullptr;
}

//static
void DatabasePrivate::repoupdate_progress_cb(void *cb_ctx, size_t p)
{
    Q_UNUSED(cb_ctx)
    Q_UNUSED(p)
    // qCDebug(LOG_QTAPK) << "repoupdate_progress_cb: " << p;
    // useless, numbers are too unpredictable
    /*
     * OK: DB was opened!
     * qtapk: Updating: [ http://mirror.yandex.ru/mirrors/alpine/v3.11/main ] v3.11.3-31-g2e6d6d513d
     * fetch http://mirror.yandex.ru/mirrors/alpine/v3.11/main/x86_64/APKINDEX.tar.gz
     * qtapk: repoupdate_progress_cb:  0
     * qtapk: repoupdate_progress_cb:  0
     * qtapk: repoupdate_progress_cb:  262144
     * qtapk: repoupdate_progress_cb:  524288
     * qtapk: repoupdate_progress_cb:  719075
     * qtapk: Updating: [ http://mirror.yandex.ru/mirrors/alpine/v3.11/community ] v3.11.3-30-g2d9c1a116c
     * fetch http://mirror.yandex.ru/mirrors/alpine/v3.11/community/x86_64/APKINDEX.tar.gz
     * qtapk: repoupdate_progress_cb:  0
     * qtapk: repoupdate_progress_cb:  0
     * qtapk: repoupdate_progress_cb:  262144
     * qtapk: repoupdate_progress_cb:  524288
     * qtapk: repoupdate_progress_cb:  786432
     * qtapk: repoupdate_progress_cb:  855159
     * qtapk: Updated:  2 ; Update errors:  0
     * qtapk: 11276  distinct packages available
     * OK: DB was updated!
     * qtapk: To install: 0 ; To remove: 0 ; To adjust: 18
     * 18  packages can be updated.
     * */
}

bool DatabasePrivate::repository_update(struct apk_repository *repo, DbUpdateFlags flags)
{
    int r = 0;
    int verify_flag = (flags & QTAPK_UPDATE_ALLOW_UNTRUSTED)
            ? APK_SIGN_NONE : APK_SIGN_VERIFY;
    constexpr int autoupdate = 1;

    qCDebug(LOG_QTAPK) << "Updating: [" << repo->url << "]"
                       << apk_blob_cstr(repo->description);

    r = apk_cache_download(db, repo, nullptr, verify_flag, autoupdate,
                           repoupdate_progress_cb, static_cast<void *>(this));
    if (r == -EALREADY) {
        return true;
    }
    if (r != 0) {
        qCWarning(LOG_QTAPK) << "Fetch failed [" << repo->url << "]: "
                             << qtapk_error_str(r);
        db->repo_update_errors++;
    } else {
        db->repo_update_counter++;
    }
    return true;
}

bool DatabasePrivate::internal_non_repository_check()
{
    // copied from libapk's add.c
    if (apk_force & APK_FORCE_NON_REPOSITORY)
        return true;
    if (apk_db_cache_active(db))
        return true;
    if (apk_db_permanent(db))
        return true;

    qCWarning(LOG_QTAPK) <<
          "You tried to add a non-repository package to system, \n"
          "but it would be lost on next reboot. Enable package caching \n"
          "(apk cache --help) or use --force-non-repository \n"
          "if you know what you are doing.";
    return false;
}

bool DatabasePrivate::package_name_to_apk_dependency(
        const char *pkgname,
        struct apk_dependency *pdep)
{
    // copied from libapk's add.c, but virtual packages support was removed
    int r;
    if (strstr(pkgname, ".apk") != NULL) {
        struct apk_package *pkg = NULL;
        struct apk_sign_ctx sctx;

        if (!internal_non_repository_check()) {
            return false;
        }

        apk_sign_ctx_init(&sctx, APK_SIGN_VERIFY_AND_GENERATE,
                  NULL, db->keys_fd);
        r = apk_pkg_read(db, pkgname, &sctx, &pkg);
        apk_sign_ctx_free(&sctx);
        if (r != 0) {
            qCWarning(LOG_QTAPK) << pkgname << ": "
                                 << qtapk_error_str(r);
            return false;
        }
        apk_dep_from_pkg(pdep, db, pkg);
    } else {
        apk_blob_t b = APK_BLOB_STR(pkgname);

        apk_blob_pull_dep(&b, db, pdep);
        if (APK_BLOB_IS_NULL(b) || b.len > 0) {
            qCWarning(LOG_QTAPK) << pkgname
                    << " is not a valid world dependency, format is "
                       "name(@tag)([<>~=]version)";
            return false;
        }
    }
    return true;
}

/**
 * @brief cb_delete_pkg
 * Used as callback and can call itself recursively.
 * Performs an actual deletion.
 * @param pkg0 - actual package to delete
 * @param dep0 - unused
 * @param pkg - unused
 * @param ctx - deletion context, stores information
 *              about current deletion procedure. Points
 *              to @see struct del_ctx.
 */
void DatabasePrivate::cb_delete_pkg(struct apk_package *pkg0,
                       struct apk_dependency *dep0,
                       struct apk_package *pkg,
                       void *ctx)
{
    Q_UNUSED(dep0)
    Q_UNUSED(pkg)
    struct del_ctx *pctx = static_cast<struct del_ctx *>(ctx);
    apk_deps_del(&pctx->world, pkg0->name);
    if (pctx->recursive_delete) {
        // call self for each reverse dep
        apk_pkg_foreach_reverse_dependency(
            pkg0, APK_FOREACH_INSTALLED | APK_DEP_SATISFIES,
            cb_delete_pkg, ctx);
    }
}

//static
int DatabasePrivate::cb_append_package_to_vector(void *hash_item, void *ctx)
{
    QVector<Package> *vec = static_cast<QVector<Package> *>(ctx);
    struct apk_package *pkg = (struct apk_package *)hash_item;
    vec->append(apk_package_to_QtApkPackage(pkg));
    return 0;
}

//static
Package DatabasePrivate::apk_package_to_QtApkPackage(struct apk_package *pkg)
{
    Package qpkg;

    if (!pkg) {
        return qpkg;
    }

    qpkg.name = QString::fromUtf8(pkg->name->name);
    if (pkg->version)
        qpkg.version = QString::fromUtf8(apk_blob_cstr(*pkg->version));
    if (pkg->arch)
        qpkg.arch = QString::fromUtf8(apk_blob_cstr(*pkg->arch));
    if (pkg->license)
        qpkg.license = QString::fromUtf8(apk_blob_cstr(*pkg->license));
    if (pkg->origin)
        qpkg.origin = QString::fromUtf8(apk_blob_cstr(*pkg->origin));
    if (pkg->maintainer)
        qpkg.maintainer = QString::fromUtf8(apk_blob_cstr(*pkg->maintainer));
    if (pkg->url)
        qpkg.url = QString::fromUtf8(pkg->url);
    if (pkg->description)
        qpkg.description = QString::fromUtf8(pkg->description);
    if (pkg->commit)
        qpkg.commit = QString::fromUtf8(pkg->commit);
    if (pkg->filename)
        qpkg.filename = QString::fromUtf8(pkg->filename);
    qpkg.buildTime = QDateTime::fromSecsSinceEpoch(pkg->build_time, Qt::UTC);
    qpkg.installedSize = pkg->installed_size;
    qpkg.size = pkg->size;
    return qpkg;
}

#ifdef QTAPK_DEVELOPER_BUILD

void DatabasePrivate::print_installed() {
    struct apk_installed_package *ipkg;
    ipkg = list_entry((&db->installed.packages)->next,
                      struct apk_installed_package,
                      installed_pkgs_list);
    if (!ipkg) {
        qCDebug(LOG_QTAPK) << "No installed packages!";
        return;
    }

    if (ipkg->installed_pkgs_list.next == &db->installed.packages) {
        qCDebug(LOG_QTAPK) << "No installed packages!";
        return;
    }

    qCDebug(LOG_QTAPK) << "Installed packages:";
    while (&ipkg->installed_pkgs_list != &db->installed.packages) {
        qCDebug(LOG_QTAPK) << "    " << ipkg->pkg->name->name
                           << "  " << apk_blob_cstr(*ipkg->pkg->version)
                           << " / " << apk_blob_cstr(*ipkg->pkg->arch);

        ipkg = list_entry(ipkg->installed_pkgs_list.next,
                          typeof(*ipkg), installed_pkgs_list);
    }
}

#endif


} // namespace QtApk
