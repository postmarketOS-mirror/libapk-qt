// SPDX-License-Identifier: GPL-2.0-or-later

#include "QtApkTransaction.h"
#include "private/QtApkTransaction_private.h"

namespace QtApk {

// simply forward all calls to private class

Transaction::Transaction()
    : d_ptr(new TransactionPrivate(this))
{
}

Transaction::Transaction(TransactionPrivate *privData)
    : d_ptr(privData)
{
    d_ptr->q_ptr = this;
}

Transaction::~Transaction()
{
    delete d_ptr;
    d_ptr = nullptr;
}

Transaction::TransactionType Transaction::type() const
{
    Q_D(const Transaction);
    return d->type();
}

float Transaction::currentProgress() const
{
    Q_D(const Transaction);
    return d->currentProgress();
}

QString Transaction::desc() const
{
    Q_D(const Transaction);
    return d->desc();
}

Changeset Transaction::changeset()
{
    Q_D(const Transaction);
    return d->changeset();
}

const Changeset Transaction::changeset() const
{
    Q_D(const Transaction);
    return d->changeset();
}

void Transaction::setCurrentProgress(float percent)
{
    Q_D(Transaction);
    d->setCurrentProgress(percent);
}

void Transaction::setDesc(const QString &newDesc)
{
    Q_D(Transaction);
    d->setDesc(newDesc);
}

void Transaction::start()
{
    Q_D(Transaction);
    d->start();
}

void Transaction::cancel()
{
    Q_D(Transaction);
    d->cancel();
}


} // namespace QtApk
