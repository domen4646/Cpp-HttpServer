#include "headerparser.h"
#include "string_utilities.h"

#ifdef DEBUG
#include <iostream>
#endif

HeaderParser::HeaderParser()
{

}

unsigned int HeaderParser::getHeaderSize()
{
    return m_headerSize;
}

unsigned int HeaderParser::getHeaderOffsetInChunk()
{
    return m_headerOffsetChunk;
}

unsigned int HeaderParser::getInteger(std::string headerKey)
{
    std::string value = m_headers[headerKey];
    return std::stoi(value);
}

// Returns true if end of header, otherwise false
// NOTE: Check if headers are invalid if the function returns true.
bool HeaderParser::addChunk(std::string chunk)
{
    if (m_invalid || m_endOfHeader) {
        return true;
    }
    char prev = '\0';
    if (!m_current.empty()) {
        prev = m_current[m_current.length() - 1];
    }
    m_headerOffsetChunk = 0;
    for (char c : chunk) {
        m_headerSize++;
        m_headerOffsetChunk++;
        if (c == '\n' && prev == '\r') {
            m_current.pop_back(); // Remove \r
            if (m_current.empty()) {
                // two consecutive \r\n -> end of header
                m_endOfHeader = true;
                return true;
            }
            parseLine(m_current);
            m_current.clear();
            prev = '\0';
            continue;
        }
        m_current.push_back(c);
        prev = c;
    }
    return false;
}

bool HeaderParser::endOfHeader()
{
    return m_endOfHeader;
}

bool HeaderParser::hasKey(std::string key)
{
    return m_headers.count(key) > 0;
}

void HeaderParser::reset()
{
    m_endOfHeader = false;
    m_current.clear();
    m_headers.clear();
    m_headerSize = 0;
    m_firstLineParsed = false;
    m_invalid = false;
    method.clear();
    path.clear();
    protocol.clear();
}

// Parses header lines
void HeaderParser::parseLine(std::string line)
{
    if (m_invalid) {
        return;
    }
    if (!m_firstLineParsed) {
        parseFirstLine(line);
    }

    std::vector<std::string> keyval = string_utilities::split(line);
    if (keyval.size() != 2) {
        // Invalid header line, ignore
        return;
    }
    std::string key = keyval[0], val = keyval[1];
    string_utilities::trim(key);
    string_utilities::trim(val);
    if (key.empty() || val.empty()) {
        return;
    }
    m_headers.insert({key, val});
}

void HeaderParser::parseFirstLine(std::string line)
{
    std::vector<std::string> split = string_utilities::split_whitespace(line);
#ifdef DEBUG
    std::cout << line << std::endl;
#endif
    if (split.size() != 3) {
        m_invalid = true;
        return;

    }
    method = split[0];
    path = split[1];
    protocol = split[2];
    m_firstLineParsed = true;
}

