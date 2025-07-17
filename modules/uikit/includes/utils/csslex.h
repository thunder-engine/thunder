#ifndef CSSLEX_H
#define CSSLEX_H

#include <set>

#include <astring.h>

#include "csslexstatus.h"

using namespace next;

class Lex {
public:
    struct CSSToken {
        CSSTokenType type;
        String data;
    };

public:
    Lex();
    ~Lex();
    CSSToken *token();
    void cleanResource();
    void setBufferString(const String &bufferString);

private:
    CSSToken *identToken();
    CSSToken *numberToken();
    CSSToken *textToken(char stringType);

    bool isDigitalCharacter(char);
    bool isLetter(char);
    bool isHexCharacter(char);
    bool isWs(char);

    String createData(size_t start, size_t end);

private:
    std::set<CSSToken *> m_tokenCache;
    String m_fileName;

    char *m_buffer;

    size_t m_bufferSize;
    size_t m_firstPos;
    size_t m_forwardPos;

};

#endif /* CSSLEX_H */
