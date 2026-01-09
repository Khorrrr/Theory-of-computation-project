#ifndef SEMANTICANALYZERWIDGET_H
#define SEMANTICANALYZERWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QRadioButton>
#include <QGroupBox>
#include <QButtonGroup>
#include <QTreeWidget>
#include "../src/utils/Semantic/SemanticAnalyzer.h"
#include "../src/utils/Semantic/CodeGenerator.h"
#include "../src/utils/LexicalAnalysis/Lexer.h"
#include "../src/utils/LexicalAnalysis/AutomatonManager.h"

#include <QCheckBox>

class SyntaxHighlighter;

class SemanticAnalyzerWidget : public QWidget {
    Q_OBJECT

private:
     
    QTextEdit* sourceCodeEdit;
    QPushButton* analyzeButton;
    QPushButton* generateCodeButton;
    QPushButton* clearButton;
    QPushButton* copyCodeButton;

    QComboBox* targetLanguageCombo;

    QTableWidget* symbolTableWidget;
    QTextEdit* errorsWarningsText;
    QTextEdit* translatedCodeEdit;
    

    QTreeWidget* astTreeWidget;

    QLabel* statusLabel;


    SemanticAnalyzer* semanticAnalyzer;
    CodeGenerator* codeGenerator;
    Lexer* lexer;
    AutomatonManager* automatonManager;
    SyntaxHighlighter* syntaxHighlighter;

public:
    explicit SemanticAnalyzerWidget(QWidget *parent = nullptr);
    ~SemanticAnalyzerWidget();

    void setAutomatonManager(AutomatonManager* manager);

private slots:
    void onAnalyzeClicked();
    void onGenerateCodeClicked();
    void onClearClicked();
    void onCopyCodeClicked();


private:
    void setupUI();
    void createConnections();
    void displaySymbolTable();
    void displayErrorsWarnings();
    void displayTranslatedCode(const QString& code);
    void displayAST();
    void highlightInlineErrors();
    void populateASTTree(QTreeWidgetItem* parent, ASTNode* node);
    void highlightErrorAtLine(int line, bool isError);
};

#endif
