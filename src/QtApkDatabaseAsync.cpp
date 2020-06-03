// SPDX-License-Identifier: GPL-2.0-or-later

#include "QtApkDatabaseAsync.h"
#include "private/QtApkDatabaseAsync_private.h"

namespace QtApk {


DatabaseAsync::DatabaseAsync()
    : d_ptr(new DatabaseAsyncPrivate(this))
{
}

DatabaseAsync::~DatabaseAsync()
{
    delete d_ptr;
    d_ptr = nullptr;
}

void DatabaseAsync::setFakeRoot(const QString& fakeRootDir)
{
    Q_D(DatabaseAsync);
    d->setFakeRoot(fakeRootDir);
}

QString DatabaseAsync::fakeRoot() const
{
    Q_D(const DatabaseAsync);
    return d->fakeRoot();
}

bool DatabaseAsync::open(DbOpenFlags flags)
{
    Q_D(DatabaseAsync);
    return d->open(flags);
}

void DatabaseAsync::close()
{
    Q_D(DatabaseAsync);
    d->close();
}

bool DatabaseAsync::isOpen() const
{
    Q_D(const DatabaseAsync);
    return d->isOpen();
}

Transaction *DatabaseAsync::updatePackageIndex(DbUpdateFlags flags)
{
    Q_D(DatabaseAsync);
    return d->updatePackageIndex(flags);
}

int DatabaseAsync::upgradeablePackagesCount()
{
    Q_D(DatabaseAsync);
    return d->upgradeablePackagesCount();
}

Transaction *DatabaseAsync::upgrade(DbUpgradeFlags flags, Changeset *changes)
{
    Q_D(DatabaseAsync);
    return d->upgrade(flags, changes);
}

Transaction *DatabaseAsync::add(const QString &packageNameSpec)
{
    Q_D(DatabaseAsync);
    return d->add(packageNameSpec);
}

Transaction *DatabaseAsync::del(const QString &packageNameSpec, DbDelFlags flags)
{
    Q_D(DatabaseAsync);
    return d->del(packageNameSpec, flags);
}

QVector<Package> DatabaseAsync::getInstalledPackages() const
{
    Q_D(const DatabaseAsync);
    return d->getInstalledPackages();
}

QVector<Package> DatabaseAsync::getAvailablePackages() const
{
    Q_D(const DatabaseAsync);
    return d->getAvailablePackages();
}


} // namespace QtApk
