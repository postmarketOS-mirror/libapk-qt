// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef H_QTAPK_DATABASE_ASYNC_PRIVATE
#define H_QTAPK_DATABASE_ASYNC_PRIVATE

#include <QObject>
#include <QThread>
#include <QSocketNotifier>

#include "../QtApkDatabaseAsync.h"
#include "QtApkDatabase_private.h"

namespace QtApk {

class BgThreadExecutor;
class TransactionPrivate;

class DatabaseAsyncPrivate: public QObject
{
    Q_OBJECT
public:
    DatabaseAsyncPrivate(DatabaseAsync *q);
    ~DatabaseAsyncPrivate();

    void setFakeRoot(const QString& fakeRootDir);
    QString fakeRoot() const;
    bool open(DbOpenFlags flags = QTAPK_OPENF_READONLY | QTAPK_OPENF_ENABLE_PROGRESSFD);
    void close();
    bool isOpen() const;
    Transaction *updatePackageIndex(DbUpdateFlags flags = QTAPK_UPDATE_DEFAULT);
    int upgradeablePackagesCount();
    Transaction *upgrade(DbUpgradeFlags flags = QTAPK_UPGRADE_DEFAULT, Changeset *changes = nullptr);
    Transaction *add(const QString &packageNameSpec);
    Transaction *del(const QString &packageNameSpec, DbDelFlags flags = QTAPK_DEL_DEFAULT);
    QVector<Package> getInstalledPackages() const;
    QVector<Package> getAvailablePackages() const;

protected:
    bool checkCanStart();
    Transaction *createReturnTransaction(TransactionPrivate *trp);
    void onSocketNotifierActivated(int sock);
    void onTransactionDestroyed(QObject *obj);

public:
    DatabaseAsync *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(DatabaseAsync)
    BgThreadExecutor *executor = nullptr;
    DatabasePrivate *dbpriv = nullptr;
    QSocketNotifier *socketNotifier = nullptr;
    QThread bgThread;
};


} // namespace QtApk

#endif
