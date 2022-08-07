#ifndef HEADERPARSER_H
#define HEADERPARSER_H

#include "common.h"
#include <map>

class HeaderParser
{
private:
    std::map<std::string, std::string> m_headers;
    std::string m_current = "";
    unsigned int m_headerSize = 0;
    unsigned int m_headerOffsetChunk = 0;
    bool m_endOfHeader = false;
    bool m_firstLineParsed = false;
    bool m_invalid = false;

    void parseLine(std::string line);
    void parseFirstLine(std::string line);

public:

    std::string method;
    std::string path;
    std::string protocol;

    HeaderParser();
    unsigned int getHeaderSize();
    unsigned int getHeaderOffsetInChunk();
    unsigned int getInteger(std::string headerKey);
    bool addChunk(std::string chunk);
    bool endOfHeader();
    bool hasKey(std::string key);
    void reset();
};

#endif // HEADERPARSER_H
