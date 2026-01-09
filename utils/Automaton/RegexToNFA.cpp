#include "RegexToNFA.h"
#include <QDebug>
#include <QPointF>

RegexToNFA::RegexToNFA() : stateCounter(0) {}

QString RegexToNFA::generateStateId() {
    return QString("q%1").arg(stateCounter++);
}

bool RegexToNFA::isValidRegex(const QString& regex, QString* error) {
    if (regex.isEmpty()) {
        if (error) *error = "Regex cannot be empty";
        return false;
    }
    
    int parenCount = 0;
    for (int i = 0; i < regex.length(); i++) {
        QChar c = regex[i];
        if (c == '(') parenCount++;
        else if (c == ')') {
            parenCount--;
            if (parenCount < 0) {
                if (error) *error = "Unmatched closing parenthesis";
                return false;
            }
        }
        
         
        if (i == 0 && (c == '*' || c == '+' || c == '?' || c == '|')) {
            if (error) *error = QString("Invalid operator '%1' at start").arg(c);
            return false;
        }
    }
    
    if (parenCount != 0) {
        if (error) *error = "Unmatched opening parenthesis";
        return false;
    }
    
    return true;
}

RegexToNFA::NFAFragment RegexToNFA::createLiteralNFA(QChar symbol) {
    NFAFragment frag;
    frag.startStateId = generateStateId();
    frag.endStateId = generateStateId();
    
    State start(frag.startStateId, frag.startStateId);
    State end(frag.endStateId, frag.endStateId);
    
    frag.states.append(start);
    frag.states.append(end);
    
    Transition trans(frag.startStateId, frag.endStateId, symbol);
    frag.transitions.append(trans);
    
    return frag;
}

RegexToNFA::NFAFragment RegexToNFA::createEpsilonNFA() {
    NFAFragment frag;
    frag.startStateId = generateStateId();
    frag.endStateId = generateStateId();
    
    State start(frag.startStateId, frag.startStateId);
    State end(frag.endStateId, frag.endStateId);
    
    frag.states.append(start);
    frag.states.append(end);
    
    Transition trans(frag.startStateId, frag.endStateId, "E");
    frag.transitions.append(trans);
    
    return frag;
}

void RegexToNFA::mergeFragments(NFAFragment& target, const NFAFragment& source) {
    target.states.append(source.states);
    target.transitions.append(source.transitions);
}

RegexToNFA::NFAFragment RegexToNFA::concatenate(NFAFragment a, NFAFragment b) {
    NFAFragment result;
    result.startStateId = a.startStateId;
    result.endStateId = b.endStateId;
    
     
    mergeFragments(result, a);
    mergeFragments(result, b);
    
     
    Transition epsilon(a.endStateId, b.startStateId, "E");
    result.transitions.append(epsilon);
    
    return result;
}

RegexToNFA::NFAFragment RegexToNFA::alternate(NFAFragment a, NFAFragment b) {
    NFAFragment result;
    result.startStateId = generateStateId();
    result.endStateId = generateStateId();
    
    State newStart(result.startStateId, result.startStateId);
    State newEnd(result.endStateId, result.endStateId);
    
    result.states.append(newStart);
    result.states.append(newEnd);
    
     
    mergeFragments(result, a);
    mergeFragments(result, b);
    
     
    result.transitions.append(Transition(result.startStateId, a.startStateId, "E"));
    result.transitions.append(Transition(result.startStateId, b.startStateId, "E"));
    
     
    result.transitions.append(Transition(a.endStateId, result.endStateId, "E"));
    result.transitions.append(Transition(b.endStateId, result.endStateId, "E"));
    
    return result;
}

RegexToNFA::NFAFragment RegexToNFA::kleeneStar(NFAFragment a) {
    NFAFragment result;
    result.startStateId = generateStateId();
    result.endStateId = generateStateId();
    
    State newStart(result.startStateId, result.startStateId);
    State newEnd(result.endStateId, result.endStateId);
    
    result.states.append(newStart);
    result.states.append(newEnd);
    mergeFragments(result, a);
    
     
    result.transitions.append(Transition(result.startStateId, a.startStateId, "E"));
    result.transitions.append(Transition(result.startStateId, result.endStateId, "E"));
    
     
    result.transitions.append(Transition(a.endStateId, a.startStateId, "E"));
    
     
    result.transitions.append(Transition(a.endStateId, result.endStateId, "E"));
    
    return result;
}

RegexToNFA::NFAFragment RegexToNFA::kleenePlus(NFAFragment a) {
     
    NFAFragment star = kleeneStar(a);
    
     
    NFAFragment aCopy;
    aCopy.startStateId = generateStateId();
    aCopy.endStateId = generateStateId();
    
    State start(aCopy.startStateId, aCopy.startStateId);
    State end(aCopy.endStateId, aCopy.endStateId);
    aCopy.states.append(start);
    aCopy.states.append(end);
    
     
    for (const auto& trans : a.transitions) {
        QString from = (trans.getFromStateId() == a.startStateId) ? aCopy.startStateId : trans.getFromStateId();
        QString to = (trans.getToStateId() == a.endStateId) ? aCopy.endStateId : trans.getToStateId();
        aCopy.transitions.append(Transition(from, to, trans.getSymbols()));
    }
    
    return concatenate(aCopy, star);
}

RegexToNFA::NFAFragment RegexToNFA::optional(NFAFragment a) {
    NFAFragment result;
    result.startStateId = generateStateId();
    result.endStateId = generateStateId();
    
    State newStart(result.startStateId, result.startStateId);
    State newEnd(result.endStateId, result.endStateId);
    
    result.states.append(newStart);
    result.states.append(newEnd);
    mergeFragments(result, a);
    
     
    result.transitions.append(Transition(result.startStateId, a.startStateId, "E"));
    
     
    result.transitions.append(Transition(result.startStateId, result.endStateId, "E"));
    
     
    result.transitions.append(Transition(a.endStateId, result.endStateId, "E"));
    
    return result;
}

bool RegexToNFA::isOperator(QChar c) {
    return c == '|' || c == '*' || c == '+' || c == '?' || c == '.';
}

int RegexToNFA::precedence(QChar op) {
    switch (op.toLatin1()) {
        case '|': return 1;
        case '.': return 2;   
        case '*':
        case '+':
        case '?': return 3;
        default: return 0;
    }
}

QString RegexToNFA::insertConcatOperator(const QString& regex) {
    QString result;
    
    for (int i = 0; i < regex.length(); i++) {
        QChar c1 = regex[i];
        result += c1;
        
        if (i + 1 < regex.length()) {
            QChar c2 = regex[i + 1];
            
             
             
             
             
             
             
             
             
             
             
             
            
            bool needsConcat = false;
            
            if (c1 != '(' && c1 != '|' && c2 != ')' && c2 != '|' && c2 != '*' && c2 != '+' && c2 != '?') {
                needsConcat = true;
            }
            
            if (needsConcat) {
                result += '.';
            }
        }
    }
    
    return result;
}

QString RegexToNFA::infixToPostfix(const QString& regex) {
    QString withConcat = insertConcatOperator(regex);
    QString postfix;
    QStack<QChar> stack;
    
    for (QChar c : withConcat) {
        if (c == '(') {
            stack.push(c);
        }
        else if (c == ')') {
            while (!stack.isEmpty() && stack.top() != '(') {
                postfix += stack.pop();
            }
            if (!stack.isEmpty()) stack.pop();  
        }
        else if (isOperator(c)) {
            while (!stack.isEmpty() && stack.top() != '(' && 
                   precedence(stack.top()) >= precedence(c)) {
                postfix += stack.pop();
            }
            stack.push(c);
        }
        else {
             
            postfix += c;
        }
    }
    
    while (!stack.isEmpty()) {
        postfix += stack.pop();
    }
    
    return postfix;
}

Automaton* RegexToNFA::convert(const QString& regex) {
    QString error;
    if (!isValidRegex(regex, &error)) {
        qWarning() << "Invalid regex:" << error;
        return nullptr;
    }
    
    stateCounter = 0;
    QString postfix = infixToPostfix(regex);
    
    QStack<NFAFragment> stack;
    
    for (QChar c : postfix) {
        if (c == '|') {
            if (stack.size() < 2) return nullptr;
            NFAFragment b = stack.pop();
            NFAFragment a = stack.pop();
            stack.push(alternate(a, b));
        }
        else if (c == '.') {
            if (stack.size() < 2) return nullptr;
            NFAFragment b = stack.pop();
            NFAFragment a = stack.pop();
            stack.push(concatenate(a, b));
        }
        else if (c == '*') {
            if (stack.isEmpty()) return nullptr;
            NFAFragment a = stack.pop();
            stack.push(kleeneStar(a));
        }
        else if (c == '+') {
            if (stack.isEmpty()) return nullptr;
            NFAFragment a = stack.pop();
            stack.push(kleenePlus(a));
        }
        else if (c == '?') {
            if (stack.isEmpty()) return nullptr;
            NFAFragment a = stack.pop();
            stack.push(optional(a));
        }
        else {
             
            if (c == 'E' || c == QChar(0x03B5)) {
                stack.push(createEpsilonNFA());
            } else {
                stack.push(createLiteralNFA(c));
            }
        }
    }
    
    if (stack.size() != 1) {
        qWarning() << "Invalid regex expression";
        return nullptr;
    }
    
    NFAFragment final = stack.pop();
    
     
    Automaton* nfa = new Automaton(QString("nfa_%1").arg(stateCounter), 
                                    QString("NFA from /%1/").arg(regex), 
                                    AutomatonType::NFA);
    
     
    for (const State& state : final.states) {
        nfa->addState(state);
    }
    
     
    State* startState = nfa->getState(final.startStateId);
    if (startState) {
        startState->setIsInitial(true);
        nfa->setInitialState(final.startStateId);
    }
    
     
    State* endState = nfa->getState(final.endStateId);
    if (endState) {
        endState->setIsFinal(true);
    }
    
     
    for (const Transition& trans : final.transitions) {
        nfa->addTransition(trans);
         
        for (const QString& sym : trans.getSymbols()) {
            if (sym != "E") {
                nfa->addToAlphabet(sym);
            }
        }
    }
    
     
    int col = 0, row = 0;
    int cols = qCeil(qSqrt(final.states.size()));
    
    for (State& state : nfa->getStates()) {
        state.setPosition(QPointF(100 + col * 120, 100 + row * 120));
        col++;
        if (col >= cols) {
            col = 0;
            row++;
        }
    }
    
    return nfa;
}
