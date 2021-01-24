// SPDX-License-Identifier: GPL-2.0-or-later

#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>

#include <QObject>
#include <QLoggingCategory>

#include "QtApkDatabaseAsync_private.h"
#include "../QtApkTransaction.h"
#include "QtApkTransaction_private.h"

Q_DECLARE_LOGGING_CATEGORY(LOG_QTAPK)

namespace QtApk {


/**
 * @brief The BgThreadExecutor class
 * Small worker class whose slots will be executed
 * in the background thread.
 */
class BgThreadExecutor: public QObject
{
    Q_OBJECT
public:
    BgThreadExecutor(): QObject(nullptr) { }
    ~BgThreadExecutor() override { }

    void connectCurrentTransaction()
    {
        if (!currentTransaction) {
            return;
        }
        // now also create the connections between bg thread and current Transaction
        // we cannot do it before, otherwise multiple connections can be triggered
        // during our emit, if there are multiple transactions created
        // because all of them would be connected to our single slot
        QObject::connect(this, &BgThreadExecutor::operationFinished,
                         currentTransaction, &Transaction::finished, Qt::QueuedConnection);
        QObject::connect(this, &BgThreadExecutor::operationErrorOccured,
                         currentTransaction, &Transaction::errorOccured, Qt::QueuedConnection);
    }

    void disconnectCurrentTransaction()
    {
        if (!currentTransaction) {
            return;
        }
        QObject::disconnect(this, &BgThreadExecutor::operationFinished,
                            currentTransaction, &Transaction::finished);
        QObject::disconnect(this, &BgThreadExecutor::operationErrorOccured,
                            currentTransaction, &Transaction::errorOccured);
    }

public Q_SLOTS:

    // this function runs in the background thread
    void startUpdatePackageIndex(void *ct, DbUpdateFlags flags)
    {
        currentTransaction = reinterpret_cast<Transaction *>(ct);
        isBusy = true;
        bool ok = dbpriv->update(flags);
        isBusy = false;
        connectCurrentTransaction();
        if (!ok) {
            Q_EMIT operationErrorOccured(tr("Update package index failed"));
        }
        Q_EMIT operationFinished();
        disconnectCurrentTransaction();
    }

    // this function runs in the background thread
    void startUpgradeSystem(void *ct, DbUpgradeFlags flags, void *pvChangeset)
    {
        currentTransaction = reinterpret_cast<Transaction *>(ct);
        Changeset *changeset = reinterpret_cast<Changeset *>(pvChangeset);
        isBusy = true;
        bool ok = dbpriv->upgrade(flags, changeset);
        isBusy = false;
        connectCurrentTransaction();
        if (!ok) {
            Q_EMIT operationErrorOccured(tr("System upgrade failed"));
        }
        Q_EMIT operationFinished();
        disconnectCurrentTransaction();
    }

    // this function runs in the background thread
    void startAddPackage(void *ct, const QString &packageNameSpec)
    {
        currentTransaction = reinterpret_cast<Transaction *>(ct);
        isBusy = true;
        bool ok = dbpriv->add(packageNameSpec);
        isBusy = false;
        connectCurrentTransaction();
        if (!ok) {
            Q_EMIT operationErrorOccured(tr("Add package failed"));
        }
        Q_EMIT operationFinished();
        disconnectCurrentTransaction();
    }

    // this function runs in the background thread
    void startDelPackage(void *ct, const QString &packageNameSpec, DbDelFlags flags)
    {
        currentTransaction = reinterpret_cast<Transaction *>(ct);
        isBusy = true;
        bool ok = dbpriv->del(packageNameSpec, flags);
        isBusy = false;
        connectCurrentTransaction();
        if (!ok) {
            Q_EMIT operationErrorOccured(tr("Remove package failed"));
        }
        Q_EMIT operationFinished();
        disconnectCurrentTransaction();
    }

Q_SIGNALS:
    void operationErrorOccured(QString msg);
    void operationFinished();
public:
    DatabasePrivate *dbpriv = nullptr;
    Transaction *currentTransaction = nullptr;
    bool isBusy = false;
};


DatabaseAsyncPrivate::DatabaseAsyncPrivate(DatabaseAsync *q)
    : q_ptr(q)
{
    // DatabasePrivate actually doesn't even use its q_ptr
    // FIXME: something to fix in DatabasePrivate?
    dbpriv = new DatabasePrivate(nullptr);
}

DatabaseAsyncPrivate::~DatabaseAsyncPrivate()
{
    close();
    delete dbpriv;
    dbpriv = nullptr;
}

void DatabaseAsyncPrivate::setFakeRoot(const QString& fakeRootDir)
{
    // cannot change fake root if database is already open
    if (isOpen()) {
        return;
    }
    dbpriv->fakeRoot = fakeRootDir;
}

QString DatabaseAsyncPrivate::fakeRoot() const
{
    return dbpriv->fakeRoot;
}

bool DatabaseAsyncPrivate::open(DbOpenFlags flags)
{
    bool ret = dbpriv->open(flags);
    if (ret) {
        // create bg thread executor object, socket notifier,
        // and start bg thread only if database was opened
        executor = new BgThreadExecutor();
        executor->dbpriv = this->dbpriv;
        executor->moveToThread(&bgThread);
        //
        socketNotifier = new QSocketNotifier(dbpriv->progressFd(), QSocketNotifier::Read);
        QObject::connect(socketNotifier, &QSocketNotifier::activated,
                         this, &DatabaseAsyncPrivate::onSocketNotifierActivated);
        socketNotifier->setEnabled(false);
        //
        bgThread.start();
    }
    return ret;
}

void DatabaseAsyncPrivate::close()
{
    if (socketNotifier) {
        socketNotifier->setEnabled(false);
        delete socketNotifier;
        socketNotifier = nullptr;
    }
    if (bgThread.isRunning()) {
        bgThread.requestInterruption();
        bgThread.exit(0);
        delete executor;
        executor = nullptr;
    }
    dbpriv->close(); // this also closes progress_fd pipe
}

bool DatabaseAsyncPrivate::isOpen() const
{
    return dbpriv->isOpen();
}

Transaction *DatabaseAsyncPrivate::updatePackageIndex(DbUpdateFlags flags)
{
    if (!checkCanStart()) {
        return nullptr;
    }
    // now that we have to do something, we can enable progress watched
    if (socketNotifier) {
        socketNotifier->setEnabled(true);
    }

    TransactionUpdatePrivate *trp = new TransactionUpdatePrivate(
                executor, "startUpdatePackageIndex", flags);
    return createReturnTransaction(trp);
}

int DatabaseAsyncPrivate::upgradeablePackagesCount()
{
    int totalUpgrades = 0;
    Changeset changes;

    if (dbpriv->upgrade(QTAPK_UPGRADE_SIMULATE, &changes)) {
        // Don't count packages to remove
        totalUpgrades = changes.numInstall() + changes.numAdjust();
    }
    return totalUpgrades;
}

Transaction *DatabaseAsyncPrivate::upgrade(DbUpgradeFlags flags, Changeset *changes)
{
    Q_UNUSED(changes)
    if (!checkCanStart()) {
        return nullptr;
    }
    // now that we have to do something, we can enable progress watcher
    if (socketNotifier) {
        socketNotifier->setEnabled(true);
    }

    TransactionUpgradePrivate *trp = new TransactionUpgradePrivate(
                executor, "startUpgradeSystem", flags);
    return createReturnTransaction(trp);
}

Transaction *DatabaseAsyncPrivate::add(const QString &packageNameSpec)
{
    if (!checkCanStart()) {
        return nullptr;
    }
    // now that we have to do something, we can enable progress watcher
    if (socketNotifier) {
        socketNotifier->setEnabled(true);
    }

    TransactionAddPrivate *trp = new TransactionAddPrivate(
                executor, "startAddPackage", packageNameSpec);
    return createReturnTransaction(trp);
}

Transaction *DatabaseAsyncPrivate::del(const QString &packageNameSpec, DbDelFlags flags)
{
    if (!checkCanStart()) {
        return nullptr;
    }
    // now that we have to do something, we can enable progress watcher
    if (socketNotifier) {
        socketNotifier->setEnabled(true);
    }

    TransactionDelPrivate *trp = new TransactionDelPrivate(
                executor, "startDelPackage", packageNameSpec, flags);
    return createReturnTransaction(trp);
}

QVector<Package> DatabaseAsyncPrivate::getInstalledPackages() const
{
    return dbpriv->get_installed_packages();
}

QVector<Package> DatabaseAsyncPrivate::getAvailablePackages() const
{
    return dbpriv->get_available_packages();
}

/**
 * @brief DatabaseAsyncPrivate::checkCanStart
 * @return true if start conditions are met
 */
bool DatabaseAsyncPrivate::checkCanStart()
{
    if (!executor) {
        qCWarning(LOG_QTAPK) << Q_FUNC_INFO << "cannot start transaction: no BG Thread!";
        return false;
    }
    if (executor->isBusy) {
        qCWarning(LOG_QTAPK) << Q_FUNC_INFO << "cannot execute more then one Transaction in parallel!";
        // Theoretically we could execute as many package operations
        // in parallel as we want. But our package manager would not like it!
        return false;
    }
    return true;
}

/**
 * @brief DatabaseAsyncPrivate::createReturnTransaction
 * @param trp - TransactionPrivate pointer with actual transaction
 * @return Transaction* object that is initialized and configured
 */
Transaction *DatabaseAsyncPrivate::createReturnTransaction(TransactionPrivate *trp)
{
    // use constructor with predefined private object
    Transaction *tr = new Transaction(trp);
    // executor->currentTransaction = tr; // this is wroooong
    // we expect Transaction object to be deleted using deleteLater()
    //     in one of the slots connected to Transaction::finished() signal

    // we need to know when our transaction ceases to exist
    QObject::connect(tr, &QObject::destroyed, this, &DatabaseAsyncPrivate::onTransactionDestroyed);
    return tr;
}

void DatabaseAsyncPrivate::onSocketNotifierActivated(int sock)
{
    char buf[64] = {0}; // 64 bytes should be enough for everyone
    std::size_t nr = ::read(sock, buf, sizeof(buf)-1);
    if (nr > 0) {
        uint64_t p1, p2;
        ::sscanf(buf, "%" PRIu64 "/%" PRIu64, &p1, &p2);
        float fpercent = 0.0f;
        if (p2) {
            fpercent = 100.0f * static_cast<float>(p1) / static_cast<float>(p2);
        }
        // now we need to invoke Transaction's progress() signal
        // pray that currentTransaction pointer is still valid
        if (executor && executor->currentTransaction) {
            Q_EMIT executor->currentTransaction->progressChanged(fpercent);
        }
    }
}

void DatabaseAsyncPrivate::onTransactionDestroyed(QObject *obj)
{
    if (!executor) {
        return;
    }
    if (obj == executor->currentTransaction) {
        executor->currentTransaction = nullptr;
    }
}


} // namespace QtApk

#include "QtApkDatabaseAsync_private.moc"
