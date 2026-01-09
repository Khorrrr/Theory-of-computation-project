#ifndef AUTOMATONCANVAS_H
#define AUTOMATONCANVAS_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include "./src/models/Automaton/Automaton.h"

enum class DrawMode {
    Select,
    AddState,
    AddTransition,
    Delete
};

class AutomatonCanvas : public QWidget {
    Q_OBJECT

private:
    Automaton* currentAutomaton;
    DrawMode currentMode;


    QString selectedStateId;
    QString hoverStateId;
    QString currentSelectedStateForPropertiesId;
    QPointF tempTransitionEnd;
    bool isDrawingTransition;

    QString draggedStateId;
    bool isDragging;


    QSet<QString> activeStateIds;
    QSet<QString> activeTransitionKeys;

    const double stateRadius = 30.0;
    const double finalStateInnerRadius = 24.0;

public:
    explicit AutomatonCanvas(QWidget *parent = nullptr);

    void setAutomaton(Automaton* automaton);
    Automaton* getAutomaton() { return currentAutomaton; }

    void setDrawMode(DrawMode mode);
    DrawMode getDrawMode() const { return currentMode; }

     
    void setActiveStates(const QSet<QString>& stateIds);
    void setActiveTransitions(const QSet<QString>& transitionKeys);
    void clearActiveElements();

signals:
    void stateAdded(const QString& stateId);
    void stateRemoved(const QString& stateId);
    void transitionAdded(const QString& from, const QString& to);
    void automatonModified();
    void stateSelected(const QString& stateId);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    void drawState(QPainter& painter, const State& state, bool highlight = false, bool isSelectedForProps = false, bool isSimulationActive = false);
    void drawTransition(QPainter& painter, const Transition& trans);
    void drawArrow(QPainter& painter, const QPointF& start, const QPointF& end, bool redArrow = true);
    void drawSelfLoop(QPainter& painter, const State& state, const QString& label, bool isSimulationActive = false);
    void drawCurvedTransition(QPainter& painter,
                              const QPointF& actualStart,
                              const QPointF& actualEnd,
                              const QPointF& refStart,
                              const QPointF& refEnd,
                              const QString& label,
                              bool curveUp,
                              bool isSimulationActive = false);
    State* findStateAtPosition(const QPointF& pos);
    QString generateStateId();

    QPointF calculateEdgePoint(const QPointF& center, const QPointF& target, double radius);
    double calculateAngle(const QPointF& from, const QPointF& to);

    bool hasReverseTransition(const QString& fromId, const QString& toId) const;
    QPointF calculateBezierPoint(double t, const QPointF& p0, const QPointF& p1,
                                 const QPointF& p2, const QPointF& p3);
};

#endif
