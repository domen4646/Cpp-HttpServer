#ifndef HTTPCLIENTHANDLER_H
#define HTTPCLIENTHANDLER_H

#include "common.h"
#include "headerparser.h"
#include <vector>

#define MAX_CONTENT_LENGTH (10 << 20) // 10MB

class HttpServer;

class HttpClientHandler
{
private:
    HttpServer* m_ref = nullptr;
    HeaderParser m_parser;
    int m_clientSocket;
    std::vector<byte> m_buffer;
    std::vector<byte> m_inData;
    std::vector<byte> m_outData;
    bool m_valid = true;
    bool m_finished = false;
    unsigned int m_contentLength = 0;
    unsigned int m_sent = 0;
    unsigned int m_toSend = 0;
    void checkContent();
    void checkWriteFinished();
    void addServerHeaders(unsigned int responseCode);

public:
    HttpClientHandler(HttpServer* ref, int sock);
    void updateRead();
    void updateWrite();
    int getSocket() const;
    bool finished();
    bool valid();
    void appendData(std::vector<byte>& data);
    void appendString(std::string& data);
    HeaderParser* getParser();
};

#endif // HTTPCLIENTHANDLER_H
