// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef H_QTAPKDATABASE
#define H_QTAPKDATABASE

#include <QString>
#include <QVector>
#include "QtApkPackage.h"
#include "QtApkRepository.h"
#include "QtApkChangeset.h"

#include "qtapk_exports.h"

namespace QtApk {


class DatabasePrivate;

/**
 * @class Database
 * @brief Main interface to interact with Alpine Package Keeper.
 *
 * Instantiate this class first, call open() to start working,
 * do some work, then call close() when finished.
 *
 * All method calls in this class are synchronous (caller is
 * blocked until return), so it does not require event loop
 * to be running, and does not have signals.
 */
class QTAPK_EXPORTS Database {
public:
    /**
     * @brief The DbOpenFlags enum
     * Used in open() method
     */
    enum DbOpenFlagEnum {
        QTAPK_OPENF_READONLY = 0x1,           //! open database only for querying info
        QTAPK_OPENF_READWRITE = 0x2,          //! open for package manipulation, may need superuser rights
        QTAPK_OPENF_ENABLE_PROGRESSFD = 0x4,  //! open pipe channel to libapk to receive progress updates
    };
    Q_DECLARE_FLAGS(DbOpenFlags, DbOpenFlagEnum)

    /**
     * @brief The DbUpdateFlags enum
     * Used in updatePackageIndex() method
     */
    enum DbUpdateFlags {
        QTAPK_UPDATE_DEFAULT = 0,            //! no flags
        QTAPK_UPDATE_ALLOW_UNTRUSTED = 1     //! allow untrusted repository sources
    };

    /**
     * @brief The DbUpgradeFlags enum
     * Used in upgrade() method
     */
    enum DbUpgradeFlags {
        QTAPK_UPGRADE_DEFAULT = 0,    //! No flags
        QTAPK_UPGRADE_SIMULATE = 1    //! Do not do a real upgrade, only simulate.
                                      //! Allows to get an "upgrade plan" first
    };

    /**
     * @brief The DbDelFlags enum
     * Used for del() method
     */
    enum DbDelFlags {
        QTAPK_DEL_DEFAULT = 0,    //! no flags
        QTAPK_DEL_RDEPENDS = 1    //! delete package and everything that depends on it
    };

    /**
     * @brief getRepositories
     * Static function that parses /etc/apk/repositories and gets
     * configured repositories list
     * @return configured repositories list
     */
    static QVector<Repository> getRepositories();
    
    /**
     * @brief Database default constructor
     */
    Database();

    /**
     * @brief ~Database destructor
     */
    virtual ~Database();

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
    bool open(DbOpenFlags flags = QTAPK_OPENF_READONLY);
    
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
     * @return true, if all was OK.
     */
    bool updatePackageIndex(DbUpdateFlags flags = QTAPK_UPDATE_DEFAULT);
    
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
     * @return true if everything was OK
     */
    bool upgrade(DbUpgradeFlags flags = QTAPK_UPGRADE_DEFAULT, Changeset *changes = nullptr);

    /**
     * @brief add
     * Add a package to world (install).
     * Database needs to be opened for writing.
     * @param packageNameSpec package name specifier.
     *            Format: "name(@tag)([<>~=]version)"
     * @return true if everything was OK
     */
    bool add(const QString &packageNameSpec);

    /**
     * @brief del
     * Delete package from world (uninstall).
     * Database needs to be opened for writing.
     * @param packageNameSpec - package name specifier.
     *            Format: "name(@tag)([<>~=]version)"
     * @param flags - flags, @see DbDelFlags
     * @return true if everything was OK
     */
    bool del(const QString &packageNameSpec, DbDelFlags flags = QTAPK_DEL_DEFAULT);

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

    /**
     * @brief progressFd
     * libapk has option to write operation progress into some file descriptor.
     * Format for each progress update step is "%u/%u\n"
     * @return file descriptor you can select()/read() from for progress updates
     */
    int progressFd() const;

#ifdef QTAPK_DEVELOPER_BUILD
    // extra debugging functions here, not part of
    //    public library interface
    void print_installed();
#endif

private:
    DatabasePrivate *d_ptr = nullptr;
    Q_DECLARE_PRIVATE(Database)
    Q_DISABLE_COPY(Database)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Database::DbOpenFlags)

} // namespace QtApk

#endif
