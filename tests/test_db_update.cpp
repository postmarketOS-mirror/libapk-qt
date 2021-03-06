// SPDX-License-Identifier: GPL-2.0-or-later

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>

#include <QtApk>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    int ret = 0;
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

    if (!db.updatePackageIndex(QtApk::QTAPK_UPDATE_ALLOW_UNTRUSTED)) {
        qWarning() << "WARNING: Failed to update DB!";
        ret = 1;
    } else {
        qDebug() << "OK: DB was updated!";
        qDebug() << db.upgradeablePackagesCount() << " packages can be updated.";
    }

    db.close();
    return ret;
}
