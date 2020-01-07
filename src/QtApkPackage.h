#ifndef H_QTAPK_PACKAGE
#define H_QTAPK_PACKAGE

#include <QString>
#include <QObject>
#include <QDateTime>


namespace QtApk {


/**
 * @brief The Package class
 * Represents information about a single package.
 * Kind of a Qt's variant of struct apk_package
 * without all complex stuff.
 */
class Package
{
    Q_GADGET
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(QString version MEMBER version)
    Q_PROPERTY(QString arch MEMBER arch)
    Q_PROPERTY(QString license MEMBER license)
    Q_PROPERTY(QString origin MEMBER origin)
    Q_PROPERTY(QString mantainer MEMBER mantainer)
    Q_PROPERTY(QString url MEMBER url)
    Q_PROPERTY(QString description MEMBER description)
    Q_PROPERTY(QString commit MEMBER commit)
    Q_PROPERTY(QString filename MEMBER filename)
    Q_PROPERTY(quint64 installedSize MEMBER installedSize)
    Q_PROPERTY(quint64 size MEMBER size)
    Q_PROPERTY(QDateTime buildTime MEMBER buildTime)

public:
    Package();
    Package(const QString &pkgName);
    Package(const Package &other) = default;
    Package(Package &&other) = default;
    virtual ~Package();

    Package &operator=(const Package &other) = default;
    Package &operator=(Package &&other) = default;

public:
    QString name;
    QString version;
    QString arch;
    QString license;
    QString origin;
    QString maintainer;
    QString url;
    QString description;
    QString commit;
    QString filename;
    quint64 installedSize = 0;
    quint64 size = 0;
    QDateTime buildTime;
};


} // namespace QtApk

#endif
