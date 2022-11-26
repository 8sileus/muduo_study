#include "SqlConnection.h"

SqlConnectionPool* SqlConnectionPool::Instance()
{
    static SqlConnectionPool conn;
    return &conn;
}

SqlConnectionPool::SqlConnectionPool()
    : useCount_(0)
    , freeCount_(0)
{
}

void SqlConnectionPool::Init(const char* host, int port,
    const char* user, const char* pwd, const char* dbName,
    int connSize)
{
    for (int i = 0; i < connSize; ++i) {
        MYSQL* sql = nullptr;
        sql = mysql_init(sql);
        if (!sql) {
            LOG_FATAL << "MySql init error!";
        }
        sql = mysql_real_connect(sql, host, user, pwd, dbName, port, nullptr, 0);
        if (!sql) {
            LOG_ERROR << "MySql Connect error!";
        }
        connQueue_.push(sql);
    }
    MAX_CONN_ = connSize;
    sem_init(&semId_, 0, MAX_CONN_);
}

MYSQL* SqlConnectionPool::getConnection()
{
    MYSQL* sql = nullptr;
    if (connQueue_.empty()) {
        LOG_WARN << "SqlConnPool busy!";
        return nullptr;
    }
    sem_wait(&semId_);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        sql = connQueue_.front();
        connQueue_.pop();
    }
    return sql;
}

void SqlConnectionPool::freeConnection(MYSQL* sql)
{
    std::lock_guard<std::mutex> lock(mutex_);
    connQueue_.push(sql);
    sem_post(&semId_);
}

void SqlConnectionPool::closePool()
{
    std::lock_guard<std::mutex> lock(mutex_);
    while (!connQueue_.empty()) {
        auto item = connQueue_.front();
        connQueue_.pop();
        mysql_close(item);
    }
    mysql_library_end();
}

int SqlConnectionPool::getQueueSize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return connQueue_.size();
}

SqlConnectionPool::~SqlConnectionPool()
{
    closePool();
}
