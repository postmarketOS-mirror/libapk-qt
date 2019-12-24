#include <QObject>

namespace QtApk {

class DatabasePrivate;

class Database {
public:
    Database();

public:
    bool open();
    void close();

private:
    DatabasePrivate *d_ptr = nullptr;
    Q_DECLARE_PRIVATE(Database)
};

} // namespace QtApk
