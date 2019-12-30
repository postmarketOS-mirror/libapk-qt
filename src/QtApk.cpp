#include "QtApk.h"
#include <QDebug>
#include <QLoggingCategory>

extern "C" {
#include "apk_blob.h"
#include "apk_database.h"
#include "apk_defines.h"
#include "apk_version.h"
}

Q_LOGGING_CATEGORY(LOG_QTAPK, "qtapk", QtDebugMsg);

// reimplemented from libapk, because libapk does not export apk_error_str()!!!
static const char *qtapk_error_str(int error)
{
    if (error < 0)
        error = -error;
    switch (error) {
    case ENOKEY:
        return "UNTRUSTED signature";
    case EKEYREJECTED:
        return "BAD signature";
    case EIO:
        return "IO ERROR";
    case EBADMSG:
        return "BAD archive";
    case ENOMSG:
        return "archive does not contain expected data";
    case ENOPKG:
        return "could not find a repo which provides this package (check repositories file and run 'apk update')";
    case ECONNABORTED:
        return "network connection aborted";
    case ECONNREFUSED:
        return "could not connect to server (check repositories file)";
    case ENETUNREACH:
        return "network error (check Internet connection and firewall)";
    case ENXIO:
        return "DNS lookup error";
    case EREMOTEIO:
        return "remote server returned error (try 'apk update')";
    case ETIMEDOUT:
        return "operation timed out";
    case EAGAIN:
        return "temporary error (try again later)";
    case EAPKBADURL:
        return "invalid URL (check your repositories file)";
    case EAPKSTALEINDEX:
        return "package mentioned in index not found (try 'apk update')";
    default:
        return strerror(error);
    }
}

namespace QtApk {


class DatabasePrivate
{
public:
    DatabasePrivate(Database *q)
        : q_ptr(q)
    {
        //
    }

    bool open() {
        int r = dbopen(APK_OPENF_READ | APK_OPENF_WRITE | APK_OPENF_CACHE_WRITE
                       | APK_OPENF_CREATE | APK_OPENF_NO_AUTOUPDATE);
        if (r != 0) {
            dbclose();
            return false;
        }
        return true;
    }

    bool openAndUpdate() {
        // probably bad idea. just calls open with flags that omit
        //  APK_OPENF_NO_AUTOUPDATE. That causes apk_db_add_repository() to
        //  automatically update given repository when opening database.
        int r = dbopen(APK_OPENF_READ | APK_OPENF_WRITE | APK_OPENF_CACHE_WRITE
                       | APK_OPENF_CREATE);
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

    bool update(bool allow_untrusted = false) {
        // apk_repository_update() // is not public??
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
            apk_db_close(db);
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
        while (ipkg) {
            qCDebug(LOG_QTAPK) << "    " << ipkg->pkg->name->name;

            ipkg = list_entry(ipkg->installed_pkgs_list.next,
                              typeof(*ipkg), installed_pkgs_list);
        }
    }

#endif

    Database *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(Database)

    struct apk_db_options db_opts;
    struct apk_database *db = nullptr;
};

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

Database::Database()
    : d_ptr(new DatabasePrivate(this))
{
    //
}

bool Database::open()
{
    Q_D(Database);
    return d->open();
}

void Database::close()
{
    Q_D(Database);
    d->close();
}

bool Database::update(bool allow_untrusted)
{
    Q_D(Database);
    return d->update(allow_untrusted);
}

#ifdef QTAPK_DEVELOPER_BUILD

void Database::print_installed()
{
    Q_D(Database);
    d->print_installed();
}

#endif


} // namespace QtApk
