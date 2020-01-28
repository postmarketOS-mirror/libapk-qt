#include "QtApkRepository.h"


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
