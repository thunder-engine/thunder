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
//
//  CSSLexStatus.h
//  DDCSSParser
//
//  Created by 1m0nster on 2018/8/1.
//  Copyright © 2018 1m0nster. All rights reserved.
//

#ifndef CSSLexStatus_h
#define CSSLexStatus_h

enum CSSDFAStatus {
    Start,
    iDentStart,
    iDent,
    NMStart,
    NMChar,
    EscapeStartInNMStart,
    EscapeStartInNMChar,
    EscapeStartInHash,
    EscapeStartInATKeyword,
    HashStart,
    Hash,
    Ws,
    AtKeyWordStart,
    AtKeyWord,
    include,
    dot,
    end,
    blockStart,
    blockEnd,
    comma,
    plus,
    greater,
    tidle,
    dashMatch,
    prefixMatch,
    suffixMatch,
    subStringMatch,
    includes,
    star,
    colon,
    semicolon,
    leftSqureBracket,
    rightSqureBracket,
    equal,
    string1Start,
    string1End,
    string2Start,
    string2End,
    annotationStart,
    annotationEnd,
    function,
    numberStart,
    num,
    rightBracket,
    minus,
    LexError
};
enum CSSTokenType {
    INCLUDES,
    DASHMATCH,
    PREFIXMATCH,
    SUFFIXMATCH,
    SUBSTRINGMATCH,
    IDENT,
    STRING,
    FUNCTION,
    NUMBER,
    HASH,
    PLUS,
    GREATER,
    COMMA,
    TIDLE,
    ATKEYWORD,
    STAR,
    PERCENTAGE,
    DIMENSION,
    CDO,
    CDC,
    WS,
    DOT,
    ERROR,
    BLOCKSTART,
    BLOCKEND,
    COLON,
    LEFTSQUREBRACKET,
    RIGHTSQUREBRACKET,
    EQUAL,
    ANNOTATION,
    SYNTAXEND,
    RIGHTBRACKET,
    MINUS,
    END
};
#define IDENT_START_SIGN    '-'
#define UNDER_LINE_SIGN     '_'
#define BACK_SPLASH         '\\'
#define HASH_SIGN           '#'
#define KEYWORD_SIGN        '@'
#define BLOCK_START_SIGN    '{'
#define BLOCK_END_SIGN      '}'
#define EQUAL_SIGN          '='
#define COMMA_SIGN          ','
#define PLUS_SIGN           '+'
#define TIDLE_SIGN          '~'
#define GREATER_SIGN        '>'
#define COLON_SIGN          ':'
#define LEFT_SQURE_BRACKET  '['
#define RIGHT_SQURE_BRACKET ']'


#endif /* CSSLexStatus_h */
