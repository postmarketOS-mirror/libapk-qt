// SPDX-License-Identifier: GPL-2.0-or-later

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QObject>
#include <QSocketNotifier>
#include <QThread>
#include <QTimer>

#include <QtApk>

#include <cstdlib>
#include <cstdint>
#include <unistd.h>

class EventLoopTestRunner: public QObject
{
    Q_OBJECT
public:
    EventLoopTestRunner(): QObject(nullptr) {
        //
    }
    ~EventLoopTestRunner() {
        db.close();
    }

public:
    bool openDb() {
        if (!db.open(QtApk::Database::QTAPK_OPENF_READWRITE
                     | QtApk::Database::QTAPK_OPENF_ENABLE_PROGRESSFD)) {
            return false;
        }
        qDebug() << "OK: DB was opened!";
        return true;
    }

    // this function runs in the background thread
    void backgroundOps() {
        if (!db.updatePackageIndex(QtApk::Database::QTAPK_UPDATE_ALLOW_UNTRUSTED)) {
            qWarning() << "WARNING: Failed to update DB!";
            db.close();
            return;
        }
        qDebug() << "OK: DB was updated!";
        qDebug() << db.upgradeablePackagesCount() << " packages can be updated.";

        const QString pkgName(QStringLiteral("clang"));
        qDebug() << "=== Install start ===";
        if (!db.add(pkgName)) {
            qWarning() << "Failed to install package " << pkgName;
        }

        qDebug() << "=== Uninstall start ===";
        if (!db.del(pkgName)) {
            qWarning() << "Failed to remove package:" << pkgName;
        }

        // report that we're done
        Q_EMIT backgroundOpsComplete();
    }

Q_SIGNALS:
    void backgroundOpsComplete();

public:
    QtApk::Database db;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QThread bgThread;
    EventLoopTestRunner runner;

    QCommandLineOption root_option(
        QStringLiteral("root"), QStringLiteral("Fake root dir path"),
        QStringLiteral("root"));

    QCommandLineParser parser;
    parser.addOption(root_option);
    parser.addHelpOption();
    parser.process(app);

    if (parser.isSet(root_option)) {
        runner.db.setFakeRoot(parser.value(root_option));
    }

    if (!runner.openDb()) {
        qWarning() << "Failed to open APK DB!";
        return 1;
    }

    // make it so that runner's slots are executed on the bg thread
    runner.moveToThread(&bgThread);

    QSocketNotifier sn(runner.db.progressFd(), QSocketNotifier::Read);
    qDebug() << "apk progress_fd = " << runner.db.progressFd();

    QObject::connect(&sn, &QSocketNotifier::activated, [](int sock) {
        char buf[64] = {0}; // 64 should be enough for everyone
        std::size_t nr = ::read(sock, buf, sizeof(buf)-1);
        if (nr > 0) {
            uint64_t p1, p2;
            ::sscanf(buf, "%lu/%lu", &p1, &p2);
            uint64_t ipercent = 0;
            if (p2) {
                ipercent = 100 * p1 / p2;
            }
            qDebug() << ipercent << "% (" << p1 << " / " << p2 << ")";
        }
    });

    // once operation is complete, shutdown bg thread and also quit mainloop
    QObject::connect(&runner, &EventLoopTestRunner::backgroundOpsComplete, &bgThread, &QThread::quit);
    QObject::connect(&runner, &EventLoopTestRunner::backgroundOpsComplete, &app, QCoreApplication::quit);

    // as soon as mainloop starts, run bg ops
    QTimer::singleShot(0, &runner, &EventLoopTestRunner::backgroundOps);

    // just to be safe, exit in any case in 120 seconds
    QTimer::singleShot(120000, &app, []() {
        qDebug() << "Quitting by timer!";
        QCoreApplication::quit();
    });

    bgThread.start();
    int mainret = app.exec();

    return mainret;
}

#include "test_progressfd.moc"
