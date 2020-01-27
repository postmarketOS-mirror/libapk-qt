#ifndef H_QTAPK
#define H_QTAPK

#include <QString>
#include <QVector>
#include "QtApkPackage.h"

namespace QtApk {


class DatabasePrivate;

/**
 * @class Database
 * @brief Main interface to interact with Alpine Package Keeper.
 *
 * Instantiate this class first, call open() to start working,
 * do some work, then call close() when finished.
 */
class Database {
public:
    /**
     * @brief The DbOpenFlags enum
     * Used in open() method
     */
    enum DbOpenFlags {
        QTAPK_OPENF_READONLY,   //! open database only for querying info 
        QTAPK_OPENF_READWRITE,  //! open for package manipulation, may need superuser rights
    };

    /**
     * @brief The DbUpdateFlags enum
     * Used in updatePackageIndex() method
     */
    enum DbUpdateFlags {
        QTAPK_UPDATE_DEFAULT = 0,            //! no flags
        QTAPK_UPDATE_ALLOW_UNTRUSTED = 1     //! allow untrusted repository sources
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
     * @return true if everything was OK
     */
    bool upgrade();

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

} // namespace QtApk

#endif
