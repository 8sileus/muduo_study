#ifndef __SQLCONNECTION_H__
#define __SQLCONNECTION_H__

#include "log/Logging.h"
#include <semaphore.h>

#include <mysql/mysql.h>

#include <mutex>
#include <queue>
#include <string>
#include <thread>

class SqlConnectionPool {
public:
    static SqlConnectionPool* Instance();

    MYSQL* getConnection();
    void freeConnection(MYSQL* conn);
    int getQueueSize();
    void Init(const char* host, int port,
        const char* user, const char* pwd,
        const char* dbName, int connSize = 10);
    void closePool();

private:
    SqlConnectionPool();
    ~SqlConnectionPool();

private:
    int MAX_CONN_;
    int useCount_;
    int freeCount_;

    std::queue<MYSQL*> connQueue_;
    std::mutex mutex_;
    sem_t semId_;
};

#endif //__SQLCONNECTION_H__