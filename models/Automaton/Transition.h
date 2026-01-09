#ifndef TRANSITION_H
#define TRANSITION_H

#include <QString>
#include <QSet>

class Transition {
private:
    QString fromStateId;
    QString toStateId;
    QSet<QString> symbols;

public:
    Transition();
    Transition(const QString& from, const QString& to, const QString& symbol);
    Transition(const QString& from, const QString& to, const QSet<QString>& symbols);


    QString getFromStateId() const { return fromStateId; }
    QString getToStateId() const { return toStateId; }
    QSet<QString> getSymbols() const { return symbols; }


    void setFromStateId(const QString& from) { fromStateId = from; }
    void setToStateId(const QString& to) { toStateId = to; }
    void addSymbol(const QString& symbol) { symbols.insert(symbol); }
    void removeSymbol(const QString& symbol) { symbols.remove(symbol); }
    void setSymbols(const QSet<QString>& syms) { symbols = syms; }


    bool isEpsilonTransition() const;
    QString getSymbolsString() const;
    bool hasSymbol(const QString& symbol) const;
};

#endif
