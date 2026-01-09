#include "SemanticAnalyzerWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QSplitter>
#include <QHeaderView>
#include <QMessageBox>
#include <QTextBlock>
#include <QTextDocument>
#include <QClipboard>
#include <QApplication>
#include "../src/ui/SyntaxHighlighter.h"
#include <QTextDocument>

SemanticAnalyzerWidget::SemanticAnalyzerWidget(QWidget *parent)
    : QWidget(parent), automatonManager(nullptr) {

    semanticAnalyzer = new SemanticAnalyzer();
    codeGenerator = new CodeGenerator();
    lexer = new Lexer();
    syntaxHighlighter = nullptr;

    setupUI();
    createConnections();
}

SemanticAnalyzerWidget::~SemanticAnalyzerWidget() {
    delete semanticAnalyzer;
    delete codeGenerator;
    delete lexer;
}

void SemanticAnalyzerWidget::setAutomatonManager(AutomatonManager* manager) {
    automatonManager = manager;
    if (lexer) {
        lexer->setAutomatonManager(manager);
    }
}

void SemanticAnalyzerWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

     
    QLabel* titleLabel = new QLabel("Semantic Analyzer & Code Generator");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

     
    QSplitter* mainSplitter = new QSplitter(Qt::Horizontal);

     
    QWidget* leftPanel = new QWidget();
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);

    QGroupBox* sourceGroup = new QGroupBox("Source Code");
    QVBoxLayout* sourceLayout = new QVBoxLayout();

    sourceCodeEdit = new QTextEdit();
    sourceCodeEdit->setPlaceholderText("Enter your source code here...");
    sourceCodeEdit->setStyleSheet(
        "QTextEdit { background-color: #1e1e1e; color: #d4d4d4; "
        "font-family: 'Courier New', monospace; font-size: 10pt; "
        "border: 1px solid #3e3e3e; padding: 5px; }"
    );
    sourceCodeEdit->setText(
        "int x = 10;\n"
        "float y = 3.14;\n"
        "int z = x + 5;\n"
        "char c = 'A';\n"
        "bool flag = true;"
        );
    
     
    syntaxHighlighter = new SyntaxHighlighter(sourceCodeEdit->document());
    
    sourceLayout->addWidget(sourceCodeEdit);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    analyzeButton = new QPushButton("🔍 Analyze");
    analyzeButton->setStyleSheet(
        "QPushButton { padding: 8px 20px; font-size: 11pt; font-weight: bold; "
        "background-color: #0078d7; color: white; border: none; border-radius: 3px; }"
        "QPushButton:hover { background-color: #005a9e; }"
        );

    generateCodeButton = new QPushButton("🚀 Generate Code");
    generateCodeButton->setStyleSheet(
        "QPushButton { padding: 8px 20px; font-size: 11pt; font-weight: bold; "
        "background-color: #28a745; color: white; border: none; border-radius: 3px; }"
        "QPushButton:hover { background-color: #218838; }"
        );
    generateCodeButton->setEnabled(false);

    clearButton = new QPushButton("🗑️ Clear");
    clearButton->setStyleSheet("QPushButton { padding: 8px 20px; }");

    buttonLayout->addWidget(analyzeButton);
    buttonLayout->addWidget(generateCodeButton);
    buttonLayout->addWidget(clearButton);
    sourceLayout->addLayout(buttonLayout);

    sourceGroup->setLayout(sourceLayout);
    leftLayout->addWidget(sourceGroup);

     
    QGroupBox* symbolGroup = new QGroupBox("Symbol Table");
    symbolGroup->setCheckable(true);
    symbolGroup->setChecked(true);
    QVBoxLayout* symbolLayout = new QVBoxLayout();

    symbolTableWidget = new QTableWidget();
    symbolTableWidget->setColumnCount(5);
    symbolTableWidget->setHorizontalHeaderLabels({"Name", "Type", "Value", "Scope", "Status"});
    symbolTableWidget->setStyleSheet(
        "QTableWidget { background-color: #1e1e1e; color: white; gridline-color: #3e3e3e; }"
        "QTableWidget::item { color: white; padding: 5px; }"
        "QTableWidget::item:selected { background-color: #0078d7; }"
        "QHeaderView::section { background-color: #2d2d2d; color: white; padding: 5px; "
        "border: 1px solid #3e3e3e; font-weight: bold; }"
        );
    symbolTableWidget->horizontalHeader()->setStretchLastSection(true);
    symbolTableWidget->setAlternatingRowColors(true);
    symbolTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    symbolLayout->addWidget(symbolTableWidget);

    symbolGroup->setLayout(symbolLayout);
    leftLayout->addWidget(symbolGroup);

     
    QGroupBox* astGroup = new QGroupBox("Abstract Syntax Tree (AST)");
    astGroup->setCheckable(true);
    astGroup->setChecked(true);
    QVBoxLayout* astLayout = new QVBoxLayout();

    astTreeWidget = new QTreeWidget();
    astTreeWidget->setHeaderLabel("AST Structure");
    astTreeWidget->setStyleSheet(
        "QTreeWidget { background-color: #1e1e1e; color: white; }"
        "QTreeWidget::item { color: white; padding: 3px; }"
        "QTreeWidget::item:selected { background-color: #0078d7; }"
        "QHeaderView::section { background-color: #2d2d2d; color: white; padding: 5px; "
        "border: 1px solid #3e3e3e; font-weight: bold; }"
        );
    astTreeWidget->setAlternatingRowColors(true);
    astLayout->addWidget(astTreeWidget);

    astGroup->setLayout(astLayout);
    leftLayout->addWidget(astGroup);

    mainSplitter->addWidget(leftPanel);

     
    QWidget* rightPanel = new QWidget();
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);

     
    QGroupBox* errorsGroup = new QGroupBox("Errors & Warnings");
    QVBoxLayout* errorsLayout = new QVBoxLayout();

    errorsWarningsText = new QTextEdit();
    errorsWarningsText->setReadOnly(true);
    errorsWarningsText->setMaximumHeight(150);
    errorsWarningsText->setStyleSheet("QTextEdit { background-color: #1e1e1e; color: white; }");
    errorsLayout->addWidget(errorsWarningsText);

    errorsGroup->setLayout(errorsLayout);
    rightLayout->addWidget(errorsGroup);

     
    QGroupBox* translateGroup = new QGroupBox("Code Generation");
    QVBoxLayout* translateLayout = new QVBoxLayout();

     
    QHBoxLayout* langLayout = new QHBoxLayout();
    langLayout->addWidget(new QLabel("Target Language:"));

    targetLanguageCombo = new QComboBox();
    targetLanguageCombo->addItem("Python", QVariant::fromValue(TargetLanguage::PYTHON));
    targetLanguageCombo->addItem("Java", QVariant::fromValue(TargetLanguage::JAVA));
    targetLanguageCombo->addItem("JavaScript", QVariant::fromValue(TargetLanguage::JAVASCRIPT));
    targetLanguageCombo->addItem("Assembly", QVariant::fromValue(TargetLanguage::ASSEMBLY));
    langLayout->addWidget(targetLanguageCombo);
    langLayout->addStretch();

    translateLayout->addLayout(langLayout);

    translatedCodeEdit = new QTextEdit();
    translatedCodeEdit->setReadOnly(true);
    translatedCodeEdit->setStyleSheet(
        "QTextEdit { background-color: #1e1e1e; color: #4ec9b0; "
        "font-family: 'Courier New', monospace; font-size: 10pt; }"
        );
    translateLayout->addWidget(translatedCodeEdit);
    
     
    copyCodeButton = new QPushButton("📋 Copy Code");
    copyCodeButton->setStyleSheet(
        "QPushButton { padding: 6px 15px; font-size: 10pt; "
        "background-color: #6c757d; color: white; border: none; border-radius: 3px; }"
        "QPushButton:hover { background-color: #5a6268; }"
        );
    copyCodeButton->setEnabled(false);
    translateLayout->addWidget(copyCodeButton);

    translateGroup->setLayout(translateLayout);
    rightLayout->addWidget(translateGroup);

    mainSplitter->addWidget(rightPanel);
    mainLayout->addWidget(mainSplitter);

     
    statusLabel = new QLabel("Ready - Enter source code and click Analyze");
    statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #e9ecef; border-radius: 3px; }");
    mainLayout->addWidget(statusLabel);

    setLayout(mainLayout);
}

void SemanticAnalyzerWidget::createConnections() {
    connect(analyzeButton, &QPushButton::clicked, this, &SemanticAnalyzerWidget::onAnalyzeClicked);
    connect(generateCodeButton, &QPushButton::clicked, this, &SemanticAnalyzerWidget::onGenerateCodeClicked);
    connect(clearButton, &QPushButton::clicked, this, &SemanticAnalyzerWidget::onClearClicked);
    connect(copyCodeButton, &QPushButton::clicked, this, &SemanticAnalyzerWidget::onCopyCodeClicked);
}


void SemanticAnalyzerWidget::onAnalyzeClicked() {
    QString sourceCode = sourceCodeEdit->toPlainText().trimmed();

    if (sourceCode.isEmpty()) {
        QMessageBox::warning(this, "Empty Input", "Please enter some source code to analyze.");
        return;
    }

     
    lexer->setSkipWhitespace(true);
    lexer->setSkipComments(true);

    if (!lexer->tokenize(sourceCode)) {
        QMessageBox::critical(this, "Tokenization Failed",
                              "Failed to tokenize source code. Check for lexical errors.");
        return;
    }

     
    semanticAnalyzer->setTokens(lexer->getTokens());
    bool success = semanticAnalyzer->analyzeProgram();

     
    displaySymbolTable();
    displayErrorsWarnings();
    displayAST();
    highlightInlineErrors();

    if (success && !semanticAnalyzer->hasErrors()) {
        generateCodeButton->setEnabled(true);
        statusLabel->setText("✅ Semantic analysis passed - Ready to generate code");
        statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #d4edda; color: #155724; border-radius: 3px; }");
    } else {
        generateCodeButton->setEnabled(false);
        statusLabel->setText(QString("❌ Semantic analysis found %1 error(s)")
                                 .arg(semanticAnalyzer->getErrors().size()));
        statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #f8d7da; color: #721c24; border-radius: 3px; }");
    }
}

void SemanticAnalyzerWidget::onGenerateCodeClicked() {
    TargetLanguage targetLang = targetLanguageCombo->currentData().value<TargetLanguage>();

    codeGenerator->setTokens(lexer->getTokens());
    codeGenerator->setSymbolTable(semanticAnalyzer->getSymbolTable());
    codeGenerator->setTargetLanguage(targetLang);
    codeGenerator->setSourceCode(sourceCodeEdit->toPlainText());  

    QString translatedCode = codeGenerator->generate();
    displayTranslatedCode(translatedCode);
    
    copyCodeButton->setEnabled(!translatedCode.isEmpty());

    statusLabel->setText(QString("✅ Code generated for %1").arg(targetLanguageCombo->currentText()));
    statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #d4edda; color: #155724; border-radius: 3px; }");
}

void SemanticAnalyzerWidget::onClearClicked() {
    sourceCodeEdit->clear();
    symbolTableWidget->setRowCount(0);
    errorsWarningsText->clear();
    translatedCodeEdit->clear();
    astTreeWidget->clear();
    generateCodeButton->setEnabled(false);
    copyCodeButton->setEnabled(false);
    statusLabel->setText("Ready");
    statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #e9ecef; border-radius: 3px; }");
}

void SemanticAnalyzerWidget::onCopyCodeClicked() {
    QString code = translatedCodeEdit->toPlainText();
    if (!code.isEmpty()) {
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(code);
        
        statusLabel->setText("📋 Code copied to clipboard!");
        statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #d1ecf1; color: #0c5460; border-radius: 3px; }");
    }
}

void SemanticAnalyzerWidget::displaySymbolTable() {
    symbolTableWidget->setRowCount(0);

    if (!semanticAnalyzer) return;

    QVector<Symbol> symbols = semanticAnalyzer->getDiscoveredSymbols();

    for (const auto& symbol : symbols) {
        int row = symbolTableWidget->rowCount();
        symbolTableWidget->insertRow(row);

        QTableWidgetItem* nameItem = new QTableWidgetItem(symbol.name);
        QTableWidgetItem* typeItem = new QTableWidgetItem(symbol.getTypeString());
        QTableWidgetItem* valueItem = new QTableWidgetItem(
            symbol.isInitialized ? symbol.value : "(uninitialized)"
            );
        QTableWidgetItem* scopeItem = new QTableWidgetItem(QString::number(symbol.scope));
        QTableWidgetItem* statusItem = new QTableWidgetItem(
            symbol.isInitialized ? "Initialized" : "Declared"
            );

         
        nameItem->setForeground(Qt::white);
        typeItem->setForeground(Qt::white);
        valueItem->setForeground(Qt::white);
        scopeItem->setForeground(Qt::white);
        statusItem->setForeground(Qt::white);

        QColor bgColor = symbol.isInitialized ? QColor(50, 150, 50) : QColor(150, 120, 50);
        nameItem->setBackground(bgColor);
        typeItem->setBackground(bgColor);
        valueItem->setBackground(bgColor);
        scopeItem->setBackground(bgColor);
        statusItem->setBackground(bgColor);

        symbolTableWidget->setItem(row, 0, nameItem);
        symbolTableWidget->setItem(row, 1, typeItem);
        symbolTableWidget->setItem(row, 2, valueItem);
        symbolTableWidget->setItem(row, 3, scopeItem);
        symbolTableWidget->setItem(row, 4, statusItem);
    }
}

void SemanticAnalyzerWidget::displayErrorsWarnings() {
    errorsWarningsText->clear();

    QString output;

    QVector<SemanticError> errors = semanticAnalyzer->getErrors();
    QVector<SemanticError> warnings = semanticAnalyzer->getWarnings();

    if (errors.isEmpty() && warnings.isEmpty()) {
        output = "<span style='color: #4ec9b0;'><b>✅ No errors or warnings!</b></span>";
    } else {
        if (!errors.isEmpty()) {
            output += "<b style='color: #f48771;'>Errors:</b><br>";
            for (const auto& error : errors) {
                output += QString("<span style='color: #f48771;'>• %1</span><br>")
                              .arg(error.toString());  
            }
            output += "<br>";
        }

        if (!warnings.isEmpty()) {
            output += "<b style='color: #ffc107;'>Warnings:</b><br>";
            for (const auto& warning : warnings) {
                output += QString("<span style='color: #ffc107;'>⚠ %1</span><br>")
                              .arg(warning.toString());  
            }
        }
    }

    errorsWarningsText->setHtml(output);
}

void SemanticAnalyzerWidget::displayTranslatedCode(const QString& code) {
    translatedCodeEdit->setPlainText(code);
}

void SemanticAnalyzerWidget::displayAST() {
    astTreeWidget->clear();

    if (!semanticAnalyzer) return;

    ASTNode* root = semanticAnalyzer->getAST();
    if (!root) {
        QTreeWidgetItem* item = new QTreeWidgetItem(astTreeWidget);
        item->setText(0, "No AST generated");
        item->setForeground(0, QColor("#999999"));
        return;
    }

    populateASTTree(nullptr, root);
    astTreeWidget->expandAll();
}

void SemanticAnalyzerWidget::populateASTTree(QTreeWidgetItem* parent, ASTNode* node) {
    if (!node) return;

    QTreeWidgetItem* item = nullptr;
    if (parent) {
        item = new QTreeWidgetItem(parent);
    } else {
        item = new QTreeWidgetItem(astTreeWidget);
    }

     
    item->setText(0, node->toString());
    
     
    QColor color;
    switch (node->getType()) {
    case ASTNodeType::PROGRAM:
        color = QColor("#4ec9b0");  
        break;
    case ASTNodeType::DECLARATION:
        color = QColor("#9cdcfe");  
        break;
    case ASTNodeType::ASSIGNMENT:
        color = QColor("#dcdcaa");  
        break;
    case ASTNodeType::LITERAL:
        color = QColor("#ce9178");  
        break;
    case ASTNodeType::IDENTIFIER:
        color = QColor("#c586c0");  
        break;
    default:
        color = QColor("#ffffff");  
        break;
    }
    item->setForeground(0, color);

     
    for (ASTNode* child : node->getChildren()) {
        populateASTTree(item, child);
    }
}

void SemanticAnalyzerWidget::highlightInlineErrors() {
     
    QTextCursor cursor(sourceCodeEdit->document());
    cursor.select(QTextCursor::Document);
    QTextCharFormat clearFormat;
    clearFormat.setUnderlineStyle(QTextCharFormat::NoUnderline);
    cursor.setCharFormat(clearFormat);

    if (!semanticAnalyzer) return;

     
    QVector<SemanticError> errors = semanticAnalyzer->getErrors();
    QVector<SemanticError> warnings = semanticAnalyzer->getWarnings();

     
    for (const auto& error : errors) {
        highlightErrorAtLine(error.line, true);
    }

     
    for (const auto& warning : warnings) {
        highlightErrorAtLine(warning.line, false);
    }
}

void SemanticAnalyzerWidget::highlightErrorAtLine(int line, bool isError) {
    if (line <= 0) return;

    QTextDocument* doc = sourceCodeEdit->document();
    QTextBlock block = doc->findBlockByLineNumber(line - 1);  
    
    if (!block.isValid()) return;

    QTextCursor cursor(block);
    cursor.select(QTextCursor::LineUnderCursor);

    QTextCharFormat format;
    format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    if (isError) {
        format.setUnderlineColor(QColor("#ff0000"));  
    } else {
        format.setUnderlineColor(QColor("#ffcc00"));  
    }

    cursor.mergeCharFormat(format);
}
