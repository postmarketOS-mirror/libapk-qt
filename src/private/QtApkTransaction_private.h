// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef H_QTAPK_TRANSACTION_PRIVATE
#define H_QTAPK_TRANSACTION_PRIVATE

#include <QString>
#include <QVector>
#include <QObject>
#include <string>
#include "../QtApkTransaction.h"
#include "../QtApkFlags.h"
#include "../QtApkChangeset.h"

namespace QtApk {

class DatabaseAsyncPrivate;

class TransactionPrivate {
public:
    TransactionPrivate(Transaction *q);
    virtual ~TransactionPrivate();

    Transaction::TransactionType type() const;
    float currentProgress() const;
    QString desc() const;

    Changeset changeset();
    const Changeset changeset() const;

    void setCurrentProgress(float percent);
    void setDesc(const QString &newDesc);

    virtual void start();
    virtual void cancel();

public:
    Transaction *q_ptr = nullptr;
    Transaction::TransactionType _typ = Transaction::TransactionType::ADD;
    float _progress = 0.0f;
    QString _desc;
    Changeset _changeset;
    DatabaseAsyncPrivate *_db = nullptr;

    // information needed to call invokeMethod:
    QObject *_asyncObject;
    std::string _asyncMethodName;
};


class TransactionUpdatePrivate: public TransactionPrivate
{
public:
    TransactionUpdatePrivate(QObject *callObject, const char *methodName, DbUpdateFlags flags);
    void start() override;
    void cancel() override;

private:
    DbUpdateFlags _callFlags;
};


class TransactionAddPrivate: public TransactionPrivate
{
public:
    TransactionAddPrivate(QObject *callObject, const char *methodName, const QString &pkgNameSpec);
    void start() override;
    void cancel() override;

private:
    QString _pkgNameSpec;
};


class TransactionDelPrivate: public TransactionPrivate
{
public:
    TransactionDelPrivate(QObject *callObject, const char *methodName, const QString &pkgNameSpec, DbDelFlags flags);
    void start() override;
    void cancel() override;

private:
    QString _pkgNameSpec;
    DbDelFlags _flags;
};


class TransactionUpgradePrivate: public TransactionPrivate
{
public:
    TransactionUpgradePrivate(QObject *callObject, const char *methodName, DbUpgradeFlags flags);
    void start() override;
    void cancel() override;

private:
    DbUpgradeFlags _flags;
};


} // namespace QtApk

#endif
