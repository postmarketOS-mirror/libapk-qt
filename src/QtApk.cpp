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
        memset(&opts, 0, sizeof(opts));
        opts.open_flags = APK_OPENF_READ | APK_OPENF_WRITE | APK_OPENF_CREATE | APK_OPENF_NO_AUTOUPDATE;
        list_init(&opts.repository_list);
        apk_atom_init();
        db = static_cast<struct apk_database *>(malloc(sizeof(struct apk_database)));
        apk_db_init(db);
        int r = apk_db_open(db, &opts);
        if (r != 0) {
            free(db);
            db = nullptr;
            return false;
        }
        return true;
    }

    void close() {
        if (db) {
            apk_db_close(db);
            free(db);
            db = nullptr;
        }
        return;
    }

#ifdef QTAPK_DEVELOPER_BUILD

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

    struct apk_db_options opts;
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
