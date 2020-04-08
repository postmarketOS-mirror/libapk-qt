#ifndef H_QTAPK_REPOSITORY
#define H_QTAPK_REPOSITORY

#include <QObject>
#include <QVector>

#include "qtapk_exports.h"

class QDataStream;

namespace QtApk {

/**
 * @class Repository
 * @brief Information about package repository
 *
 * Represents information about a package repo.
 */
class QTAPK_EXPORTS Repository {
    Q_GADGET
    Q_PROPERTY(QString url MEMBER url)
    Q_PROPERTY(QString comment MEMBER comment)
    Q_PROPERTY(bool enabled MEMBER enabled)

public:
    Repository();
    Repository(const QString &repoUrl, const QString &repoComment, bool en);
    Repository(const Repository &other) = default;
    Repository(Repository &&other) = default;
    virtual ~Repository();

    Repository &operator=(const Repository &other) = default;
    Repository &operator=(Repository &&other) = default;

    QString url;
    QString comment;
    bool enabled = true;
};

}

Q_DECLARE_METATYPE(QtApk::Repository)

QDataStream &operator<<(QDataStream &stream, const QtApk::Repository &repo);
QDataStream &operator>>(QDataStream &stream, QtApk::Repository &repo);
QDataStream &operator<<(QDataStream &stream, const QVector<QtApk::Repository> &repoVec);
QDataStream &operator>>(QDataStream &stream, QVector<QtApk::Repository> &repoVec);

#endif
