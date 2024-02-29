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
