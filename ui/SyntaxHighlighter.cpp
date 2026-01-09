#include "SyntaxHighlighter.h"

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent) {
    
    HighlightingRule rule;

     
    keywordFormat.setForeground(QColor("#569cd6"));  
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns = {
        "\\bint\\b", "\\bfloat\\b", "\\bdouble\\b", "\\bchar\\b", "\\bbool\\b",
        "\\bvoid\\b", "\\bif\\b", "\\belse\\b", "\\bwhile\\b", "\\bfor\\b",
        "\\breturn\\b", "\\bbreak\\b", "\\bcontinue\\b", "\\bswitch\\b", "\\bcase\\b",
        "\\bdefault\\b", "\\bstruct\\b", "\\bclass\\b", "\\btrue\\b", "\\bfalse\\b",
        "\\blong\\b", "\\bshort\\b", "\\bconst\\b", "\\bstatic\\b"
    };
    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

     
    numberFormat.setForeground(QColor("#b5cea8"));  
    rule.pattern = QRegularExpression("\\b[0-9]+\\.?[0-9]*\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

     
    quotationFormat.setForeground(QColor("#ce9178"));  
    rule.pattern = QRegularExpression("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

     
    rule.pattern = QRegularExpression("'.'");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

     
    singleLineCommentFormat.setForeground(QColor("#6a9955"));  
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

     
    multiLineCommentFormat.setForeground(QColor("#6a9955"));  
    commentStartExpression = QRegularExpression("/\\*");
    commentEndExpression = QRegularExpression("\\*/");
}

void SyntaxHighlighter::highlightBlock(const QString &text) {
     
    for (const HighlightingRule &rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

     
    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);

    while (startIndex >= 0) {
        QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + match.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}
