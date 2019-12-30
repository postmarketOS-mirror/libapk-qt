#include <QObject>

namespace QtApk {

class DatabasePrivate;

class Database {
public:
    Database();

public:
    bool open();
    void close();

#ifdef QTAPK_DEVELOPER_BUILD
    void print_installed();
#endif

private:
    DatabasePrivate *d_ptr = nullptr;
    Q_DECLARE_PRIVATE(Database)
};

} // namespace QtApk
