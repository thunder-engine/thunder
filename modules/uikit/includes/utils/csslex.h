#ifndef CSSLEX_H
#define CSSLEX_H

#include "CSSLexStatus.h"

#include <string>
#include <set>

class Lex {
public:
    struct CSSToken {
        CSSTokenType type;
        std::string data;
    };

public:
    Lex();
    ~Lex();
    CSSToken *token();
    void cleanResource();
    void setBufferSource(const std::string &fileName);
    void setBufferString(const std::string &bufferString);

private:
    CSSToken *identToken();
    CSSToken *numberToken();
    CSSToken *textToken(char stringType);

    bool isDigitalCharacter(char);
    bool isLetter(char);
    bool isHexCharacter(char);
    bool isWs(char);

    std::string createData(size_t start, size_t end);

private:
    std::set<CSSToken *> m_tokenCache;
    std::string m_fileName;

    const char *m_buffer;

    size_t m_bufferSize;
    size_t m_firstPos;
    size_t m_forwardPos;

};

#endif /* CSSLEX_H */
