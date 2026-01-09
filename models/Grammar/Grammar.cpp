#include "Grammar.h"

Grammar::Grammar() : name("Untitled"), startSymbol("S") {}

Grammar::Grammar(const QString& name, const QString& start)
    : name(name), startSymbol(start) {}

bool Grammar::addProduction(const Production& prod) {
    productions.append(prod);

     
    if (!prod.getNonTerminal().isEmpty()) {
        nonTerminals.insert(prod.getNonTerminal());
    }

     
    for (const QString& symbol : prod.getSymbols()) {
        if (symbol == "ε" || symbol == "epsilon") continue;

         
        if (!symbol.isEmpty() && symbol[0].isUpper() && symbol.length() > 1) {
            nonTerminals.insert(symbol);
        } else if (symbol.length() == 1 && symbol[0].isUpper()) {
            nonTerminals.insert(symbol);
        } else {
            terminals.insert(symbol);
        }
    }

    return true;
}

bool Grammar::removeProduction(int index) {
    if (index < 0 || index >= productions.size()) {
        return false;
    }
    productions.removeAt(index);
    return true;
}

void Grammar::clear() {
    productions.clear();
    terminals.clear();
    nonTerminals.clear();
}

void Grammar::addTerminal(const QString& terminal) {
    terminals.insert(terminal);
}

void Grammar::addNonTerminal(const QString& nonTerminal) {
    nonTerminals.insert(nonTerminal);
}

QVector<Production> Grammar::getProductionsFor(const QString& nonTerminal) const {
    QVector<Production> result;
    for (const auto& prod : productions) {
        if (prod.getNonTerminal() == nonTerminal) {
            result.append(prod);
        }
    }
    return result;
}

bool Grammar::isTerminal(const QString& symbol) const {
    return terminals.contains(symbol);
}

bool Grammar::isNonTerminal(const QString& symbol) const {
    return nonTerminals.contains(symbol);
}

QString Grammar::toString() const {
    QString result = QString("Grammar: %1\n").arg(name);
    result += QString("Start Symbol: %1\n\n").arg(startSymbol);
    result += "Productions:\n";

    for (const auto& prod : productions) {
        result += "  " + prod.toString() + "\n";
    }

    return result;
}

Grammar Grammar::createArithmeticGrammar() {
    Grammar g("Arithmetic Expression Grammar", "E");

     
    g.addProduction(Production("E", {"E", "+", "T"}));
    g.addProduction(Production("E", {"E", "-", "T"}));
    g.addProduction(Production("E", {"T"}));

     
    g.addProduction(Production("T", {"T", "*", "F"}));
    g.addProduction(Production("T", {"T", "/", "F"}));
    g.addProduction(Production("T", {"F"}));

     
    g.addProduction(Production("F", {"(", "E", ")"}));
    g.addProduction(Production("F", {"id"}));
    g.addProduction(Production("F", {"num"}));

    return g;
}

Grammar Grammar::createSimpleStatementGrammar() {
    Grammar g("Simple Statement Grammar", "S");

     
    g.addProduction(Production("S", {"if", "E", "then", "S", "else", "S"}));
    g.addProduction(Production("S", {"while", "E", "do", "S"}));
    g.addProduction(Production("S", {"id", "=", "E"}));
    g.addProduction(Production("S", {";"}));

     
    g.addProduction(Production("E", {"E", "+", "E"}));
    g.addProduction(Production("E", {"E", "*", "E"}));
    g.addProduction(Production("E", {"(", "E", ")"}));
    g.addProduction(Production("E", {"id"}));
    g.addProduction(Production("E", {"num"}));

    return g;
}

Grammar Grammar::createExpressionGrammar() {
    Grammar g("Expression Grammar (LL)", "E");

     
    g.addProduction(Production("E", {"T", "E'"}));

     
    g.addProduction(Production("E'", {"+", "T", "E'"}));
    g.addProduction(Production("E'", {"ε"}));

     
    g.addProduction(Production("T", {"F", "T'"}));

     
    g.addProduction(Production("T'", {"*", "F", "T'"}));
    g.addProduction(Production("T'", {"ε"}));

     
    g.addProduction(Production("F", {"(", "E", ")"}));
    g.addProduction(Production("F", {"id"}));

    return g;
}
