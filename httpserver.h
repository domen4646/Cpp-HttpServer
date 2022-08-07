#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "common.h"
#include <vector>
#include <map>
#include <functional>
#include <signal.h>
#include <poll.h>

class HttpClientHandler;

class HttpServer
{
private:
    std::vector<HttpClientHandler*> m_clients;
    std::vector<struct pollfd> m_pollClients;
    int m_serverSocket;
    std::function<unsigned int(HttpClientHandler*, unsigned int&)> m_dataHandler;
    void acceptNewClient();
    void cleanup();
    void cleanClientVector();

public:
    std::map<std::string, std::string> m_serverHeaders;
    std::string m_protocol;

    HttpServer();
    ~HttpServer();

    void addServerHeader(std::string key, std::string val);
    int start(unsigned short port);
    void removeClient(int clifd, bool close = true);
    unsigned int callDataHandler(HttpClientHandler* hch, unsigned int &code);
    void setDataHandler(std::function<unsigned int(HttpClientHandler*, unsigned int&)> _h);
};

#endif // HTTPSERVER_H
