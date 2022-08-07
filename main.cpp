#include "common.h"
#include <iostream>
#include <ctime>
#include "httpserver.h"
#include "httpclienthandler.h"

#define PORT 3000

std::string indexPage = "<html>"
        "<head>"
        "<title>C++ HTTP Server 1.0</title>"
        "</head>"
        "<body>"
        "<h3>C++ HTTP Server 1.0</h3>"
        "<hr>"
        "If you are seeing this page, it means that the server works!"
        "</body>"
        "</html>";

int main()
{
    HttpServer* server = new HttpServer();

    // Server data handler
    server->m_protocol = "HTTP/1.1";
    server->addServerHeader("Server", "C++ HTTP Server 1.0");
    server->setDataHandler([](HttpClientHandler* handler, unsigned int &code) -> unsigned int {
        if (handler->getParser()->path == "/") {
            code = 200;
            handler->appendString(indexPage);
            return indexPage.length();
        }
        if (handler->getParser()->path == "/time") {
            code = 200;
            time_t _time = time(nullptr);
            struct tm* currentTime = localtime(&_time);
            std::string result = std::string(asctime(currentTime));
            handler->appendString(result);
            return result.length();
        }

        code = 404;
        std::string response = "404 Not found.";
        handler->appendString(response);
        return response.length();
    });


    std::cout << "Starting server at port " << PORT << " ..." << std::endl;
    server->start(PORT);
    std::cout << "Server stopped" << std::endl;
}
