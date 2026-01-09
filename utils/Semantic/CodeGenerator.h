#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include "./models/LexicalAnalysis/Token.h"
#include "./models/Semantic/SymbolTable.h"
#include <QString>
#include <QVector>
#include <QProcess>

enum class TargetLanguage {
    PYTHON,
    JAVA,
    JAVASCRIPT,
    ASSEMBLY
};

class CodeGenerator {
private:
    QVector<Token> tokens;
    SymbolTable* symbolTable;
    TargetLanguage targetLanguage;
    QString generatedCode;
    QString m_sourceCode;

     
    int indentLevel;
    int currentPosition;
    int labelCounter;
    bool inGlobalScope;  

public:
    CodeGenerator();
    ~CodeGenerator();

    void setTokens(const QVector<Token>& toks);
    void setSymbolTable(SymbolTable* table);
    void setTargetLanguage(TargetLanguage lang);

    void setSourceCode(const QString& source);

    QString generate();
    QString getGeneratedCode() const { return generatedCode; }
    void reset();

private:
     
    QString translateToPython();
    QString translateToJava();
    QString translateToJavaScript();
    QString translateToAssembly();



     
    void optimizeTokens();

     
    void processStatement(QString& code);
    void processBlock(QString& code);

     
    void processFunctionDeclaration(QString& code);
    bool isFunctionDeclaration() const;

     
    void processIfStatement(QString& code);
    void processWhileLoop(QString& code);
    void processForLoop(QString& code);

     
    void processDeclaration(QString& code);
    void processAssignment(QString& code);
    void processExpression(QString& result);
    void processCondition(QString& result);

     
    void processCout(QString& code);
    void processCin(QString& code);

     
    void processPreprocessor(QString& code);

     
    QString getIndent() const;
    QString extractLoopVariable(const QString& init);
    QString convertConditionToRange(const QString& condition, const QString& init);
    QString generateLabel();
    QString mapType(const QString& cppType);  
    bool isTypeKeyword(const QString& keyword) const;  

     
    Token peek() const;
    Token peekAhead(int offset) const;  
    Token advance();
    bool match(TokenType type);
    bool check(TokenType type) const;
    bool isAtEnd() const;
    bool isControlStructure() const;

     
    void skipToNextStatement();
    void synchronize();
};

#endif  
