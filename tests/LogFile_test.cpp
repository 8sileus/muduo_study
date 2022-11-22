#include "log/LogFile.h"
#include "log/Logging.h"

#include <unistd.h>

using namespace muduo;

std::unique_ptr<muduo::LogFile> g_logFile;

void outputFunc(const char* msg, int len)
{
    g_logFile->append(msg, len);
}

void flushFunc()
{
    g_logFile->flush();
}

int main(int argc, char* argv[])
{
    char name[256] = { '\0' };
    strncpy(name, argv[0], sizeof name - 1);
    g_logFile.reset(new muduo::LogFile(::basename(name), 300 * 1000));
    muduo::Logger::setOutputFunc(outputFunc);
    muduo::Logger::setFlushFunc(flushFunc);

    std::string line = "1234567890 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

    for (int i = 0; i < 10000; ++i) {
        LOG_INFO << line << i;

        usleep(1000);
    }
}
