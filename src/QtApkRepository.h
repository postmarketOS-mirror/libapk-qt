#ifndef H_QTAPK_REPOSITORY
#define H_QTAPK_REPOSITORY

#include <QObject>

namespace QtApk {

class Repository {
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

#endif
