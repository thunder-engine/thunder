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
//  CSSParserStatus.h
//  DDCSSParser
//
//  Created by 1m0nster on 2018/8/7.
//  Copyright © 2018 1m0nster. All rights reserved.
//

#ifndef CSSParserStatus_h
#define CSSParserStatus_h

//enum CSSParserStatus {
//    START,
//    KEYWORD,
//    TYPESELECTOR,
//    UNIVERSIALSELECTOR,
//    IDSELECTOR,
//    ATTRIBUTSELECTOR,
//    CLASSSELECTOR,
//    SELECTORSEQUENCE,
//    PSEUDOSELECOT,
//    SELECTOR,
//    SELECTORGROUP,
//    RULESTART,
//    RULEEND,
//    PARSEERROR
//};

enum CSSParserStatus {
    START,
    INSELECTOR,
    STARTBLOCK,
    INATKEYWORD
};
extern const int HTMLTAGMAXSIZE;
extern const char* HTMLTagNames[];
#endif /* CSSParserStatus_h */
