#include "QtApk.h"

extern "C" {
#include "apk_blob.h"
#include "apk_database.h"
#include "apk_defines.h"
#include "apk_version.h"
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


} // namespace QtApk
