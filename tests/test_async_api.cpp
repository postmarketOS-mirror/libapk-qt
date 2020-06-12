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

    // just to be safe, exit in any case in 180 seconds
    QTimer::singleShot(180000, &app, []() {
        qDebug() << "Quitting by timer!";
        QCoreApplication::exit(100);
    });

    // we will execute 3 transactions in order:
    // 1) update package index
    // 2) install package
    // 3) remove package
    QtApk::Transaction *tr1 = db.updatePackageIndex(QtApk::QTAPK_UPDATE_ALLOW_UNTRUSTED);
    QtApk::Transaction *tr2 = db.add(QLatin1String("clang"));
    QtApk::Transaction *tr3 = db.del(QLatin1String("clang"));

    // receive progress notifications
    QObject::connect(tr1, &QtApk::Transaction::progressChanged, &reportProgress);
    QObject::connect(tr2, &QtApk::Transaction::progressChanged, &reportProgress);
    QObject::connect(tr3, &QtApk::Transaction::progressChanged, &reportProgress);

    // example how to receive error messages
    QObject::connect(tr1, &QtApk::Transaction::errorOccured, [](QString msg) {
        qDebug() << "error:" << msg;
    });

    // once first operation is complete, start second
    QObject::connect(tr1, &QtApk::Transaction::finished, [tr1, tr2]() {
        qDebug() << "transaction 1 complete, start 2nd";
        tr1->deleteLater();
        tr2->start();
    });
    // once second operation is complete, start third
    QObject::connect(tr2, &QtApk::Transaction::finished, [tr2, tr3]() {
        qDebug() << "transaction 2 complete, start 3rd";
        tr2->deleteLater();
        tr3->start();
    });
    // once third operation is complete, quit app
    QObject::connect(tr3, &QtApk::Transaction::finished, [tr3, &app]() {
        qDebug() << "transaction 3 complete, quit after 1 second";
        tr3->deleteLater();
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
