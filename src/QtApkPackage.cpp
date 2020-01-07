#include "QtApkPackage.h"


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
