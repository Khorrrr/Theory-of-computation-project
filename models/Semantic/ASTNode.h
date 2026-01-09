#ifndef ASTNODE_H
#define ASTNODE_H

#include <QString>
#include <QVector>

 
enum class ASTNodeType {
    PROGRAM,
    DECLARATION,
    ASSIGNMENT,
    EXPRESSION,
    IDENTIFIER,
    LITERAL,
    BINARY_OP,
    FUNCTION_DECL,
    FUNCTION_CALL,
    IF_STATEMENT,
    BLOCK,
    UNKNOWN
};

class ASTNode {
private:
    ASTNodeType type;
    QString value;
    int line;
    QVector<ASTNode*> children;
    ASTNode* parent;

public:

    ASTNode(ASTNodeType nodeType, const QString& nodeValue = "", int lineNumber = 0);
    

    ~ASTNode();


    void addChild(ASTNode* child);
    void removeChild(ASTNode* child);
    void setParent(ASTNode* parentNode);


    ASTNodeType getType() const { return type; }
    QString getValue() const { return value; }
    int getLine() const { return line; }
    QVector<ASTNode*> getChildren() const { return children; }
    ASTNode* getParent() const { return parent; }
    int getChildCount() const { return children.size(); }


    void setValue(const QString& val) { value = val; }
    void setLine(int ln) { line = ln; }


    QString getTypeString() const;
    QString toString() const;
    

    void print(int indent = 0) const;
};

#endif
