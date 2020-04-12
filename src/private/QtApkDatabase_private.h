#ifndef H_QTAPK_DB_PRIV
#define H_QTAPK_DB_PRIV

#include <QString>
#include <QVector>
#include <QLoggingCategory>

#include "../QtApk.h"
#include "../QtApkPackage.h"
#include "../QtApkRepository.h"
#include "../QtApkChangeset.h"

Q_DECLARE_LOGGING_CATEGORY(LOG_QTAPK)

// libapk forward decls
struct apk_database;
struct apk_dependency;
struct apk_dependency_array;
struct apk_package;
struct apk_repository;


namespace QtApk {

class DatabasePrivate
{
public:
    /**
     * Internal struct passed as void* context in delete
     * package callbacks
     **/
    struct del_ctx {
        int recursive_delete;   //! delete rdepends?
        struct apk_dependency_array *world; //! world copy
        int errors;             //! errors counter
    };

    DatabasePrivate(Database *q);
    ~DatabasePrivate();

    // return read end of the pipe
    int progressFd() const { return progress_fd[0]; }
    bool open(Database::DbOpenFlags flags);
    void close();
    bool isOpen() const;
    bool update(Database::DbUpdateFlags flags);
    bool upgrade(Database::DbUpgradeFlags flags = Database::QTAPK_UPGRADE_DEFAULT,
                 Changeset *changes = nullptr);

    /**
     * @brief add
     * @param pkgNameSpec  - package name spec, in format: "name(@tag)([<>~=]version)"
     * @param solver_flags - solver flags, like APK_SOLVERF_UPGRADE | APK_SOLVERF_LATEST,
     *                       optional, default 0
     * @return true on OK
     */
    bool add(const QString &pkgNameSpec, unsigned short solver_flags = 0);

    /**
     * @brief del
     * @param pkgNameSpec
     * @param delete_rdepends - Recursively delete all top-level
     *                          reverse dependencies too
     * @return true on OK
     */
    bool del(const QString &pkgNameSpec, Database::DbDelFlags flags);

    QVector<Package> get_installed_packages() const;
    QVector<Package> get_available_packages() const;

private:
    int dbopen(unsigned long open_flags);
    void dbclose();
    static void repoupdate_progress_cb(void *cb_ctx, size_t p);
    bool repository_update(struct apk_repository *repo, Database::DbUpdateFlags flags);
    bool internal_non_repository_check();
    bool package_name_to_apk_dependency(
            const char *pkgname,
            struct apk_dependency *pdep);

    /**
     * @brief cb_delete_pkg
     * Used as callback and can call itself recursively.
     * Performs an actual deletion.
     * @param pkg0 - actual package to delete
     * @param dep0 - unused
     * @param pkg - unused
     * @param ctx - deletion context, stores information
     *              about current deletion procedure. Points
     *              to @see struct del_ctx.
     */
    static void cb_delete_pkg(struct apk_package *pkg0,
                           struct apk_dependency *dep0,
                           struct apk_package *pkg,
                           void *ctx);

    static int cb_append_package_to_vector(void *hash_item, void *ctx);
    static Package apk_package_to_QtApkPackage(struct apk_package *pkg);

#ifdef QTAPK_DEVELOPER_BUILD
public:
    void print_installed();
#endif

private:
    // Qt's PIMPL members
    Database *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(Database)

    QString fakeRoot; //! if set, libapk will operate inside this
                      //! virtual root dir

    // libapk structs
    struct apk_database *db = nullptr;
    int progress_fd[2];
};

} // namespace QtApk

#endif
