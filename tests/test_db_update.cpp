#include <QCoreApplication>
#include <QDebug>

#include <QtApk.h>

int main()
{
    int ret = 0;
    QtApk::Database db;

    if (!db.open(QtApk::Database::QTAPK_OPENF_READWRITE)) {
        qWarning() << "Failed to open APK DB!";
        return 1;
    }
    qDebug() << "OK: DB was opened!";

    if (!db.updatePackageIndex(true)) {
        qWarning() << "WARNING: Failed to update DB!";
        ret = 1;
    } else {
        qDebug() << "OK: DB was updated!";
        qDebug() << db.upgradeablePackagesCount() << " packages can be updated.";
    }

    db.close();
    return ret;
}
