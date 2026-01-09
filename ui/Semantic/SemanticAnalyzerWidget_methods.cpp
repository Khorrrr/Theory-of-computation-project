
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
