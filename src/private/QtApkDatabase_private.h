// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef H_QTAPK_DB_PRIV
#define H_QTAPK_DB_PRIV

#include <QString>
#include <QVector>
#include <QLoggingCategory>

#include "../QtApkDatabase.h"
#include "../QtApkPackage.h"
#include "../QtApkRepository.h"
#include "../QtApkChangeset.h"

Q_DECLARE_LOGGING_CATEGORY(LOG_QTAPK)

//  libapk wrapper's forward decls
struct w_apk_database;


namespace QtApk {

class DatabaseAsyncPrivate;  // forward decl, we need to add it as friend

class DatabasePrivate
{
public:
    DatabasePrivate(Database *q);
    ~DatabasePrivate();

    // return read end of the pipe
    int progressFd() const { return progress_fd[0]; }
    bool open(DbOpenFlags flags);
    void close();
    bool isOpen() const;
    bool update(DbUpdateFlags flags);
    bool upgrade(DbUpgradeFlags flags = QTAPK_UPGRADE_DEFAULT,
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
    bool del(const QString &pkgNameSpec, DbDelFlags flags);

    QVector<Package> get_installed_packages() const;
    QVector<Package> get_available_packages() const;

private:
    // Qt's PIMPL members
    Database *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(Database)
    friend class QtApk::DatabaseAsyncPrivate;

    QString fakeRoot; //! if set, libapk will operate inside this
                      //! virtual root dir

    struct w_apk_database *wdb = nullptr;
    int progress_fd[2];
};

} // namespace QtApk

#endif
