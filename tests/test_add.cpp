#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>

#include <QtApk.h>

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
        db.setUseFakeRoot(parser.value(root_option));
    }

    if (!db.open(QtApk::Database::QTAPK_OPENF_READWRITE)) {
        qWarning() << "Failed to open APK DB!";
        return 1;
    }
    qDebug() << "OK: DB was opened!";

    if (!db.updatePackageIndex(true)) {
        qWarning() << "WARNING: Failed to update DB!";
        db.close();
        return 1;
    }
    qDebug() << "OK: DB was updated!";
    qDebug() << db.upgradeablePackagesCount() << " packages c"
                                                 "an be updated.";

    const QString pkgName(QStringLiteral("fish"));
    if (!db.add(pkgName)) {
        qWarning() << "Failed to install package " << pkgName;
    }

    db.close();
    return 0;
}
