#include "QtApkPackage.h"
#include <QDataStream>


namespace QtApk {


Package::Package()
{
}

Package::Package(const QString &pkgName)
{
    name = pkgName;
}

Package::~Package()
{
}

bool Package::isEmpty() const
{
    if (name.isEmpty() || version.isEmpty() || arch.isEmpty()) {
        return true;
    }
    return false;
}


} // namespace QtApk


QDataStream &operator<<(QDataStream &stream, const QtApk::Package &pkg)
{
    stream << pkg.name;
    stream << pkg.version;
    stream << pkg.arch;
    stream << pkg.license;
    stream << pkg.origin;
    stream << pkg.maintainer;
    stream << pkg.url;
    stream << pkg.description;
    stream << pkg.commit;
    stream << pkg.filename;
    stream << pkg.installedSize;
    stream << pkg.size;
    stream << pkg.buildTime;
    return stream;
}

QDataStream &operator>>(QDataStream &stream, QtApk::Package &pkg)
{
    stream >> pkg.name;
    stream >> pkg.version;
    stream >> pkg.arch;
    stream >> pkg.license;
    stream >> pkg.origin;
    stream >> pkg.maintainer;
    stream >> pkg.url;
    stream >> pkg.description;
    stream >> pkg.commit;
    stream >> pkg.filename;
    stream >> pkg.installedSize;
    stream >> pkg.size;
    stream >> pkg.buildTime;
    return stream;
}

QDataStream &operator<<(QDataStream &stream, const QVector<QtApk::Package> &pkgVec)
{
    stream << pkgVec.size();
    for (const QtApk::Package &pkg : pkgVec) {
        stream << pkg;
    }
    return stream;
}

QDataStream &operator>>(QDataStream &stream, QVector<QtApk::Package> &pkgVec)
{
    int sz = 0;
    stream >> sz;
    pkgVec.reserve(sz);
    for (int i = 0; i < sz; i++) {
        QtApk::Package pkg;
        stream >> pkg;
        pkgVec.append(std::move(pkg));
    }
    return stream;
}
