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
