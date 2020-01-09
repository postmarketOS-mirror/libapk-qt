#include <QCoreApplication>
#include <QDebug>

#include <QtApk.h>
#include <QtApkPackage.h>


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QtApk::Database apkdb;

    if (!apkdb.open()) {
        qWarning() << "Failed to open apk database!";
        return 1;
    }

    qDebug() << "OK: Apk database opened!";

    QVector<QtApk::Package> installed = apkdb.getInstalledPackages();

    qDebug() << "OK: Total installed packages: " << installed.size();

    apkdb.close();
    return 0;
}
