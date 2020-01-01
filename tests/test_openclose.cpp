#include <QCoreApplication>
#include <QDebug>

#include <QtApk.h>

int main()
{
    int ret = 0;
    QtApk::Database db;

    if (!db.open(QtApk::Database::QTAPK_OPENF_READONLY)) {
        qWarning() << "Failed to open APK DB!";
        return 1;
    }
    qDebug() << "OK: DB was opened.";

    db.print_installed();

    db.close();
    return ret;
}
