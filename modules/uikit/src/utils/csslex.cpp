#include "utils/csslex.h"

#include <cstring>

#define NextChar(buffer) m_forwardPos >= m_bufferSize ? 0 : *(buffer + m_forwardPos++)
#define ErrorInLoop STATUS = LexError;stopLoop = true;
#define WS_CASE ' ': case '\r': case '\n': case '\t': case '\f'
#define NUMBER_CASE '0': case '1': case '2': case '3': case '4':\
case '5': case '6': case '7': case '8': case '9'

Lex::Lex() {
    m_buffer = 0;
    m_firstPos = 0;
    m_forwardPos = 0;
}

Lex::~Lex() {
    cleanResource();
}

void Lex::setBufferString(const TString &bufferString) {
    if(bufferString.isEmpty()) {
        return;
    }

    if(m_buffer) {
        delete [] m_buffer;
        m_buffer = 0;
    }

    m_bufferSize = bufferString.size();
    m_buffer = new char[m_bufferSize];
    memcpy(static_cast<void *>(m_buffer), bufferString.data(), m_bufferSize);
    m_firstPos = 0;
    m_forwardPos = 0;
}

Lex::CSSToken *Lex::textToken(char stringType) {
    enum {
        _START = 0,
        _ESCAPESTART,
        _STRING
    };
    char status = _START;
    CSSToken *token = new CSSToken;
    m_tokenCache.insert(token);
    m_forwardPos = m_firstPos;

    if(m_forwardPos >= m_bufferSize) {
        token->type = END;
        return token;
    }
    char stringBoundary = stringType == 1 ? '"' : '\'';
    while(1) {
        char c = NextChar(m_buffer);
        switch(status) {
            case _START: {
                if((c != '\r' && c != '\n' && c != '\f' && c != stringBoundary) || (c & 0x80)) {
                    status = _STRING;
                    break;
                } else if(c == '\\') {
                    status = _ESCAPESTART;
                    break;
                }
                break;
            }
            case _ESCAPESTART: {
                if (isDigitalCharacter(c) || isLetter(c)) {
                    // [0-9a-z]{1,6}
                    for(int i = 0; i < 5; i ++) {
                        c = NextChar(m_buffer);
                        if (isDigitalCharacter(c) || isLetter(c)) {
                            continue;
                        } else {
                            break;
                        }
                    }
                } else if (c == '\r' || c == '\n' || c == '\f') {
                    // \\{nl}
                    // nl \n|\r\n|\r|\f
                } else {
                    // escape's \\[^\n\r\f0-9a-f]
                }
                status = _STRING;
                break;
            }
            case _STRING: {
                if((c != '\r' && c != '\n' && c != '\f' && c != stringBoundary) || (c & 0x80) || c == '\\') {
                    --m_forwardPos;
                    status = _START;
                } else {
                    token->type = STRING;
                    token->data = createData(m_firstPos, m_forwardPos);
                    return token;
                }
                break;
            }
            default: break;
        }
    }

    return token;
}

Lex::CSSToken *Lex::identToken() {
    CSSToken *token = new CSSToken;
    m_tokenCache.insert(token);
    token->type = IDENT;
    CSSDFAStatus STATUS = Start;
    bool stopLoop = false;
    while(1) {
        if (m_forwardPos >= m_bufferSize) {
            STATUS = end;
            break;
        }
        unsigned char c = NextChar(m_buffer);
        switch (STATUS) {
            case Start: {
                if(c == IDENT_START_SIGN) {
                    STATUS = iDentStart;
                } else if(isLetter(c) || c == UNDER_LINE_SIGN || (c & 0x80)) {
                    STATUS = NMStart;
                } else if(c == BACK_SPLASH) {
                    STATUS = EscapeStartInNMStart;
                } else if(isWs(c)) {
                    STATUS = Ws;
                } else if(c == HASH_SIGN) {
                    STATUS = HashStart;
                } else if(c == KEYWORD_SIGN) {
                    STATUS = AtKeyWordStart;
                } else {
                    STATUS = LexError;
                    stopLoop = true;
                }
                break;
            }
            case iDentStart: {
                if(isLetter(c) || c == UNDER_LINE_SIGN || (c & 0x80)) {
                    STATUS = NMStart;
                } else if(c == BACK_SPLASH) {
                    STATUS = EscapeStartInNMStart;
                } else {
                    STATUS = LexError;
                    stopLoop = true;
                }
                break;
            }
            case NMStart: {
                if(isLetter(c) || isDigitalCharacter(c) || c == UNDER_LINE_SIGN || c == IDENT_START_SIGN || (c &0x80)) {
                    STATUS = NMChar;
                } else if(c == BACK_SPLASH) {
                    STATUS = EscapeStartInNMChar;
                } else {
                    STATUS = iDent;
                    stopLoop = true;
                }
                break;
            }
            case NMChar: {
                if(isLetter(c) || isDigitalCharacter(c) || c == UNDER_LINE_SIGN || c == IDENT_START_SIGN || (c &0x80)) {
                    STATUS = NMChar;
                } else if(c == BACK_SPLASH) {
                    STATUS = EscapeStartInNMChar;
                } else {
                    STATUS = iDent;
                    stopLoop = true;
                }
                break;
            }
            case EscapeStartInNMStart: {
                if(isHexCharacter(c)) {
                    for(int i = 0; i < 5; i++) {
                        c = NextChar(m_buffer);
                        if(c == 0) {
                            break;
                        }
                        if(isHexCharacter(c)) {
                            continue;
                        } else {
                            --m_forwardPos;
                            break;
                        }
                    }
                    STATUS = NMStart;
                }
                break;
            }
            case EscapeStartInNMChar: {
                if(isHexCharacter(c)) {
                    for(int i = 0; i < 5; i++) {
                        c = NextChar(m_buffer);
                        if(c == 0) {
                            break;
                        }
                        if(isHexCharacter(c)) {
                            continue;
                        } else {
                            --m_forwardPos;
                            break;
                        }
                    }
                    STATUS = NMChar;
                }
                break;
            }
            default: {
                STATUS = LexError;
                break;
            }
        }
        if(stopLoop) {
            --m_forwardPos;
            break;
        }
    }
    token->data = createData(m_firstPos, m_forwardPos);
    if(STATUS == iDent) {
        token->type = IDENT;
    } else if(STATUS == end) {
        token->type = END;
    } else {
        token->type = ERROR;
    }
    m_firstPos = m_forwardPos;
    return token;
}

Lex::CSSToken *Lex::numberToken() {
    enum {
        _start,
        _numberStart,
        _numberFinish,
        _numberInDecimal,
        _error
    };
    TString s;
    char status = _start;
    char c = NextChar(m_buffer);
    if (c == 0) {
        return nullptr;
    }
    TString data;
    while(1) {
        switch(status) {
            case _start: {
                if(!isDigitalCharacter(c)) {
                    status = _error;
                    break;
                } else {
                    status = _numberStart;
                    data.append(TString(1, c));
                }
                break;
            }
            case _numberStart: {
                if(!isDigitalCharacter(c)) {
                    if(c == '.') {
                        status = _numberInDecimal;
                    } else {
                        status = _numberFinish;
                        break;
                    }
                }
                data.append(TString(1, c));
                break;
            }
            case _numberInDecimal:{
                if(!isDigitalCharacter(c)) {
                    status = _numberFinish;
                }
                data.append(TString(1, c));
                break;
            }
            default: {
                status = _error;
                break;
            }
        }
        if(status == _numberFinish ||
            status == _error) {
            --m_forwardPos;
            break;
        }
        if(m_bufferSize <= m_forwardPos + 1) {
            //status = _numberFinish;
            break;
        }
        c = NextChar(m_buffer);
    }

    if(data.size() == 0) {
        return NULL;
    } else {
        CSSToken* token = new CSSToken;
        m_tokenCache.insert(token);
        token->type = NUMBER;
        token->data = data;
        return token;
    }
}

Lex::CSSToken *Lex::token() {
    CSSToken *token = new CSSToken;
    m_tokenCache.insert(token);
    token->type = IDENT;
    m_firstPos = m_forwardPos;
    bool stopLoop = false;
    TString data;
    static CSSDFAStatus STATUS;
    if (m_firstPos >= m_bufferSize || !m_buffer) {
        token->type = END;
        return token;
    }
    while(1) {
        char c = NextChar(m_buffer);
        switch (STATUS) {
            case Start: {
                switch (c) {
                    case '@': {
                        CSSToken* identToken = Lex::identToken();
                        stopLoop = true;
                        if(identToken->type == IDENT) {
                            STATUS = AtKeyWord;
                            data = identToken->data;
                        } else {
                            STATUS = LexError;
                        }
                        break;
                    }
                    case '#': {
                        CSSToken *identToken = Lex::identToken();
                        stopLoop = true;
                        if(identToken->type == IDENT) {
                            STATUS = Hash;
                            data = identToken->data;
                        } else {
                            STATUS = LexError;
                        }
                        break;
                    }
                    case '~': {
                        if(NextChar(m_buffer) == EQUAL_SIGN) {
                            STATUS = include;
                        } else {
                            STATUS = tidle;
                            --m_forwardPos;
                        }
                        break;
                    }
                    case '|': {
                        if(NextChar(m_buffer) == EQUAL_SIGN) {
                            STATUS = dashMatch;
                        } else {
                            STATUS = LexError;
                        }
                        break;
                    }
                    case '^': {
                        if(NextChar(m_buffer) == EQUAL_SIGN) {
                            STATUS = prefixMatch;
                        } else {
                            STATUS = LexError;
                        }
                        break;
                    }
                    case '$': {
                        if(NextChar(m_buffer) == EQUAL_SIGN) {
                            STATUS = suffixMatch;
                        } else {
                            STATUS = LexError;
                        }
                        break;
                    }
                    case '*': {
                        if(NextChar(m_buffer) == EQUAL_SIGN) {
                            STATUS = subStringMatch;
                        } else {
                            --m_forwardPos;
                            STATUS = star;
                        }
                        break;
                    }
                    case '"': {
                        STATUS = string1Start;
                        break;
                    }
                    case '\'': {
                        STATUS = string2Start;
                        break;
                    }
                    case WS_CASE: {
                        STATUS = Ws;
                        break;
                    }
                    case '.': {
                        STATUS = dot;
                        stopLoop = true;
                        break;
                    }
                    case '{': {
                        STATUS = blockStart;
                        stopLoop = true;
                        break;
                    }
                    case '}': {
                        STATUS = blockEnd;
                        stopLoop = true;
                        break;
                    }
                    case ',': {
                        STATUS = comma;
                        stopLoop = true;
                        break;
                    }
                    case '>': {
                        STATUS = greater;
                        stopLoop = true;
                        break;
                    }
                    case '+': {
                        STATUS = plus;
                        stopLoop = true;
                        break;
                    }
                    case '[': {
                        STATUS = leftSqureBracket;
                        break;
                    }
                    case ']': {
                        STATUS = rightSqureBracket;
                        break;
                    }
                    case ':': {
                        STATUS = colon;
                        break;
                    }
                    case '=': {
                        STATUS = equal;
                        break;
                    }
                    case '/': {
                        char cn = NextChar(m_buffer);
                        if(cn == '*') {
                            STATUS = annotationStart;
                        } else {
                            --m_forwardPos;
                            goto identLabel;
                        }
                        break;
                    }
                    case ';' : {
                        STATUS = semicolon;
                        break;
                    }
                    case NUMBER_CASE: {
                        --m_forwardPos;
                        STATUS = numberStart;
                        break;
                    }
                    case ')': {
                        STATUS = rightBracket;
                        break;
                    }
                    case '-': {
                        STATUS = minus;
                        break;
                    }

                    default: {
                    identLabel:
                        m_forwardPos = m_firstPos;
                        CSSToken* idToken = identToken();
                        char next = NextChar(m_buffer);
                        if(next == '(') {
                            STATUS = function;
                            data = idToken->data;
                        } else {
                            --m_forwardPos;
                            if(idToken->type == IDENT) {
                                STATUS = iDent;
                                data = idToken->data;
                            } else if(idToken->type == END) {
                                STATUS = end;
                            } else {
                                STATUS = LexError;
                            }
                        }
                        stopLoop = true;
                        break;
                    }
                }
                break;
            }
            case Ws: {
                stopLoop = true;
                if (isWs(c)) {
                    STATUS = Ws;
                    stopLoop = false;
                } else if (c == PLUS_SIGN) {
                    STATUS = plus;
                } else if (c == COMMA_SIGN) {
                    STATUS = comma;
                } else if (c == GREATER_SIGN) {
                    STATUS = greater;
                } else  if (c == TIDLE_SIGN) {
                    STATUS = tidle;
                } else if ( c == '\0') {
                      STATUS = end;
                } else  {
                    --m_forwardPos;
                    STATUS = Ws;
                }
                break;
            }
            case include: {
                stopLoop = true;
                break;
            }
            case blockStart: {
                if (c == BLOCK_END_SIGN) {
                    STATUS = blockEnd;
                }
                while (c != BLOCK_END_SIGN) {
                    if (m_forwardPos >= m_bufferSize) {
                        STATUS = LexError;
                        break;
                    } else {
                        c = NextChar(m_buffer);
                        STATUS = blockEnd;
                    }
                }
                data = createData(m_firstPos, m_forwardPos);
                stopLoop = true;
                break;
            }
            case string1Start: {
                CSSToken *stringToken = textToken(1);
                if (stringToken->type == STRING) {
                    STATUS = string1End;
                    data = stringToken->data;
                } else {
                    STATUS = LexError;
                }
                break;
            }
            case string2Start: {
                CSSToken *stringToken = textToken(2);
                if (stringToken->type == STRING) {
                    STATUS = string2End;
                    data = stringToken->data;
                } else {
                    STATUS = LexError;
                }
                break;
            }
            case annotationStart: {
                char cn = NextChar(m_buffer);
                if (cn == 0) {break;}
                if (c == '*' && cn == '/') {
                    STATUS = annotationEnd;
                } else {
                    --m_forwardPos;
                    continue;
                }
                break;
            }
            case numberStart: {
                --m_forwardPos;
                CSSToken *numberToken = Lex::numberToken();
                if (numberToken) {
                    data = numberToken->data;
                    STATUS = num;
                } else {
                    STATUS = LexError;
                }
                break;
            }

            default: {
                stopLoop = true;
            }
            break;
        }
        bool resetStatus = true;
        switch(STATUS) {
            case AtKeyWord: {
                token->type = ATKEYWORD;
                token->data = data;
                break;
            }
            case LexError: {
                token->type = ERROR;
                stopLoop = true;
                break;
            }
            case Hash: {
                token->type = HASH;
                token->data = data;
                break;
            }
            case Ws: {
                token->type = WS;
                break;
            }
            case include: {
                token->type = INCLUDES;
                stopLoop = true;
                break;
            }
            case iDent: {
                token->type = IDENT;
                token->data = data;
                break;
            }
            case dot: {
                token->type = DOT;
                break;
            }
            case blockStart: {
                token->type = BLOCKSTART;
                resetStatus = false;
                break;
            }
            case blockEnd:{
                token->type = BLOCKEND;
                token->data = data;
                STATUS = Start;
                break;
            }
            case end: {
                token->type = END;
                resetStatus = true;
                break;
            }
            case comma: {
                token->type = COMMA;
                break;
            }
            case plus:{
                token->type = PLUS;
                break;
            }
            case tidle: {
                token->type = TIDLE;
                stopLoop = true;
                break;
            }
            case greater: {
                token->type = GREATER;
                stopLoop = true;
                break;
            }
            case star:{
                token->type = STAR;
                stopLoop = true;
                break;
            }
            case dashMatch:{
                token->type = DASHMATCH;
                stopLoop = true;
                break;
            }
            case prefixMatch:{
                token->type = PREFIXMATCH;
                stopLoop = true;
                break;
            }
            case suffixMatch:{
                token->type = SUFFIXMATCH;
                stopLoop = true;
                break;
            }
            case includes:{
                token->type = INCLUDES;
                stopLoop = true;
                break;
            }
            case subStringMatch:{
                token->type = SUBSTRINGMATCH;
                stopLoop = true;
                break;
            }
            case leftSqureBracket: {
                token->type = LEFTSQUREBRACKET;
                stopLoop = true;
                break;
            }
            case rightSqureBracket: {
                token->type = RIGHTSQUREBRACKET;
                stopLoop = true;
                break;
            }
            case colon: {
                token->type = COLON;
                stopLoop = true;
                break;
            }
            case equal: {
                token->type = EQUAL;
                stopLoop = true;
                break;
            }
            case semicolon: {
                token->type =SYNTAXEND;
                stopLoop = true;
                break;
            }
            case string1End: case string2End: {
                token->type = STRING;
                token->data = data;
                stopLoop = true;
                break;
            }
            case annotationEnd: {
                token->type = ANNOTATION;
                stopLoop = true;
                break;
            }
            case function: {
                token->type = FUNCTION;
                token->data = data;
                stopLoop = true;
                break;
            }
            case numberStart: {
                stopLoop = false;
                break;
            }
            case num: {
                token->data = data;
                token->type = NUMBER;
                stopLoop = true;
                break;
            }
            case rightBracket: {
                token->type = RIGHTBRACKET;
                stopLoop = true;
                break;
            }
            case minus: {
                token->type = MINUS;
                stopLoop = true;
                break;
            }
            default:
                break;
        }
        if(stopLoop) {
            if(resetStatus) {
                STATUS = Start;
            }
            break;
        }
    }
    return token;
}

TString Lex::createData(size_t start, size_t end) {
    if(m_buffer == NULL || start > end) {
        return TString();
    }

    size_t size = end - start + 1;
    if(m_bufferSize < end) {
        return TString();
    }

    char *ptr = new char[size];
    ptr[size -1] = '\0';
    memmove(ptr, m_buffer + start, size - 1);
    TString ret = ptr;
    delete [] ptr;

    return ret;
}

void Lex::cleanResource() {
    for(auto it : m_tokenCache) {
        delete it;
    }

    delete [] m_buffer;
    m_buffer = 0;
}

inline bool Lex::isDigitalCharacter(char c) {
    return (c > 0x2F && c < 0x3A);
}

inline bool Lex::isLetter(char c) {
    return (c > 0x60 && c < 0x7B) || (c > 0x40 && c < 0x5B);
}

inline bool Lex::isHexCharacter(char c) {
    return isDigitalCharacter(c) || (c > 0x60 && c < 0x67) || (c > 0x40 && c < 0x47);
}

inline bool Lex::isWs(char c) {
    return (c == ' ' || c == '\r' || c == '\n' || c == '\f' || c == '\t');
}

