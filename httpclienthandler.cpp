#include "httpclienthandler.h"
#include "httpserver.h"
#include "string_utilities.h"
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

void HttpClientHandler::checkContent()
{
    if (m_contentLength > MAX_CONTENT_LENGTH || m_inData.size() > m_contentLength) {
        //close(m_clientSocket);
        m_ref->removeClient(m_clientSocket);
        m_valid = false;
        return;
    }
    if (m_contentLength == m_inData.size()) {
        // Finished with data, call handler
        m_finished = true;
        unsigned int code = 0;
        m_toSend = m_ref->callDataHandler(this, code);
        if (m_toSend == 0) {
            m_ref->removeClient(m_clientSocket);
            m_valid = false;
            return;
        }
        addServerHeaders(code);
    }
}

HttpClientHandler::HttpClientHandler(HttpServer *ref, int sock)
{
    m_ref = ref;
    m_clientSocket = sock;
    m_buffer.resize(BUFFER_SIZE);
}

void HttpClientHandler::updateRead()
{
    if (!m_valid || m_buffer.empty()) {
        return;
    }
    int result = recv(m_clientSocket, (void*) &m_buffer[0], m_buffer.size(), 0);
    if (result < 1) {
        m_valid = false;
        m_ref->removeClient(m_clientSocket);
        //close(m_clientSocket);
        return;
    }
    if (!m_parser.endOfHeader()) {
        if (m_parser.addChunk(std::string(m_buffer.begin(), m_buffer.begin() + result))) {
            if (m_parser.hasKey("Content-Length")) {
                m_contentLength = m_parser.getInteger("Content-Length");
            }
            unsigned int chunkOffset = m_parser.getHeaderOffsetInChunk();
            if (result > chunkOffset) {
                m_inData.insert(m_inData.end(), m_buffer.begin() + chunkOffset, m_buffer.begin() + result);
            }
            checkContent();
        }
    }
    else {
        m_inData.insert(m_inData.end(), m_buffer.begin(), m_buffer.begin() + result);
        checkContent();
    }
}

void HttpClientHandler::updateWrite()
{
    if (m_outData.empty()) {
        return;
    }
    int result = send(m_clientSocket, (const void*) &m_outData[0], m_outData.size(), 0);
    if (result < 1) {
        m_valid = false;
        //close(m_clientSocket);
        m_ref->removeClient(m_clientSocket);
        return;
    }
    m_sent += result;
    m_outData.erase(m_outData.begin(), m_outData.begin() + result);
    checkWriteFinished();
}

int HttpClientHandler::getSocket() const
{
    return m_clientSocket;
}

bool HttpClientHandler::finished()
{
    return m_finished;
}

bool HttpClientHandler::valid()
{
    return m_valid;
}

void HttpClientHandler::appendData(std::vector<byte>& data)
{
    m_outData.insert(m_outData.end(), data.begin(), data.end());
}

void HttpClientHandler::appendString(std::string& data)
{
    m_outData.insert(m_outData.end(), data.begin(), data.end());
}

HeaderParser *HttpClientHandler::getParser()
{
    return &m_parser;
}

void HttpClientHandler::checkWriteFinished()
{
    if (m_sent >= m_toSend) {
        m_ref->removeClient(m_clientSocket);
        m_valid = false;
    }
}

void HttpClientHandler::addServerHeaders(unsigned int responseCode)
{
    // Construct headers
    std::stringstream ss;
    ss << m_ref->m_protocol << " " << responseCode << " " << string_utilities::messageFromHttpResponseCode(responseCode) << "\r\n";
    for (const auto &pair : m_ref->m_serverHeaders) {
        ss << pair.first << ": " << pair.second << "\r\n";
    }
    ss << "Content-Length: " << m_toSend << "\r\n";
    ss << "\r\n";
    std::string headers = ss.str();
    m_outData.insert(m_outData.begin(), headers.begin(), headers.end());
    m_toSend += headers.length();
}
