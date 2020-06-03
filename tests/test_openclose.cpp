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

    if (!db.open(QtApk::Database::QTAPK_OPENF_READONLY)) {
        qWarning() << "Failed to open APK DB!";
        return 1;
    }
    qDebug() << "OK: DB was opened.";

    db.close();
    return 0;
}
