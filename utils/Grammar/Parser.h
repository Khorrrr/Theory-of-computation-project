#ifndef PARSER_H
#define PARSER_H

#include "./src/models/Grammar/Grammar.h"
#include "./src/models/Grammar/ParseTree.h"
#include "./src/models/LexicalAnalysis/Token.h"
#include <QVector>
#include <QString>

struct ParseError {
    QString message;
    int position;
    QString expected;
    QString found;

    ParseError(const QString& msg, int pos, const QString& exp, const QString& fnd)
        : message(msg), position(pos), expected(exp), found(fnd) {}

    QString toString() const {
        return QString("Parse Error at position %1: %2\nExpected: %3\nFound: %4")
        .arg(position).arg(message).arg(expected).arg(found);
    }
};

class Parser {
private:
    Grammar* grammar;
    QVector<Token> tokens;
    int currentPosition;
    QVector<ParseError> errors;
    std::shared_ptr<ParseTreeNode> currentNode;

public:
    Parser();
    Parser(Grammar* g);
    ~Parser();

    void setGrammar(Grammar* g);
    void setTokens(const QVector<Token>& toks);

    ParseTree parse();
    ParseTree parseExpression();   

    QVector<ParseError> getErrors() const { return errors; }
    bool hasErrors() const { return !errors.isEmpty(); }

    void reset();

private:
     
    std::shared_ptr<ParseTreeNode> parseE();       
    std::shared_ptr<ParseTreeNode> parseEPrime();  
    std::shared_ptr<ParseTreeNode> parseT();       
    std::shared_ptr<ParseTreeNode> parseTPrime();  
    std::shared_ptr<ParseTreeNode> parseF();       

     
    Token peek() const;
    Token advance();
    bool match(const QString& expected);
    bool check(const QString& expected) const;
    bool isAtEnd() const;

    void addError(const QString& message, const QString& expected);
    QString getCurrentTokenString() const;
};

#endif  
