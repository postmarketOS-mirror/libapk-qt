// SPDX-License-Identifier: GPL-2.0-or-later

#include "QtApkRepository.h"
#include <QDataStream>


namespace QtApk {


Repository::Repository()
{
}

Repository::Repository(const QString &repoUrl, const QString &repoComment, bool en)
{
    url = repoUrl;
    comment = repoComment;
    enabled = en;
}

Repository::~Repository()
{
}


} // namespace QtApk


QDataStream &operator<<(QDataStream &stream, const QtApk::Repository &repo)
{
    stream << repo.url;
    stream << repo.comment;
    stream << repo.enabled;
    return stream;
}

QDataStream &operator>>(QDataStream &stream, QtApk::Repository &repo)
{
    stream >> repo.url;
    stream >> repo.comment;
    stream >> repo.enabled;
    return stream;
}

QDataStream &operator<<(QDataStream &stream, const QVector<QtApk::Repository> &repoVec)
{
    stream << repoVec.size();
    for (const QtApk::Repository &repo : repoVec) {
        stream << repo;
    }
    return stream;
}

QDataStream &operator>>(QDataStream &stream, QVector<QtApk::Repository> &repoVec)
{
    int sz = 0;
    stream >> sz;
    repoVec.reserve(sz);
    for (int i = 0; i < sz; i++) {
        QtApk::Repository repo;
        stream >> repo;
        repoVec.append(std::move(repo));
    }
    return stream;
}
