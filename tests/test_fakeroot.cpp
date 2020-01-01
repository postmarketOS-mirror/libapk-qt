#include <QCoreApplication>
#include <QDebug>

#include <QtApk.h>

int main()
{
    int ret = 0;
    QtApk::Database db;

    const QString fake_root(QLatin1String("./fr"));
    db.setUseFakeRoot(fake_root);

    if (!db.open(QtApk::Database::QTAPK_OPENF_READWRITE)) {
        qWarning() << "Failed to open APK DB!";
        return 1;
    }
    qDebug() << "OK: DB was opened!";
    qDebug() << "FakeRoot:" << db.fakeRoot();

    //

    db.close();
    return ret;
}
