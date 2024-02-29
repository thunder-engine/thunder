#ifndef CSSPARSER_H
#define CSSPARSER_H

#include "csslex.h"
#include "cssparserstatus.h"
#include "keyworditem.h"
#include "pseudoselector.h"
#include "signselector.h"

#include <stack>
#include <vector>
#include <list>

class Lex;
class Selector;

class CSSParser {
public:
    struct ASTNode {
        Selector *head;
        ASTNode *left;
        ASTNode *right;
        ASTNode() {
            head = NULL;
            left = NULL;
            right = NULL;
        }
    };
public:
    CSSParser();
    ~CSSParser();

    bool parseByString (const std::string& cssString);

    std::vector<Selector *> &selectors();
    std::vector<KeywordItem *> &keywords();

    void cleanRes();

private:
    typedef void(*treeTranverseAction)(ASTNode *);
    typedef CSSParser::ASTNode *(*treeTranverseWithUserDataAction)(std::stack<CSSParser::ASTNode *> *stack);
    friend CSSParser::ASTNode *TreeTranverseCreateExpressionAction(std::stack<CSSParser::ASTNode *> *);

    static void initialASTNode(ASTNode *target, Selector *head, ASTNode *left, ASTNode *right);
    static void pushOperatedElement(std::stack<ASTNode *> &, Selector *head);

    bool parse();
    void prepareByString(const std::string &cssString);
    void clean();

    bool startSelector(CSSTokenType);
    bool tokenHasInfo(CSSTokenType);
    bool topHaveSign(std::stack<Selector *> &);
    Selector* getSelector(Lex::CSSToken* token);
    PseudoSelector::Parameter *getFunctionParamenter();
    std::list<ASTNode *> createATS(std::stack<Selector *>&);
    void pushSign(std::stack<Selector *> &, SignSelector::SignType);
    void buildReversePolishNotation(std::stack<ASTNode*>& operatorStack, std::stack<ASTNode*>& operandStack);

    void RMLtranverseAST(ASTNode *root, treeTranverseAction action);
    void LRMtranverseAST(ASTNode *root, treeTranverseAction action);
    void LMRtranverseAST(ASTNode *root, treeTranverseAction action);
    void MLRtranverseAST(ASTNode *root, treeTranverseWithUserDataAction action, void *userData);

private:
    Lex*                        m_lexer;
    CSSParserStatus             m_status;
    std::string                 m_hostCssFile;
    std::vector<Selector *>     m_selectors;
    std::vector<KeywordItem *>  m_keywords;
    std::list<Selector *> 		m_signSelecors;
};

#endif /* CSSPARSER_H */
