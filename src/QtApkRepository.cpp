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


QDataStream &operator<<(QDataStream &stream, const QtApk::Repository &pkg)
{
    stream << pkg.url;
    stream << pkg.comment;
    stream << pkg.enabled;
    return stream;
}

QDataStream &operator>>(QDataStream &stream, QtApk::Repository &pkg)
{
    stream >> pkg.url;
    stream >> pkg.comment;
    stream >> pkg.enabled;
    return stream;
}
