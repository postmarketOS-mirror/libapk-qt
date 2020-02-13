#include <QCoreApplication>
#include <QObject>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QSocketNotifier>
#include <QTimer>

#include <QtApk.h>

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

    void run() {
        QSocketNotifier sn(db.progressFd(), QSocketNotifier::Read);
        sn_conn = QObject::connect(&sn, &QSocketNotifier::activated,
                                   this, &EventLoopTestRunner::onSocketActivated);

        if (!db.updatePackageIndex(QtApk::Database::QTAPK_UPDATE_ALLOW_UNTRUSTED)) {
            qWarning() << "WARNING: Failed to update DB!";
            db.close();
            return;
        }
        qDebug() << "OK: DB was updated!";
        qDebug() << db.upgradeablePackagesCount() << " packages can be updated.";

        const QString pkgName(QStringLiteral("fish"));
        if (!db.add(pkgName)) {
            qWarning() << "Failed to install package " << pkgName;
        }

        if (!db.del(pkgName)) {
            qWarning() << "Failed to remove package:" << pkgName;
        }

        QObject::disconnect(sn_conn);
        QCoreApplication::quit();
    }

public Q_SLOTS:
    void onSocketActivated(int sock) {
        Q_UNUSED(sock)
        qDebug() << "socket notifier activated! can read!";
    }

public:
    QtApk::Database db;
    QMetaObject::Connection sn_conn;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

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
    qDebug() << "OK: DB was opened!";

    QTimer::singleShot(0, &runner, &EventLoopTestRunner::run);
    QTimer::singleShot(30000, &app, &QCoreApplication::quit);

    int mainret = app.exec();

    return mainret;
}

#include "test_progressfd.moc"
