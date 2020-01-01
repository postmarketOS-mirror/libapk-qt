#ifndef H_QTAPK
#define H_QTAPK

#include <QObject>

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
    enum DbOpenFlags {
        QTAPK_OPENF_READONLY,   //! open database only for querying info 
        QTAPK_OPENF_READWRITE,  //! open for package manipulation, may need superuser rights
    };
    
    /**
     * @brief Database default constructor
     * @param fakeRoot can be set to operate on some chroot.
     */
    Database(const QString& fakeRoot = QString());

public:
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
     * @param allow_untrusted Ignore signature checking
     * @return true, if all was OK.
     */
    bool updatePackageIndex(bool allow_untrusted = false);
    
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
     * @param packageNameSpec package name specifier
     * @return true if everything was OK
     */
    bool add(const QString &packageNameSpec);

    /**
     * @brief del
     * Delete package from world (uninstall).
     * Database needs to be opened for writing.
     * @param packageNameSpec package name specifier
     * @return true if everything was OK
     */
    bool del(const QString &packageNameSpec);

#ifdef QTAPK_DEVELOPER_BUILD
    // extra debugging functions here, not part of
    //    public library interface
    void print_installed();
#endif

private:
    DatabasePrivate *d_ptr = nullptr;
    Q_DECLARE_PRIVATE(Database)
};

} // namespace QtApk

#endif
