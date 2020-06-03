// SPDX-License-Identifier: GPL-2.0-or-later

#include "QtApkTransaction_private.h"


namespace QtApk {


TransactionPrivate::TransactionPrivate(Transaction *q)
    : q_ptr(q)
{
}

TransactionPrivate::~TransactionPrivate()
{
    // we actually do not own anything, no resources that we
    // create by ourselves here, so nothing to delete
    _asyncMethodName.clear(); // may free some mem?
}

Transaction::TransactionType TransactionPrivate::type() const
{
    return _typ;
}

float TransactionPrivate::currentProgress() const
{
    return _progress;
}

QString TransactionPrivate::desc() const
{
    return _desc;
}

Changeset TransactionPrivate::changeset()
{
    return _changeset;
}

const Changeset TransactionPrivate::changeset() const
{
    return _changeset;
}

void TransactionPrivate::setCurrentProgress(float percent)
{
    if (percent != _progress) {
        _progress = percent;
        // at early stage during construction there is a short period of time
        // when q_ptr may be nullptr
        if (q_ptr) {
            Q_EMIT q_ptr->progressChanged(_progress);
        }
    }
}

void TransactionPrivate::setDesc(const QString &newDesc)
{
    if (newDesc != _desc) {
        _desc = newDesc;
        // at early stage during construction there is a short period of time
        // when q_ptr may be nullptr
        if (q_ptr) {
            Q_EMIT q_ptr->descChanged();
        }
    }
}

void TransactionPrivate::start()
{
}

void TransactionPrivate::cancel()
{
}

TransactionUpdatePrivate::TransactionUpdatePrivate(QObject *callObject,
                                                   const char *methodName,
                                                   DbUpdateFlags flags)
    : TransactionPrivate(nullptr)
{
    _typ = Transaction::TransactionType::UPDATE;
    _asyncObject = callObject;
    _asyncMethodName.assign(methodName);
    _callFlags = flags;
    setDesc(QStringLiteral("Update package index"));
}

void TransactionUpdatePrivate::start()
{
    QMetaObject::invokeMethod(_asyncObject, _asyncMethodName.c_str(),
                              Qt::QueuedConnection,
                              Q_ARG(void *, q_ptr),
                              Q_ARG(DbUpdateFlags, _callFlags));
}

void TransactionUpdatePrivate::cancel()
{
    // we cannot cancel apk update operation (yet?)
    // maybe to do later
}

TransactionAddPrivate::TransactionAddPrivate(QObject *callObject,
                                             const char *methodName,
                                             const QString &pkgNameSpec)
    : TransactionPrivate(nullptr)
{
    _typ = Transaction::TransactionType::ADD;
    _asyncObject = callObject;
    _asyncMethodName.assign(methodName);
    _pkgNameSpec = pkgNameSpec;
    setDesc(QStringLiteral("Install package: ") + _pkgNameSpec);
}

void TransactionAddPrivate::start()
{
    QMetaObject::invokeMethod(_asyncObject, _asyncMethodName.c_str(),
                              Qt::QueuedConnection,
                              Q_ARG(void *, q_ptr),
                              Q_ARG(QString, _pkgNameSpec));
}

void TransactionAddPrivate::cancel()
{
}

TransactionDelPrivate::TransactionDelPrivate(QObject *callObject,
                                             const char *methodName,
                                             const QString &pkgNameSpec,
                                             DbDelFlags flags)
    : TransactionPrivate(nullptr)
{
    _typ = Transaction::TransactionType::DEL;
    _asyncObject = callObject;
    _asyncMethodName.assign(methodName);
    _pkgNameSpec = pkgNameSpec;
    _flags = flags;
    setDesc(QStringLiteral("Remove package: ") + _pkgNameSpec);
}

void TransactionDelPrivate::start()
{
    QMetaObject::invokeMethod(_asyncObject, _asyncMethodName.c_str(),
                              Qt::QueuedConnection,
                              Q_ARG(void *, q_ptr),
                              Q_ARG(QString, _pkgNameSpec),
                              Q_ARG(DbDelFlags, _flags));
}

void TransactionDelPrivate::cancel()
{
}

TransactionUpgradePrivate::TransactionUpgradePrivate(QObject *callObject,
                                                     const char *methodName,
                                                     DbUpgradeFlags flags)
    : TransactionPrivate(nullptr)
{
    _typ = Transaction::TransactionType::UPGRADE;
    _asyncObject = callObject;
    _asyncMethodName.assign(methodName);
    _flags = flags;
    setDesc(QStringLiteral("System upgrade"));
}

void TransactionUpgradePrivate::start()
{
    QMetaObject::invokeMethod(_asyncObject, _asyncMethodName.c_str(),
                              Qt::QueuedConnection,
                              Q_ARG(void *, q_ptr),
                              Q_ARG(DbUpgradeFlags, _flags),
                              Q_ARG(void *, &_changeset));
}

void TransactionUpgradePrivate::cancel()
{
}

} // namespace QtApk
