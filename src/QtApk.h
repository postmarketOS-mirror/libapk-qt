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
     * Default constructor
     * @param fakeRoot can be set to open database in some chroot.
     */
    Database(const QString& fakeRoot = QString());

public:
    /**
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
     * @return true if database is currently open.
     */
    bool isOpen() const;

    /**
     * Updates all packages repositories. Database needs to be
     * opened for writing.
     * @param allow_untrusted Ignore signature checking
     * @return true, if all was OK.
     */
    bool updatePackageIndex(bool allow_untrusted = false);
    
    /**
     * Calculates how many packages in the world can be upgraded.
     * Database needs to be opened for reading.
     * @return number of packages that can be upgraded
     */
    int upgradeablePackagesCount();

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
