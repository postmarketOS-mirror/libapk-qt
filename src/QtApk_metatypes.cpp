#include <QtGlobal>
#include <QMetaType>

#include "QtApkFlags.h"
#include "QtApkPackage.h"
#include "QtApkRepository.h"

namespace QtApk {

static void registerMetaTypes()
{
    // register types withing Qt's type system
    // it will allow us to use our types with QVariant
    // and in signal-slot connections
    qRegisterMetaType<QtApk::Package>("QtApk::Package");
    qRegisterMetaTypeStreamOperators<QtApk::Package>("QtApk::Package");
    qRegisterMetaTypeStreamOperators<QVector<QtApk::Package>>("QVector<QtApk::Package>");
    qRegisterMetaType<QtApk::Repository>("QtApk::Repository");
    qRegisterMetaTypeStreamOperators<QtApk::Repository>("QtApk::Repository");
    qRegisterMetaTypeStreamOperators<QVector<QtApk::Repository>>("QVector<QtApk::Repository>");
    // also register flags
    qRegisterMetaType<QtApk::DbOpenFlags>("QtApk::DbOpenFlags");
    qRegisterMetaType<QtApk::DbOpenFlags>("DbOpenFlags"); // without namespace
    qRegisterMetaType<QtApk::DbUpdateFlags>("QtApk::DbUpdateFlags");
    qRegisterMetaType<QtApk::DbUpdateFlags>("DbUpdateFlags"); // without namespace
    qRegisterMetaType<QtApk::DbUpgradeFlags>("QtApk::DbUpgradeFlags");
    qRegisterMetaType<QtApk::DbUpgradeFlags>("DbUpgradeFlags"); // without namespace
    qRegisterMetaType<QtApk::DbDelFlags>("QtApk::DbDelFlags");
    qRegisterMetaType<QtApk::DbDelFlags>("DbDelFlags"); // without namespace
}

Q_CONSTRUCTOR_FUNCTION(registerMetaTypes);

} // namespace QtApk
