#ifndef REGEXTONFA_H
#define REGEXTONFA_H

#include "./src/models/Automaton/Automaton.h"
#include <QString>
#include <QStack>
#include <QVector>

 
class RegexToNFA {
public:
    RegexToNFA();
    
     
    Automaton* convert(const QString& regex);
    
     
    bool isValidRegex(const QString& regex, QString* error = nullptr);

private:
    struct NFAFragment {
        QString startStateId;
        QString endStateId;
        QVector<State> states;
        QVector<Transition> transitions;
    };
    
    int stateCounter;
    
     
    NFAFragment createLiteralNFA(QChar symbol);
    NFAFragment createEpsilonNFA();
    NFAFragment concatenate(NFAFragment a, NFAFragment b);
    NFAFragment alternate(NFAFragment a, NFAFragment b);
    NFAFragment kleeneStar(NFAFragment a);
    NFAFragment kleenePlus(NFAFragment a);
    NFAFragment optional(NFAFragment a);
    
     
    QString infixToPostfix(const QString& regex);
    bool isOperator(QChar c);
    int precedence(QChar op);
    QString insertConcatOperator(const QString& regex);
    
     
    QString generateStateId();
    void mergeFragments(NFAFragment& target, const NFAFragment& source);
};

#endif  
