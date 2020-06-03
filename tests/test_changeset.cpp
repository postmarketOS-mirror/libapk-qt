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

    if (!db.open(QtApk::Database::QTAPK_OPENF_READWRITE)) {
        qWarning() << "Failed to open APK DB!";
        return 1;
    }
    qDebug() << "OK: DB was opened!";

    QtApk::Changeset changes;
    if (!db.upgrade(QtApk::Database::QTAPK_UPGRADE_SIMULATE, &changes)) {
        qWarning() << "WARNING: Failed to update DB!";
        ret = 1;
    } else {
        qDebug() << "OK: fake upgrade run was OK!";
        const QVector<QtApk::ChangesetItem> &ch = changes.changes();
        qDebug() << "Number of changes:" << ch.size();
        int i = 0;
        for (const QtApk::ChangesetItem &it : ch) {
            qDebug() << i << ":";
            qDebug() << "  " << it.oldPackage.name << " == " << it.newPackage.name;
            qDebug() << "  " << it.oldPackage.version << " > " << it.newPackage.version;
            qDebug() << "  reinstall: " << it.reinstall;
            i++;
        }
    }

    db.close();
    return ret;
}
