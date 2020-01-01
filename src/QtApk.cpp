#include "QtApk.h"
#include <QDebug>
#include <QLoggingCategory>

extern "C" {
#include "apk_blob.h"
#include "apk_database.h"
#include "apk_defines.h"
#include "apk_version.h"
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
    static const unsigned long DBOPENF_READONLY = APK_OPENF_READ
            | APK_OPENF_NO_AUTOUPDATE;
    static const unsigned long DBOPENF_READWRITE = APK_OPENF_READ
            | APK_OPENF_WRITE | APK_OPENF_CACHE_WRITE | APK_OPENF_CREATE
            | APK_OPENF_NO_AUTOUPDATE;
    
    DatabasePrivate(Database *q)
        : q_ptr(q)
    {
    }
    
    bool open(unsigned long open_flags) {
        int r = dbopen(open_flags);
        if (r != 0) {
            dbclose();
            return false;
        }
        return true;
    }

    void close() {
        dbclose();
        return;
    }
    
    bool isOpen() const {
        if (!db) return false;
        return (db->open_complete != 0);
    }

    bool update(bool allow_untrusted = false) {
        if (!isOpen()) {
            qCWarning(LOG_QTAPK) << "update: Database is not open!";
            return false;
        }
        // apk_repository_update() // is not public?? why, libapk??
        // update each repo
        bool res = true;
        for (unsigned int i = APK_REPOSITORY_FIRST_CONFIGURED; i < db->num_repos; i++) {
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

private:
    int dbopen(unsigned long open_flags) {
        memset(&db_opts, 0, sizeof(db_opts));
        db_opts.open_flags = open_flags;
        list_init(&db_opts.repository_list);
        apk_atom_init();
        db = static_cast<struct apk_database *>(malloc(sizeof(struct apk_database)));
        memset(db, 0, sizeof(struct apk_database));
        apk_db_init(db);
        return apk_db_open(db, &db_opts);
    }

    void dbclose() {
        if (db) {
            if (db->open_complete) {
                apk_db_close(db);
            }
            free(db);
        }
        db = nullptr;
    }

    bool repository_update(struct apk_repository *repo, bool allow_untrusted) {
        int r = 0;
        int verify_flag = allow_untrusted ? APK_SIGN_NONE : APK_SIGN_VERIFY;
        constexpr int autoupdate = 1;

        qCDebug(LOG_QTAPK) << "Updating: [" << repo->url << "]" << repo->description.ptr;

        r = apk_cache_download(db, repo, nullptr, verify_flag, autoupdate, nullptr, nullptr);
        if (r == -EALREADY) {
            return true;
        }
        if (r != 0) {
            qCWarning(LOG_QTAPK) << "Fetch failed [" << repo->url << "]: " << qtapk_error_str(r);
            db->repo_update_errors++;
        } else {
            db->repo_update_counter++;
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

    Database *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(Database)
    
    QString fakeRoot;

    struct apk_db_options db_opts;
    struct apk_database *db = nullptr;
};

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

Database::Database(const QString &fakeRoot)
    : d_ptr(new DatabasePrivate(this))
{
    d_ptr->fakeRoot = fakeRoot;
}

bool Database::open(DbOpenFlags flags)
{
    Q_D(Database);
    // map flags from API enum to libapk defines
    unsigned long openf = 0;
    switch (flags) {
    case QTAPK_OPENF_READONLY: openf = DatabasePrivate::DBOPENF_READONLY; break;
    case QTAPK_OPENF_READWRITE: openf = DatabasePrivate::DBOPENF_READWRITE; break;
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
    return 0;
}

#ifdef QTAPK_DEVELOPER_BUILD

void Database::print_installed()
{
    Q_D(Database);
    d->print_installed();
}

#endif


} // namespace QtApk
