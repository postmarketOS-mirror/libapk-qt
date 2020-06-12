// SPDX-License-Identifier: GPL-2.0-or-later

#include "private/QtApkDatabase_private.h"
#include "QtApkDatabase.h"
#include <QDebug>
#include <QLoggingCategory>
#include <QFile>


namespace QtApk {


QVector<Repository> Database::getRepositories()
{
    QVector<Repository> ret;
    const QString fakeRoot = qEnvironmentVariable("QTAPK_FAKEROOT", QStringLiteral("/"));
    const QString reposFile(QStringLiteral("%1/etc/apk/repositories").arg(fakeRoot));
    QFile f(reposFile);
    if (!f.open(QIODevice::ReadOnly)) {
        qCWarning(LOG_QTAPK) << "Failed to open:" << reposFile;
        return ret;
    }
    const QString comment(QLatin1String("/etc/apk/repositories"));
    while (!f.atEnd()) {
        QString line = QString::fromUtf8(f.readLine(1024)).trimmed();
        bool enabled = true;
        if (line.startsWith(QLatin1Char('#'))) {
            enabled = false;
            line = line.mid(1);
        }
        ret.append(Repository(line, comment, enabled));
    }
    f.close();
    return ret;
}

Database::Database()
    : d_ptr(new DatabasePrivate(this))
{
    // allow to override fakeroot from environment variable
    if (qEnvironmentVariableIsSet("QTAPK_FAKEROOT")) {
        qCDebug(LOG_QTAPK) << "Overriding fakeroot from env QTAPK_FAKEROOT:"
                           << qgetenv("QTAPK_FAKEROOT");
        setFakeRoot(qEnvironmentVariable("QTAPK_FAKEROOT"));
    }
}

Database::~Database()
{
    delete d_ptr;
    d_ptr = nullptr;
}

void Database::setFakeRoot(const QString &fakeRootDir)
{
    Q_D(Database);
    if (isOpen()) return;
    d->fakeRoot = fakeRootDir;
}

QString Database::fakeRoot() const
{
    Q_D(const Database);
    return d->fakeRoot;
}

bool Database::open(DbOpenFlags flags)
{
    Q_D(Database);
    return d->open(flags);
}

void Database::close()
{
    Q_D(Database);
    d->close();
}

bool Database::isOpen() const
{
    Q_D(const Database);
    return d->isOpen();
}


bool Database::updatePackageIndex(DbUpdateFlags flags)
{
    Q_D(Database);
    return d->update(flags);
}

int Database::upgradeablePackagesCount()
{
    Q_D(Database);
    int totalUpgrades = 0;
    Changeset changes;

    if (d->upgrade(QTAPK_UPGRADE_SIMULATE, &changes)) {
        // Don't count packages to remove
        totalUpgrades = changes.numInstall() + changes.numAdjust();
    }
    return totalUpgrades;
}

bool Database::upgrade(DbUpgradeFlags flags, Changeset *changes)
{
    Q_D(Database);
    return d->upgrade(flags, changes);
}

bool Database::add(const QString &packageNameSpec)
{
    Q_D(Database);
    return d->add(packageNameSpec);
}

bool Database::del(const QString &packageNameSpec, DbDelFlags flags)
{
    Q_D(Database);
    return d->del(packageNameSpec, flags);
}

QVector<Package> Database::getInstalledPackages() const
{
    Q_D(const Database);
    return d->get_installed_packages();
}

QVector<Package> Database::getAvailablePackages() const
{
    Q_D(const Database);
    return d->get_available_packages();
}

int Database::progressFd() const
{
    Q_D(const Database);
    return d->progressFd();
}

#ifdef QTAPK_DEVELOPER_BUILD

void Database::print_installed()
{
    Q_D(Database);
    d->print_installed();
}

#endif


} // namespace QtApk
