// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef H_QTAPK_FLAGS
#define H_QTAPK_FLAGS

#include <QFlags>
#include <QMetaType>

namespace QtApk {


/**
 * @brief The DbOpenFlags enum
 * Used in open() method
 */
enum DbOpenFlagEnum {
    QTAPK_OPENF_READONLY = 0x1,           //! open database only for querying info
    QTAPK_OPENF_READWRITE = 0x2,          //! open for package manipulation, may need superuser rights
    QTAPK_OPENF_ENABLE_PROGRESSFD = 0x4,  //! open pipe channel to libapk to receive progress updates
};
Q_DECLARE_FLAGS(DbOpenFlags, DbOpenFlagEnum)
Q_DECLARE_OPERATORS_FOR_FLAGS(DbOpenFlags)

/**
 * @brief The DbUpdateFlags enum
 * Used in updatePackageIndex() method
 */
enum DbUpdateFlags {
    QTAPK_UPDATE_DEFAULT = 0,            //! no flags
    QTAPK_UPDATE_ALLOW_UNTRUSTED = 1     //! allow untrusted repository sources
};

/**
 * @brief The DbUpgradeFlags enum
 * Used in upgrade() method
 */
enum DbUpgradeFlags {
    QTAPK_UPGRADE_DEFAULT = 0,    //! No flags
    QTAPK_UPGRADE_SIMULATE = 1,   //! Do not do a real upgrade, only simulate.
                                  //! Allows to get an "upgrade plan" first
    QTAPK_UPGRADE_AVAILABLE = 2,  //! Resets versioned world dependencies, and changes to prefer replacing
                                  //! or downgrading packages (instead of holding them) if the currently
                                  //! installed package is no longer available from any repository
    QTAPK_UPGRADE_LATEST = 4,     //! Select latest version of package (if it is not pinned), and print
                                  //! error if it cannot be installed due to other dependencies
};

/**
 * @brief The DbDelFlags enum
 * Used for del() method
 */
enum DbDelFlags {
    QTAPK_DEL_DEFAULT = 0,    //! no flags
    QTAPK_DEL_RDEPENDS = 1    //! delete package and everything that depends on it
};


} // namespace QtApk

Q_DECLARE_METATYPE(QtApk::DbOpenFlags);
Q_DECLARE_METATYPE(QtApk::DbUpdateFlags);
Q_DECLARE_METATYPE(QtApk::DbUpgradeFlags);
Q_DECLARE_METATYPE(QtApk::DbDelFlags);

#endif
