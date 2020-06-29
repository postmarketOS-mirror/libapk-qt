// SPDX-License-Identifier: GPL-2.0-or-later

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QFile>

#include <QtApk>

static QByteArray readReposFile(const QString &fn)
{
    QByteArray ba;
    QFile f(fn);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open:" << fn;
        return ba;
    }
    ba = f.readAll();
    f.close();
    return ba;
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

    QString fakeRoot;
    if (parser.isSet(root_option)) {
        fakeRoot = parser.value(root_option);
        qputenv("QTAPK_FAKEROOT", fakeRoot.toUtf8());
    } else {
        if (!qEnvironmentVariableIsSet("QTAPK_FAKEROOT")) {
            qWarning() << "Need to be run with --root option or with QTAPK_FAKEROOT env var set!";
            return 1;
        }
    }

    const QString reposFile(QStringLiteral("%1/etc/apk/repositories").arg(fakeRoot));
    QFile f(reposFile);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open:" << reposFile;
        return 1;
    }
    f.close();

    // read - save the same repos and compare file contents before / after

    QByteArray baBefore = readReposFile(reposFile);
    QByteArray baAfter;

    QVector<QtApk::Repository> repos = QtApk::Database::getRepositories();

    if (repos.size() < 1) {
        qWarning() << "Failed to read apk repositories!";
        return 1;
    }

    QtApk::Database::saveRepositories(repos);

    baAfter = readReposFile(reposFile);

    if (baAfter != baBefore) {
        qWarning() << "Content before != content after";
        return 1;
    }

    return 0;
}
