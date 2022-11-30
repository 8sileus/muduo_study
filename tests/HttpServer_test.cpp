#include "EventLoop.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "http/HttpServer.h"
#include "log/AsyncLogging.h"
#include "log/Logging.h"

#include <iostream>
#include <map>

using namespace muduo;

muduo::AsyncLogging* g_asyncLog = nullptr;

void asyncOutput(const char* msg, int len)
{
    g_asyncLog->append(msg, len);
}

bool benchmark = false;

void onRequest(const HttpRequest& req, HttpResponse* resp)
{
    std::cout << "Headers " << req.methodString() << " " << req.path() << std::endl;
    if (!benchmark) {
        const std::map<std::string, std::string>& headers = req.headers();
        for (const auto& header : headers) {
            std::cout << header.first << ": " << header.second << std::endl;
        }
    }

    if (req.path() == "/") {
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("text/html");
        resp->addHeader("Server", "Muduo");
        std::string now = Timestamp::now().toFormattedString(false);
        resp->setBody("<html><head><title>This is title</title></head>"
                      "<body><h1>Hello</h1>Now is "
            + now + "</body></html>");
    } else if (req.path() == "/hello") {
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("text/plain");
        resp->addHeader("Server", "Muduo");
        resp->setBody("hello, world!\n");
    } else {
        resp->setStatusCode(HttpResponse::k404NotFound);
        resp->setStatusMessage("Not Found");
        resp->setCloseConnection(true);
    }
}

int main(int argc, char* argv[])
{
    int numThreads = 0;
    if (argc > 1) {
        benchmark = true;
        Logger::setLogLevel(Logger::WARN);
        numThreads = atoi(argv[1]);
    }
    off_t kRollSize = 500 * 1000 * 1000;
    muduo::AsyncLogging log(::basename("HttpServer_test"), kRollSize);
    log.start();
    g_asyncLog = &log;
    EventLoop loop;
    Logger::setOutputFunc(asyncOutput);

    HttpServer server(&loop, InetAddress(8000), "dummy");
    server.setThreadNum(4);
    server.setHttpCallback(onRequest);
    server.setThreadNum(numThreads);
    server.start();
    loop.loop();
}