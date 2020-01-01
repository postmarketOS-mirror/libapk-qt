#include <errno.h>
#include "apk_defines.h"

namespace QtApk {

// reimplemented from libapk, because libapk does not export apk_error_str()!!!
const char *qtapk_error_str(int error)
{
    if (error < 0)
        error = -error;
    switch (error) {
    case ENOKEY:
        return "UNTRUSTED signature";
    case EKEYREJECTED:
        return "BAD signature";
    case EIO:
        return "IO ERROR";
    case EBADMSG:
        return "BAD archive";
    case ENOMSG:
        return "archive does not contain expected data";
    case ENOPKG:
        return "could not find a repo which provides this package (check repositories file and run 'apk update')";
    case ECONNABORTED:
        return "network connection aborted";
    case ECONNREFUSED:
        return "could not connect to server (check repositories file)";
    case ENETUNREACH:
        return "network error (check Internet connection and firewall)";
    case ENXIO:
        return "DNS lookup error";
    case EREMOTEIO:
        return "remote server returned error (try 'apk update')";
    case ETIMEDOUT:
        return "operation timed out";
    case EAGAIN:
        return "temporary error (try again later)";
    case EAPKBADURL:
        return "invalid URL (check your repositories file)";
    case EAPKSTALEINDEX:
        return "package mentioned in index not found (try 'apk update')";
    default:
        return strerror(error);
    }
}

} // namespace QtApk
