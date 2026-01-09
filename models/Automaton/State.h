#ifndef STATE_H
#define STATE_H

#include <QString>
#include <QPointF>

class State {
private:
    QString id;
    QString label;
    QPointF position;
    bool isInitial;
    bool isFinal;
    double radius;

public:
    State();
    State(const QString& id, const QString& label = "",
          const QPointF& pos = QPointF(0, 0));


    QString getId() const { return id; }
    QString getLabel() const { return label; }
    QPointF getPosition() const { return position; }
    bool getIsInitial() const { return isInitial; }
    bool getIsFinal() const { return isFinal; }
    double getRadius() const { return radius; }


    void setId(const QString& newId) { id = newId; }
    void setLabel(const QString& newLabel) { label = newLabel; }
    void setPosition(const QPointF& pos) { position = pos; }
    void setIsInitial(bool initial) { isInitial = initial; }
    void setIsFinal(bool final) { isFinal = final; }
    void setRadius(double r) { radius = r; }


    bool containsPoint(const QPointF& point) const;
};

#endif
