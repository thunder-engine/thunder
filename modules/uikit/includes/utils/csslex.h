/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#ifndef CSSLEX_H
#define CSSLEX_H

#include <set>

#include <astring.h>

#include "csslexstatus.h"

class Lex {
public:
    struct CSSToken {
        CSSTokenType type;
        TString data;
    };

public:
    Lex();
    ~Lex();
    CSSToken *token();
    void cleanResource();
    void setBufferString(const TString &bufferString);

private:
    CSSToken *identToken();
    CSSToken *numberToken();
    CSSToken *textToken(char stringType);

    bool isDigitalCharacter(char);
    bool isLetter(char);
    bool isHexCharacter(char);
    bool isWs(char);

    TString createData(size_t start, size_t end);

private:
    std::set<CSSToken *> m_tokenCache;
    TString m_fileName;

    char *m_buffer;

    size_t m_bufferSize;
    size_t m_firstPos;
    size_t m_forwardPos;

};

#endif /* CSSLEX_H */
