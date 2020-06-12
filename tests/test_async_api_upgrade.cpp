// SPDX-License-Identifier: GPL-2.0-or-later

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QObject>
#include <QTimer>

#include <QtApk>

static void reportProgress(float percent)
{
    qDebug() << "app: percent:" << percent;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCommandLineOption root_option(
        QStringLiteral("root"), QStringLiteral("Fake root dir path"),
        QStringLiteral("root"));

    QCommandLineParser parser;
    parser.addOption(root_option);
    parser.addHelpOption();
    parser.process(app);

    QtApk::DatabaseAsync db;

    if (parser.isSet(root_option)) {
        db.setFakeRoot(parser.value(root_option));
    }

    if (!db.open(QtApk::QTAPK_OPENF_READWRITE | QtApk::QTAPK_OPENF_ENABLE_PROGRESSFD)) {
        qWarning() << "Failed to open APK DB!";
        return 1;
    }

    // just to be safe, exit in any case after some time
    constexpr int TIMEOUT_MINS = 15;
    QTimer::singleShot(TIMEOUT_MINS * 60 * 1000, &app, []() {
        qDebug() << "Quitting by timer!" << TIMEOUT_MINS;
        QCoreApplication::exit(100);
    });

    // we will execute 1 transaction
    QtApk::Transaction *tr1 = db.upgrade();

    // receive progress notifications
    QObject::connect(tr1, &QtApk::Transaction::progressChanged, &reportProgress);

    // example how to receive error messages
    QObject::connect(tr1, &QtApk::Transaction::errorOccured, [](QString msg) {
        qDebug() << "error:" << msg;
    });

    // once operation is complete, quit app
    QObject::connect(tr1, &QtApk::Transaction::finished, [tr1, &app]() {
        qDebug() << "transaction complete, quit after 1 second";
        tr1->deleteLater();
        // 1 second delay allows to run all other signal-slot connections
        // and otehr events in queue, like above deleteLater() call
        // exit after 1 second
        QTimer::singleShot(1000, &app, &QCoreApplication::quit);
    });

    // don't forget to actually start first transaction
    tr1->start();

    int mainret = app.exec();

    qDebug() << "mainloop exited with code:" << mainret;

    db.close();
    return mainret;
}
