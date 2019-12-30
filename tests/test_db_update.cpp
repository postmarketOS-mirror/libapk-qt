#include <QCoreApplication>
#include <QDebug>

#include <QtApk.h>

int main()
{
    int ret = 0;
    QtApk::Database db;

    if (!db.open()) {
        qWarning() << "Failed to open APK DB!";
        return 1;
    }
    qDebug() << "OK: DB was opened!";

    if (!db.update(true)) {
        qWarning() << "WARNING: Failed to update DB!";
    } else {
        qDebug() << "OK: DB was updated!";
    }

    db.close();
    return ret;
}
