#ifndef H_QTAPK_CHANGESET
#define H_QTAPK_CHANGESET

#include <QVector>
#include "QtApkPackage.h"

#include "qtapk_exports.h"

namespace QtApk {

class QTAPK_EXPORTS ChangesetItem
{
public:
    ChangesetItem();

    Package oldPackage;
    Package newPackage;
    bool reinstall = false;
};

/**
 * @brief The Changeset class
 * Represents set of changes APK will apply
 * when installing, deleting packages, or during
 * system upgrade
 */
class QTAPK_EXPORTS Changeset
{
public:
    Changeset();

    int numInstall() const { return m_numInstall; }
    int numRemove() const { return m_numRemove; }
    int numAdjust() const { return m_numAdjust; }

    void setNumInstall(int n) { m_numInstall = n; }
    void setNumRemove(int n) { m_numRemove = n; }
    void setNumAdjust(int n) { m_numAdjust = n; }

    QVector<ChangesetItem> &changes() { return m_changes; }
    const QVector<ChangesetItem> &changes() const { return m_changes; }

protected:
    int m_numInstall = 0;
    int m_numRemove = 0;
    int m_numAdjust = 0;
    QVector<ChangesetItem> m_changes;
};

} // namespace QtApk

#endif  /* H_QTAPK_CHANGESET */
