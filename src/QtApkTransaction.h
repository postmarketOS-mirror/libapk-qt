// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef H_QTAPK_TRANSACTION
#define H_QTAPK_TRANSACTION

#include <QObject>
#include "qtapk_exports.h"

namespace QtApk {

class Changeset;
class DatabaseAsyncPrivate;
class TransactionPrivate;

/**
 * @class Transaction
 * @brief Transaction class represents background package
 * manipulation operation, such as install/remove/upgrade
 * procedure. You can monitor operation progress ar force
 * cancel it.
 *
 * Transaction object is created as a result of function call
 * on DatabaseAsync class, such as add(), del() etc. You must
 * manually call start() to initiate transaction, but you may
 * also want to connect to its signals before that, to prevent
 * race conditions (like transaction finished before you make
 * all the connections).
 */
class QTAPK_EXPORTS Transaction : public QObject
{
    Q_OBJECT
    Q_PROPERTY(float currentProgress READ currentProgress WRITE setCurrentProgress NOTIFY progressChanged)
    Q_PROPERTY(QString desc READ desc WRITE setDesc NOTIFY descChanged)

public:
    enum TransactionType {
        ADD,
        DEL,
        UPDATE,
        UPGRADE
    };

    // only destructor is public
    virtual ~Transaction();

    TransactionType type() const;
    float currentProgress() const;
    QString desc() const;

    Changeset changeset();
    const Changeset changeset() const;

public Q_SLOTS:
    void setCurrentProgress(float percent);
    void setDesc(const QString &newDesc);
    // control
    void start();
    void cancel();

Q_SIGNALS:
    void descChanged();
    void finished();
    void progressChanged(float percent);
    void errorOccured(QString msg);

private:
    TransactionPrivate *d_ptr = nullptr;
    Q_DECLARE_PRIVATE(Transaction)

    // user cannot create/copy/move objects of this type
    Transaction();
    Transaction(TransactionPrivate *privData); // takes ownership over privData
    Q_DISABLE_COPY_MOVE(Transaction)

    friend class QtApk::DatabaseAsyncPrivate;
};

} // namespace QtApk

#endif
