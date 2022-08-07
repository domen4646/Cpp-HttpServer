#include "httpserver.h"
#include "httpclienthandler.h"
#include <iostream>
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

void HttpServer::acceptNewClient()
{
#ifdef DEBUG
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(struct sockaddr_in);
    int clifd = accept(m_serverSocket, (struct sockaddr*) &clientAddr, &addrLen);
#else
    int clifd = accept(m_serverSocket, nullptr, nullptr);
#endif
    if (clifd < 1) {
        // Invalid client
        return;
    }



#ifdef DEBUG
    char addr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, (const void*) &clientAddr.sin_addr, addr, INET_ADDRSTRLEN);
    std::cout << "New client connected: " << addr << std::endl;
#endif

    int status = fcntl(clifd, F_SETFL, fcntl(clifd, F_GETFL, 0) | O_NONBLOCK);
    if (status < 0) {
        close(clifd);
        return;
    }

    HttpClientHandler* handler = new HttpClientHandler(this, clifd);
    m_clients.push_back(handler);
    m_pollClients.push_back({clifd, POLLIN | POLLPRI, 0});
}

void HttpServer::cleanup()
{
    close(m_serverSocket);
    m_serverSocket = 0;
    for (int i=0; i<m_clients.size(); i++) {
        delete m_clients[i];
    }
    m_clients.clear();
    m_pollClients.clear();
}

void HttpServer::cleanClientVector()
{
    m_clients.erase(std::remove_if(m_clients.begin(), m_clients.end(), [](const HttpClientHandler* hch) -> bool {
        return (hch == nullptr || hch->getSocket() < 0);
    }), m_clients.end());
    m_pollClients.erase(std::remove_if(m_pollClients.begin(), m_pollClients.end(), [](const struct pollfd &pfd) -> bool {
        return pfd.fd < 0;
    }), m_pollClients.end());
}

HttpServer::HttpServer()
{

}

HttpServer::~HttpServer()
{
    cleanup();
}

void HttpServer::addServerHeader(std::string key, std::string val)
{
    m_serverHeaders.insert({key, val});
}

int HttpServer::start(unsigned short port)
{
    m_serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_serverSocket < 1) {
        int err = errno;
        perror("socket");
        return err;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (bind(m_serverSocket, (struct sockaddr*) &addr, sizeof(addr))) {
        int err = errno;
        perror("bind");
        close(m_serverSocket);
        return err;
    }

    int status = fcntl(m_serverSocket, F_SETFL, fcntl(m_serverSocket, F_GETFL, 0) | O_NONBLOCK);
    if (status < 0) {
        int err = errno;
        perror("fcntl");
        close(m_serverSocket);
        return err;
    }

    if (listen(m_serverSocket, SOMAXCONN)) {
        int err = errno;
        perror("listen");
        close(m_serverSocket);
        return err;
    }

    m_pollClients.push_back({m_serverSocket, POLLIN | POLLPRI, 0});

    int result = 0, err = 0;
    while (true) {
        result = poll(&m_pollClients[0], m_pollClients.size(), -1);
        if (result < 0) {
            err = errno;
            perror("poll");
            cleanup();
            return err;
        }

        for (int i=0; i<m_pollClients.size(); i++) {
            if (m_pollClients[i].revents & POLLIN) {
                if (i == 0) {
                    // New client joined
                    acceptNewClient();
                }
                else {
                    m_clients[i - 1]->updateRead();
                    if (m_clients[i - 1] != nullptr && m_clients[i - 1]->finished()) {
                        // Set to wait for write
                        m_pollClients[i].events = POLLOUT | POLLPRI;
                    }
                }
            }
            if (m_pollClients[i].revents & POLLOUT && i > 0) {
                // Writing possible
                m_clients[i - 1]->updateWrite();
            }
        }
        cleanClientVector();
    }

    return 0;
}

void HttpServer::removeClient(int clifd, bool close)
{
    std::vector<struct pollfd>::iterator pollit = std::find_if(m_pollClients.begin(), m_pollClients.end(), [&](const struct pollfd& pfd) -> bool {
        return pfd.fd == clifd;
    });
    std::vector<HttpClientHandler*>::iterator cliit = std::find_if(m_clients.begin(), m_clients.end(), [&](const HttpClientHandler* hch) -> bool {
        return hch->getSocket() == clifd;
    });
    pollit->fd = -1;
    delete (*cliit);
    *cliit = nullptr;
    if (close)
        ::close(clifd);
}

unsigned int HttpServer::callDataHandler(HttpClientHandler *hch, unsigned int &code)
{
    if (m_dataHandler) {
        return m_dataHandler(hch, code);
    }
    code = 0;
    return 0;
}


void HttpServer::setDataHandler(std::function<unsigned int(HttpClientHandler *, unsigned int&)> _h)
{
    m_dataHandler = _h;
}
