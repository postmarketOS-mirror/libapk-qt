#include "QtApk.h"
#include <QDebug>
#include <QLoggingCategory>

extern "C" {
#include "apk_blob.h"
#include "apk_database.h"
#include "apk_defines.h"
#include "apk_version.h"
#include "apk_solver.h"
#include "apk_package.h"
}

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(LOG_QTAPK, "qtapk", QtDebugMsg);
#else
Q_LOGGING_CATEGORY(LOG_QTAPK, "qtapk", QtInfoMsg);
#endif

namespace QtApk {


// internal externals
extern const char *qtapk_error_str(int error);


class DatabasePrivate
{
public:
    // predefined sets of libapk database open flags
    static const unsigned long DBOPENF_READONLY = APK_OPENF_READ
            | APK_OPENF_NO_AUTOUPDATE;
    static const unsigned long DBOPENF_READWRITE = APK_OPENF_READ
            | APK_OPENF_WRITE | APK_OPENF_CACHE_WRITE | APK_OPENF_CREATE
            | APK_OPENF_NO_AUTOUPDATE;
    
    DatabasePrivate(Database *q)
        : q_ptr(q)
    {
    }
    
    bool open(unsigned long open_flags)
    {
        int r = dbopen(open_flags);
        if (r != 0) {
            dbclose();
            return false;
        }
        return true;
    }

    void close()
    {
        dbclose();
        return;
    }
    
    bool isOpen() const
    {
        if (!db) return false;
        return (db->open_complete != 0);
    }

    bool update(bool allow_untrusted = false)
    {
        if (!isOpen()) {
            qCWarning(LOG_QTAPK) << "update: Database is not open!";
            return false;
        }
        // apk_repository_update() // is not public?? why, libapk??
        // update each repo
        bool res = true;
        for (unsigned int i = APK_REPOSITORY_FIRST_CONFIGURED;
             i < db->num_repos; i++)
        {
            // skip always-configured cache repo
            if (i == APK_REPOSITORY_CACHED) {
                continue;
            }
            res = (res && repository_update(&db->repos[i], allow_untrusted));
        }
        qCDebug(LOG_QTAPK) << "Updated: " << db->repo_update_counter
                           << "; Update errors: " << db->repo_update_errors;
        qCDebug(LOG_QTAPK) << db->available.packages.num_items
                           << " distinct packages available";
        return res;
    }

    bool upgrade(int *num_install, int *num_remove, int *num_adjust,
                 bool only_simulate)
    {
        if (!isOpen()) {
            qCWarning(LOG_QTAPK) << "count_upgrades: Database is not open!";
            return false;
        }

        struct apk_changeset changeset = {};
        int r;
        bool ret = false;

        if (apk_db_check_world(db, db->world) != 0) {
            qCWarning(LOG_QTAPK) << "Missing repository tags. Use "
                                    "--force-broken-world to override.";
            return false;
        }

        // Calculate what will be done
        unsigned short solver_flags = APK_SOLVERF_UPGRADE;
        r = apk_solver_solve(db, solver_flags, db->world, &changeset);
        if (r == 0) {
            ret = true;
            *num_install = changeset.num_install;
            *num_remove = changeset.num_remove;
            *num_adjust = changeset.num_adjust;
            qCDebug(LOG_QTAPK) << "To install:" << (*num_install)
                               << "; To remove:" << (*num_remove)
                               << "; To adjust:" << (*num_adjust);

            // if we are not simulating upgrade, actually install packages
            if (!only_simulate) {
                qCDebug(LOG_QTAPK) << "Installing...";
                r = apk_solver_commit_changeset(db, &changeset, db->world);
                if (r != 0) {
                    ret = false;
                    qCWarning(LOG_QTAPK) << "Upgrade failed:"
                                         << qtapk_error_str(r);
                }
            }
        } else {
            // solver could not solve a world upgrade.
            // there is an apk_solver_print_errors() function
            // for that case, but we will not use it here.
            qCWarning(LOG_QTAPK) << "Failed to resolve world:"
                                 << qtapk_error_str(r);
            num_install = num_remove = num_adjust = 0;
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
    bool add(const QString &pkgNameSpec, unsigned short solver_flags = 0)
    {
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
                                 << dep.name->name << "-" << dep.version->ptr
                                 << ": " << qtapk_error_str(r);
        }

        return (r == 0);
    }

private:
    int dbopen(unsigned long open_flags)
    {
        memset(&db_opts, 0, sizeof(db_opts));
        db_opts.open_flags = open_flags;
        // apply "fake" root, if set
        if (!fakeRoot.isEmpty()) {
            db_opts.root = strdup(fakeRoot.toUtf8().constData());
        }
        list_init(&db_opts.repository_list);
        apk_atom_init();
        db = static_cast<struct apk_database *>(
                    malloc(sizeof(struct apk_database)));
        memset(db, 0, sizeof(struct apk_database));
        apk_db_init(db);
        return apk_db_open(db, &db_opts);
    }

    void dbclose()
    {
        if (db) {
            if (db->open_complete) {
                apk_db_close(db);
            }
            free(db);
        }
        db = nullptr;
    }

    bool repository_update(struct apk_repository *repo, bool allow_untrusted)
    {
        int r = 0;
        int verify_flag = allow_untrusted ? APK_SIGN_NONE : APK_SIGN_VERIFY;
        constexpr int autoupdate = 1;

        qCDebug(LOG_QTAPK) << "Updating: [" << repo->url << "]"
                           << repo->description.ptr;

        r = apk_cache_download(db, repo, nullptr, verify_flag, autoupdate,
                               nullptr, nullptr);
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

    bool internal_non_repository_check()
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

    bool package_name_to_apk_dependency(
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

#ifdef QTAPK_DEVELOPER_BUILD
public:

    void print_installed() {
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
                               << "  " << ipkg->pkg->version->ptr
                               << " / " << ipkg->pkg->arch->ptr;

            ipkg = list_entry(ipkg->installed_pkgs_list.next,
                              typeof(*ipkg), installed_pkgs_list);
        }
    }

#endif

    // Qt's PIMPL members
    Database *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(Database)
    
    //
    QString fakeRoot; //! if set, libapk will operate inside this
                      //! virtual root dir

    // libapk structs
    struct apk_db_options db_opts;
    struct apk_database *db = nullptr;
};

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

Database::Database()
    : d_ptr(new DatabasePrivate(this))
{
}

void Database::setUseFakeRoot(const QString &fakeRootDir)
{
    Q_D(Database);
    if (isOpen()) return;
    d->fakeRoot = fakeRootDir;
}

QString Database::fakeRoot() const
{
    Q_D(const Database);
    return d->fakeRoot;
}

bool Database::open(DbOpenFlags flags)
{
    Q_D(Database);
    // map flags from DbOpenFlags enum to libapk defines
    unsigned long openf = 0;
    switch (flags) {
    case QTAPK_OPENF_READONLY:
        openf = DatabasePrivate::DBOPENF_READONLY;
        break;
    case QTAPK_OPENF_READWRITE:
        openf = DatabasePrivate::DBOPENF_READWRITE;
        break;
    }
    return d->open(openf);
}

void Database::close()
{
    Q_D(Database);
    d->close();
}

bool Database::isOpen() const
{
    Q_D(const Database);
    return d->isOpen();
}


bool Database::updatePackageIndex(bool allow_untrusted)
{
    Q_D(Database);
    return d->update(allow_untrusted);
}

int Database::upgradeablePackagesCount()
{
    Q_D(Database);
    int total_upgrades = 0, num_install = 0,
            num_remove = 0, num_adjust = 0;
    constexpr bool only_simulate = true;
    if (d->upgrade(&num_install, &num_remove, &num_adjust,
                   only_simulate)) {
        // Don't count packages to remove
        total_upgrades = num_install + num_adjust;
    }
    return total_upgrades;
}

bool Database::upgrade()
{
    Q_D(Database);
    int num_install = 0, num_remove = 0, num_adjust = 0;
    constexpr bool only_simulate = false;
    return d->upgrade(&num_install, &num_remove,
                      &num_adjust, only_simulate);
}

bool Database::add(const QString &packageNameSpec)
{
    Q_D(Database);
    return d->add(packageNameSpec);
}

bool Database::del(const QString &packageNameSpec)
{
    Q_UNUSED(packageNameSpec)
    return false;
}

#ifdef QTAPK_DEVELOPER_BUILD

void Database::print_installed()
{
    Q_D(Database);
    d->print_installed();
}

#endif


} // namespace QtApk
