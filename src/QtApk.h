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
     */
    Database();

public:
    /**
     * @brief setUseFakeRoot
     * You can set a path for apk to operate inside. All paths
     * that libapk will access will be relative to that directory.
     * This should be called before open(), otherwise it will
     * have no effect.
     * @param fakeRoot - can be set to path to operate in chroot.
     */
    void setUseFakeRoot(const QString& fakeRootDir);

    /**
     * @brief fakeRoot
     * Returns currently set fake root.
     * @see setUseFakeRoot()
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
     * @param delete_rdepends - delete package and
     *                   everything that depends on it
     * @return true if everything was OK
     */
    bool del(const QString &packageNameSpec, bool delete_rdepends = false);

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
