// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef H_QTAPKDATABASE_ASYNC
#define H_QTAPKDATABASE_ASYNC

#include <QString>
#include <QVector>
#include "QtApkChangeset.h"
#include "QtApkFlags.h"
#include "QtApkPackage.h"
#include "QtApkRepository.h"
#include "QtApkTransaction.h"

#include "qtapk_exports.h"

namespace QtApk {


class DatabaseAsyncPrivate;

/**
 * @class DatabaseAsync
 * @brief Main interface for asynchronous interactions with
 * Alpine Package Keeper - Alpine Linux package manager.
 *
 * Instantiate this class first, call open() to start working,
 * do some work, then call close() when finished.
 *
 * All method calls in this class are asynchronous, operations are
 * performed in a background thread, so caller is
 * NOT blocked until return.
 */
class QTAPK_EXPORTS DatabaseAsync {
public:
    /**
     * @brief Database default constructor
     */
    DatabaseAsync();

    /**
     * @brief ~Database destructor
     */
    virtual ~DatabaseAsync();

public:
    /**
     * @brief setFakeRoot
     * You can set a path for apk to operate inside. All paths
     * that libapk will access will be relative to that directory.
     * This should be called before open(), otherwise it will
     * have no effect.
     * @param fakeRoot - can be set to path to operate in chroot.
     */
    void setFakeRoot(const QString& fakeRootDir);

    /**
     * @brief fakeRoot
     * Returns currently set fake root.
     * @see setFakeRoot()
     * @return currently set fake root.
     */
    QString fakeRoot() const;

    /**
     * @brief open
     * Open package database. Call this before doing anything
     * @param flags database open flags
     * @return true, if opened OK
     */
    bool open(DbOpenFlags flags = QTAPK_OPENF_READONLY | QTAPK_OPENF_ENABLE_PROGRESSFD);

    /**
     * Close database after finishing all operations
     */
    void close();

    /**
     * @brief isOpen
     * @return true if database is currently open.
     */
    bool isOpen() const;

    /**
     * @brief updatePackageIndex
     * Updates all packages repositories. Database needs to be
     * opened for writing.
     * @param flags update flags, @see DbUpdateFlags
     * @return Transaction object that you can use to control background operation
     */
    Transaction *updatePackageIndex(DbUpdateFlags flags = QTAPK_UPDATE_DEFAULT);

    /**
     * @brief upgradeablePackagesCount
     * Calculates how many packages in the world can be upgraded.
     * Database needs to be opened for reading.
     * @return rough number of packages that can be upgraded
     */
    int upgradeablePackagesCount();

    /**
     * @brief upgrade
     * Upgrade world.
     * Database needs to be opened for writing.
     * @param flags upgrade flags, @see DbUpgradeFlags
     * @param changes set of changes that apk will do to your
     *                system during upgrade, @see Changeset
     * @return Transaction object that you can use to control background operation
     */
    Transaction *upgrade(DbUpgradeFlags flags = QTAPK_UPGRADE_DEFAULT, Changeset *changes = nullptr);

    /**
     * @brief add
     * Add a package to world (install).
     * Database needs to be opened for writing.
     * @param packageNameSpec package name specifier.
     *            Format: "name(@tag)([<>~=]version)"
     * @return Transaction object that you can use to control background operation
     */
    Transaction *add(const QString &packageNameSpec);

    /**
     * @brief del
     * Delete package from world (uninstall).
     * Database needs to be opened for writing.
     * @param packageNameSpec - package name specifier.
     *            Format: "name(@tag)([<>~=]version)"
     * @param flags - flags, @see DbDelFlags
     * @return Transaction object that you can use to control background operation
     */
    Transaction *del(const QString &packageNameSpec, DbDelFlags flags = QTAPK_DEL_DEFAULT);

    /**
     * @brief getInstalledPackages
     * @return a QVector of QtApk::Package: all installed packages
     */
    QVector<Package> getInstalledPackages() const;

    /**
     * @brief getInstalledPackages
     * @return a QVector of QtApk::Package: all available packages
     */
    QVector<Package> getAvailablePackages() const;

private:
    DatabaseAsyncPrivate *d_ptr = nullptr;
    Q_DECLARE_PRIVATE(DatabaseAsync)
    Q_DISABLE_COPY(DatabaseAsync)
};

} // namespace QtApk

#endif
