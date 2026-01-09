#ifndef DFAMINIMIZER_H
#define DFAMINIMIZER_H

#include "./src/models/Automaton/Automaton.h"
#include <QSet>
#include <QMap>
#include <QPair>
#include <QVector>

class DFAMinimizer {
public:
    DFAMinimizer();

     
    Automaton* minimize(const Automaton* dfa);

private:
     
    void removeUnreachableStates(Automaton* dfa);

     
    QSet<QString> getReachableStates(const Automaton* dfa);

     
    QSet<QPair<QString, QString>> findDistinguishablePairs(const Automaton* dfa);

     
    QVector<QSet<QString>> createEquivalenceClasses(
        const Automaton* dfa,
        const QSet<QPair<QString, QString>>& distinguishable
        );

     
    Automaton* buildMinimizedDFA(
        const Automaton* dfa,
        const QVector<QSet<QString>>& equivalenceClasses
        );

     
    int findClassIndex(const QVector<QSet<QString>>& classes, const QString& stateId);

     
    QPair<QString, QString> makePair(const QString& s1, const QString& s2);
};

#endif  
