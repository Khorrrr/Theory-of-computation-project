#include "./src/models/Semantic/ASTNode.h"
#include <QDebug>

ASTNode::ASTNode(ASTNodeType nodeType, const QString& nodeValue, int lineNumber)
    : type(nodeType), value(nodeValue), line(lineNumber), parent(nullptr) {
}

ASTNode::~ASTNode() {
     
    for (ASTNode* child : children) {
        if (child) {
            delete child;
        }
    }
    children.clear();
}

void ASTNode::addChild(ASTNode* child) {
    if (child) {
        children.append(child);
        child->setParent(this);
    }
}

void ASTNode::removeChild(ASTNode* child) {
    if (child) {
        children.removeOne(child);
        child->setParent(nullptr);
    }
}

void ASTNode::setParent(ASTNode* parentNode) {
    parent = parentNode;
}

QString ASTNode::getTypeString() const {
    switch (type) {
    case ASTNodeType::PROGRAM:
        return "Program";
    case ASTNodeType::DECLARATION:
        return "Declaration";
    case ASTNodeType::ASSIGNMENT:
        return "Assignment";
    case ASTNodeType::EXPRESSION:
        return "Expression";
    case ASTNodeType::IDENTIFIER:
        return "Identifier";
    case ASTNodeType::LITERAL:
        return "Literal";
    case ASTNodeType::BINARY_OP:
        return "Binary Operation";
    case ASTNodeType::FUNCTION_DECL:
        return "Function Declaration";
    case ASTNodeType::FUNCTION_CALL:
        return "Function Call";
    case ASTNodeType::IF_STATEMENT:
        return "If Statement";
    case ASTNodeType::BLOCK:
        return "Block";
    default:
        return "Unknown";
    }
}

QString ASTNode::toString() const {
    QString result = getTypeString();
    if (!value.isEmpty()) {
        result += QString(" (%1)").arg(value);
    }
    if (line > 0) {
        result += QString(" [Line %1]").arg(line);
    }
    return result;
}

void ASTNode::print(int indent) const {
    QString indentStr = QString(indent * 2, ' ');
    qDebug().noquote() << indentStr + toString();
    
    for (const ASTNode* child : children) {
        if (child) {
            child->print(indent + 1);
        }
    }
}
