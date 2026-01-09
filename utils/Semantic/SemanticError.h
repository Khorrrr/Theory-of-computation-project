#ifndef SEMANTICERROR_H
#define SEMANTICERROR_H

#include <QString>

struct SemanticError {
    QString message;
    int line;
    QString type;  

    SemanticError(const QString& msg = "", int ln = 0, const QString& t = "Error")
        : message(msg), line(ln), type(t) {}
};

#endif  
