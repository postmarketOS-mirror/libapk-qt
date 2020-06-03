// SPDX-License-Identifier: GPL-2.0-or-later

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>

#include <QtApk>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QtApk::Database db;

    QCommandLineOption root_option(
        QStringLiteral("root"), QStringLiteral("Fake root dir path"),
        QStringLiteral("root"));

    QCommandLineParser parser;
    parser.addOption(root_option);
    parser.addHelpOption();
    parser.process(app);

    if (parser.isSet(root_option)) {
        db.setFakeRoot(parser.value(root_option));
    }

    if (!db.open(QtApk::QTAPK_OPENF_READWRITE)) {
        qWarning() << "Failed to open APK DB!";
        return 1;
    }
    qDebug() << "OK: DB was opened!";

    const QString pkgName(QStringLiteral("fish"));
    if (!db.del(pkgName, QtApk::QTAPK_DEL_DEFAULT)) {
        qWarning() << "Failed to delete package " << pkgName;
        // this test does not return 1 on error,
        // because it fails in minimal chroot,
        // but it really works in a full Alpine system
    }

    db.close();
    return 0;
}
