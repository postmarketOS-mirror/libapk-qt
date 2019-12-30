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

private:
    int dbopen(unsigned long open_flags) {
        memset(&db_opts, 0, sizeof(db_opts));
        db_opts.open_flags = open_flags;
        list_init(&db_opts.repository_list);
        apk_atom_init();
        db = static_cast<struct apk_database *>(malloc(sizeof(struct apk_database)));
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

#ifdef QTAPK_DEVELOPER_BUILD

void Database::print_installed()
{
    Q_D(Database);
    d->print_installed();
}

#endif


} // namespace QtApk
