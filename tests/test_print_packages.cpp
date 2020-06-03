// SPDX-License-Identifier: GPL-2.0-or-later

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>

#include <QtApk>

static void printPkgs(const char *comment, const QVector<QtApk::Package> &pkgs)
{
    qDebug() << comment << ": " << pkgs.size() << "total";
    for (const QtApk::Package &p: pkgs) {
        qDebug().nospace() << p.name << "-" << p.version << " / "
                           << p.arch << " {" << p.license << "}";
        qDebug().nospace() << "    " << "Url: " << p.url;
        qDebug().nospace() << "    " << "Maintainer: " << p.maintainer << " (commit "
                           << p.commit << ")";
        // file is usually an empty string :(
        if (!p.filename.isEmpty())
            qDebug().nospace() << "    " << "file: " << p.filename;
        qDebug().nospace() << "    " << "Size: " << p.size
                           << "; Installed size: " << p.installedSize;
        qDebug().nospace() << "    " << "Built on: " << p.buildTime;
    }
}

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

    if (!db.open(QtApk::QTAPK_OPENF_READONLY)) {
        qWarning() << "Failed to open APK DB!";
        return 1;
    }

    const QVector<QtApk::Package> installed = db.getInstalledPackages();
    const QVector<QtApk::Package> all = db.getAvailablePackages();

    qDebug() << "\n" << "====================\n";
    printPkgs("Installed", installed);

    qDebug() << "\n" << "====================\n";
    printPkgs("All", all);

    db.close();
    return 0;
}
