#include <QCoreApplication>
#include <QDebug>

#include <QtApk.h>

int main()
{
    int ret = 0;
    QtApk::Database db;

    if (db.open()) {
        qDebug() << "OK: DB was opened/created";
        db.close();
    } else {
        qWarning() << "Failed to open APK DB!";
        ret = 1;
    }

    return ret;
}
