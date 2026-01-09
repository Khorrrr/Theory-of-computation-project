#include "AutomatonCanvas.h"
#include <QPainter>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QMainWindow>
#include <QStatusBar>
#include <QPainterPath>
#include <cmath>

AutomatonCanvas::AutomatonCanvas(QWidget *parent)
    : QWidget(parent), currentAutomaton(nullptr), currentMode(DrawMode::Select),
    selectedStateId(""), hoverStateId(""), currentSelectedStateForPropertiesId(""),
    isDrawingTransition(false), draggedStateId(""), isDragging(false) {

    setMinimumSize(800, 600);
    setMouseTracking(true);
    setStyleSheet("background-color: white;");
}

void AutomatonCanvas::setAutomaton(Automaton* automaton) {
    currentAutomaton = automaton;

     
    selectedStateId = "";
    hoverStateId = "";
    currentSelectedStateForPropertiesId = "";
    draggedStateId = "";
    isDrawingTransition = false;
    isDragging = false;

    emit stateSelected("");
    clearActiveElements();
    update();
}

void AutomatonCanvas::setDrawMode(DrawMode mode) {
    currentMode = mode;
    selectedStateId = "";
    isDrawingTransition = false;
    isDragging = false;
    draggedStateId = "";
    setCursor(mode == DrawMode::AddState ? Qt::CrossCursor : Qt::ArrowCursor);
}

void AutomatonCanvas::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (!currentAutomaton) {
        painter.setPen(Qt::black);
        painter.drawText(rect(), Qt::AlignCenter,
                         "No automaton loaded\n\nClick 'New' button in Automatons panel to create one");
        return;
    }

     
    for (const auto& trans : currentAutomaton->getTransitions()) {
        drawTransition(painter, trans);
    }

     
    if (isDrawingTransition && !selectedStateId.isEmpty()) {
        State* selectedState = currentAutomaton->getState(selectedStateId);
        if (selectedState) {  
            painter.setPen(QPen(Qt::gray, 2, Qt::DashLine));
            painter.drawLine(selectedState->getPosition(), tempTransitionEnd);
        } else {
             
            selectedStateId = "";
            isDrawingTransition = false;
        }
    }

     
    for (const auto& state : currentAutomaton->getStates()) {
         
        bool isHovered = (state.getId() == hoverStateId);
        bool isSelected = (state.getId() == selectedStateId);
        bool isSelectedForProps = (state.getId() == currentSelectedStateForPropertiesId);
        bool isSimulationActive = activeStateIds.contains(state.getId());

        bool highlight = isHovered || isSelected || isSelectedForProps;
        drawState(painter, state, highlight, isSelectedForProps, isSimulationActive);
    }
}

void AutomatonCanvas::drawState(QPainter& painter, const State& state, bool highlight, bool isSelectedForProps, bool isSimulationActive) {
    QPointF pos = state.getPosition();
    double radius = state.getRadius();

     
    QColor borderColor = Qt::black;
    int borderWidth = 2;

    if (isSimulationActive) {
        borderColor = QColor("#ffc107");  
        borderWidth = 4;
    } else if (isSelectedForProps) {
        borderColor = QColor("#0078d7");   
        borderWidth = 3;
    } else if (highlight) {
        borderColor = Qt::blue;
        borderWidth = 2;
    }

    painter.setPen(QPen(borderColor, borderWidth));

     
    if (isSimulationActive) {
        painter.setBrush(QColor("#fff8e1"));  
    } else if (isSelectedForProps) {
        painter.setBrush(QColor("#e3f2fd"));   
    } else {
        painter.setBrush(Qt::white);
    }

    painter.drawEllipse(pos, radius, radius);

    if (state.getIsFinal()) {
        painter.drawEllipse(pos, finalStateInnerRadius, finalStateInnerRadius);
    }

    if (state.getIsInitial()) {
        QPointF arrowStart = pos - QPointF(radius + 30, 0);
        QPointF arrowEnd = pos - QPointF(radius + 5, 0);
        painter.setPen(QPen(borderColor, borderWidth));
        drawArrow(painter, arrowStart, arrowEnd, true);
    }

    painter.setPen(Qt::black);
    QFont font = painter.font();
    font.setPointSize(12);
    font.setBold(true);
    painter.setFont(font);

    QRectF textRect(pos.x() - radius, pos.y() - radius, radius * 2, radius * 2);
    painter.drawText(textRect, Qt::AlignCenter, state.getLabel());
}

void AutomatonCanvas::drawTransition(QPainter& painter, const Transition& trans) {
    const State* fromState = currentAutomaton->getState(trans.getFromStateId());
    const State* toState = currentAutomaton->getState(trans.getToStateId());

    if (!fromState || !toState) return;

     
     
    QString transKey = trans.getFromStateId() + "|" + trans.getToStateId();
    bool isSimulationActive = activeTransitionKeys.contains(transKey);

     
    if (fromState == toState) {
        drawSelfLoop(painter, *fromState, trans.getSymbolsString(), isSimulationActive);
        return;
    }

    QPointF start = fromState->getPosition();
    QPointF end = toState->getPosition();

     
    bool hasReverse = hasReverseTransition(trans.getFromStateId(), trans.getToStateId());

    if (hasReverse) {
         
         
        QPointF refStart, refEnd;
        bool isForwardDirection;

        if (trans.getFromStateId() < trans.getToStateId()) {
             
            refStart = start;
            refEnd = end;
            isForwardDirection = true;
        } else {
             
             
            refStart = end;
            refEnd = start;
            isForwardDirection = false;
        }

         
        drawCurvedTransition(painter, start, end, refStart, refEnd,
                             trans.getSymbolsString(), isForwardDirection, isSimulationActive);
    } else {
         
        QPointF edgeStart = calculateEdgePoint(start, end, fromState->getRadius());
        QPointF edgeEnd = calculateEdgePoint(end, start, toState->getRadius());

        if (isSimulationActive) {
            painter.setPen(QPen(QColor("#dc3545"), 3));  
        } else {
            painter.setPen(QPen(Qt::black, 2));
        }
        painter.drawLine(edgeStart, edgeEnd);

        drawArrow(painter, edgeStart, edgeEnd, true);

        QPointF midPoint = (edgeStart + edgeEnd) / 2.0;
        painter.setPen(Qt::darkBlue);
        QFont font = painter.font();
        font.setPointSize(10);
        font.setBold(true);
        painter.setFont(font);

        QRectF labelRect(midPoint.x() - 30, midPoint.y() - 25, 60, 20);
        painter.fillRect(labelRect, Qt::white);
        painter.drawText(labelRect, Qt::AlignCenter, trans.getSymbolsString());
    }
}

void AutomatonCanvas::drawCurvedTransition(QPainter& painter,
                                           const QPointF& actualStart,
                                           const QPointF& actualEnd,
                                           const QPointF& refStart,
                                           const QPointF& refEnd,
                                           const QString& label,
                                           bool curveUp,
                                           bool isSimulationActive) {
     
     
    double refDx = refEnd.x() - refStart.x();
    double refDy = refEnd.y() - refStart.y();
    double refDist = qSqrt(refDx * refDx + refDy * refDy);

    if (refDist < 1.0) return;

     
    double refNdx = refDx / refDist;
    double refNdy = refDy / refDist;

     
     
    double perpX = -refNdy;
    double perpY = refNdx;

     
    if (!curveUp) {
        perpX = -perpX;
        perpY = -perpY;
    }

     
    double midX = (actualStart.x() + actualEnd.x()) / 2.0;
    double midY = (actualStart.y() + actualEnd.y()) / 2.0;

     
    double actualDx = actualEnd.x() - actualStart.x();
    double actualDy = actualEnd.y() - actualStart.y();
    double actualDist = qSqrt(actualDx * actualDx + actualDy * actualDy);
    double curveHeight = actualDist * 0.35;

    QPointF controlPoint(midX + perpX * curveHeight, midY + perpY * curveHeight);

     
    QPointF curveStart = actualStart;
    for (double t = 0.0; t <= 0.3; t += 0.01) {
        double u = 1 - t;
        QPointF point(
            u * u * actualStart.x() + 2 * u * t * controlPoint.x() + t * t * actualEnd.x(),
            u * u * actualStart.y() + 2 * u * t * controlPoint.y() + t * t * actualEnd.y()
            );

        double dist = qSqrt((point.x() - actualStart.x()) * (point.x() - actualStart.x()) +
                            (point.y() - actualStart.y()) * (point.y() - actualStart.y()));
        if (dist >= stateRadius) {
            curveStart = point;
            break;
        }
    }

     
    QPointF curveEnd = actualEnd;
    for (double t = 1.0; t >= 0.7; t -= 0.01) {
        double u = 1 - t;
        QPointF point(
            u * u * actualStart.x() + 2 * u * t * controlPoint.x() + t * t * actualEnd.x(),
            u * u * actualStart.y() + 2 * u * t * controlPoint.y() + t * t * actualEnd.y()
            );

        double dist = qSqrt((point.x() - actualEnd.x()) * (point.x() - actualEnd.x()) +
                            (point.y() - actualEnd.y()) * (point.y() - actualEnd.y()));
        if (dist >= stateRadius) {
            curveEnd = point;
            break;
        }
    }

     
    double trimMidX = (curveStart.x() + curveEnd.x()) / 2.0;
    double trimMidY = (curveStart.y() + curveEnd.y()) / 2.0;
    double trimDx = curveEnd.x() - curveStart.x();
    double trimDy = curveEnd.y() - curveStart.y();
    double trimDist = qSqrt(trimDx * trimDx + trimDy * trimDy);

    if (trimDist < 1.0) return;

    double trimHeight = trimDist * 0.35;
    QPointF trimControl(trimMidX + perpX * trimHeight, trimMidY + perpY * trimHeight);

    QPainterPath path;
    path.moveTo(curveStart);
    path.quadTo(trimControl, curveEnd);

    if (isSimulationActive) {
        painter.setPen(QPen(QColor("#dc3545"), 3));  
    } else {
        painter.setPen(QPen(Qt::black, 2));
    }
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(path);

     
    double tEnd = 0.95;
    double uEnd = 1 - tEnd;
    QPointF beforeEnd(
        uEnd * uEnd * curveStart.x() + 2 * uEnd * tEnd * trimControl.x() + tEnd * tEnd * curveEnd.x(),
        uEnd * uEnd * curveStart.y() + 2 * uEnd * tEnd * trimControl.y() + tEnd * tEnd * curveEnd.y()
        );

    double arrowAngle = atan2(curveEnd.y() - beforeEnd.y(),
                              curveEnd.x() - beforeEnd.x());

     
    double arrowSize = 12.0;
    QPointF arrowP1 = curveEnd - QPointF(
                          arrowSize * cos(arrowAngle - M_PI / 6),
                          arrowSize * sin(arrowAngle - M_PI / 6)
                          );

    QPointF arrowP2 = curveEnd - QPointF(
                          arrowSize * cos(arrowAngle + M_PI / 6),
                          arrowSize * sin(arrowAngle + M_PI / 6)
                          );

    painter.setBrush(QColor("#dc3545"));  
    painter.setPen(QPen(QColor("#dc3545"), 2));
    QPolygonF arrowHead;
    arrowHead << curveEnd << arrowP1 << arrowP2;
    painter.drawPolygon(arrowHead);

     
    painter.setPen(Qt::darkBlue);
    QFont font = painter.font();
    font.setPointSize(10);
    font.setBold(true);
    painter.setFont(font);

    QRectF labelRect(trimControl.x() - 30, trimControl.y() - 10, 60, 20);
    painter.fillRect(labelRect, Qt::white);
    painter.drawText(labelRect, Qt::AlignCenter, label);
}

void AutomatonCanvas::drawArrow(QPainter& painter, const QPointF& start, const QPointF& end, bool redArrow) {
    double angle = calculateAngle(start, end);
    double arrowSize = 12.0;

    QPointF arrowP1 = end - QPointF(
                          arrowSize * cos(angle - M_PI / 6),
                          arrowSize * sin(angle - M_PI / 6)
                          );

    QPointF arrowP2 = end - QPointF(
                          arrowSize * cos(angle + M_PI / 6),
                          arrowSize * sin(angle + M_PI / 6)
                          );

    if (redArrow) {
        painter.setBrush(QColor("#dc3545"));  
        painter.setPen(QPen(QColor("#dc3545"), 2));
    } else {
        painter.setBrush(Qt::black);
        painter.setPen(QPen(Qt::black, 2));
    }

    QPolygonF arrowHead;
    arrowHead << end << arrowP1 << arrowP2;
    painter.drawPolygon(arrowHead);
}

void AutomatonCanvas::drawSelfLoop(QPainter& painter, const State& state, const QString& label, bool isSimulationActive) {
    QPointF pos = state.getPosition();
    double radius = state.getRadius();

    QRectF loopRect(pos.x() - 20, pos.y() - radius - 50, 40, 40);

    if (isSimulationActive) {
        painter.setPen(QPen(QColor("#dc3545"), 3));  
    } else {
        painter.setPen(QPen(Qt::black, 2));
    }
    painter.setBrush(Qt::NoBrush);
    painter.drawArc(loopRect, 0, 270 * 16);

     
    QPointF arrowEnd = pos - QPointF(0, radius);
    QPointF arrowP1 = arrowEnd - QPointF(-5, 8);
    QPointF arrowP2 = arrowEnd - QPointF(5, 8);

    painter.setBrush(QColor("#dc3545"));  
    painter.setPen(QPen(QColor("#dc3545"), 2));
    QPolygonF arrowHead;
    arrowHead << arrowEnd << arrowP1 << arrowP2;
    painter.drawPolygon(arrowHead);

     
    painter.setPen(Qt::darkBlue);
    QFont font = painter.font();
    font.setPointSize(10);
    font.setBold(true);
    painter.setFont(font);

    QRectF labelRect(pos.x() - 25, pos.y() - radius - 70, 50, 20);
    painter.fillRect(labelRect, Qt::white);
    painter.drawText(labelRect, Qt::AlignCenter, label);
}

bool AutomatonCanvas::hasReverseTransition(const QString& fromId, const QString& toId) const {
    if (!currentAutomaton) return false;

     
    for (const auto& trans : currentAutomaton->getTransitions()) {
        if (trans.getFromStateId() == toId && trans.getToStateId() == fromId) {
            return true;
        }
    }

    return false;
}

void AutomatonCanvas::mousePressEvent(QMouseEvent *event) {
    if (!currentAutomaton) return;

    QPointF clickPos = event->pos();
    State* clickedState = findStateAtPosition(clickPos);

    switch (currentMode) {
    case DrawMode::AddState: {
        if (!clickedState) {
            QString stateId = generateStateId();
            State newState(stateId, stateId, clickPos);
            newState.setRadius(stateRadius);

            if (currentAutomaton->addState(newState)) {
                emit stateAdded(stateId);
                emit automatonModified();
                update();
            }
        }
        break;
    }

    case DrawMode::AddTransition: {
        if (clickedState) {
            if (!isDrawingTransition) {
                 
                selectedStateId = clickedState->getId();
                isDrawingTransition = true;
                tempTransitionEnd = clickPos;
            } else {
                 
                State* fromState = currentAutomaton->getState(selectedStateId);
                if (!fromState) {
                     
                    selectedStateId = "";
                    isDrawingTransition = false;
                    update();
                    return;
                }

                QDialog dialog(this);
                dialog.setWindowTitle("Add Transition");
                dialog.setStyleSheet(
                    "QDialog { background-color: #f0f0f0; }"
                    "QLabel { color: black; background-color: transparent; }"
                    "QLineEdit { color: black; background-color: white; border: 1px solid #999; padding: 5px; }"
                    "QPushButton { color: black; background-color: #e0e0e0; border: 1px solid #999; padding: 5px 15px; }"
                    "QPushButton:hover { background-color: #d0d0d0; }"
                    );

                QVBoxLayout* layout = new QVBoxLayout(&dialog);

                QLabel* infoLabel = new QLabel(QString("From: <b>%1</b> → To: <b>%2</b>")
                                                   .arg(fromState->getLabel())
                                                   .arg(clickedState->getLabel()));
                infoLabel->setStyleSheet("color: black; padding: 5px;");
                layout->addWidget(infoLabel);

                QLabel* promptLabel = new QLabel("Enter transition symbol:");
                promptLabel->setStyleSheet("color: black; font-weight: bold;");
                layout->addWidget(promptLabel);

                QLineEdit* symbolInput = new QLineEdit();
                symbolInput->setPlaceholderText("e.g., a, b, 0, 1   (Type 'E' for epsilon)");
                layout->addWidget(symbolInput);

                QLabel* hintLabel = new QLabel("Hint: Use 'E' for epsilon (ε) transitions in NFA");
                hintLabel->setStyleSheet("color: #666; font-size: 9pt; font-style: italic;");
                layout->addWidget(hintLabel);

                QHBoxLayout* btnLayout = new QHBoxLayout();
                QPushButton* okBtn = new QPushButton("Add");
                QPushButton* cancelBtn = new QPushButton("Cancel");
                btnLayout->addWidget(okBtn);
                btnLayout->addWidget(cancelBtn);
                layout->addLayout(btnLayout);

                connect(okBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
                connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
                connect(symbolInput, &QLineEdit::returnPressed, &dialog, &QDialog::accept);

                symbolInput->setFocus();

                if (dialog.exec() == QDialog::Accepted) {
                    QString symbol = symbolInput->text().trimmed();

                    if (symbol.isEmpty()) {
                        QMessageBox msgBox(this);
                        msgBox.setStyleSheet(
                            "QMessageBox { background-color: white; }"
                            "QLabel { color: black; }"
                            "QPushButton { color: black; background-color: #e0e0e0; border: 1px solid #999; padding: 5px 15px; }"
                            );
                        msgBox.setWindowTitle("Invalid Input");
                        msgBox.setText("Symbol cannot be empty.\nUse 'E' for epsilon transitions.");
                        msgBox.setIcon(QMessageBox::Warning);
                        msgBox.exec();
                    } else {
                        if (symbol.toLower() == "epsilon" || symbol == "ε") {
                            symbol = "E";
                        }

                        Transition trans(fromState->getId(), clickedState->getId(), symbol);

                        QString errorMsg;
                        if (currentAutomaton->canAddTransition(trans, &errorMsg)) {
                            if (currentAutomaton->addTransition(trans)) {
                                if (symbol != "E") {
                                    currentAutomaton->addToAlphabet(symbol);
                                }

                                emit transitionAdded(fromState->getId(), clickedState->getId());
                                emit automatonModified();

                                QMainWindow* mainWindow = qobject_cast<QMainWindow*>(window());
                                if (mainWindow && mainWindow->statusBar()) {
                                    QString displaySymbol = (symbol == "E") ? "ε (epsilon)" : symbol;
                                    mainWindow->statusBar()->showMessage(
                                        QString("✓ Transition added: %1 --(%2)--> %3")
                                            .arg(fromState->getLabel())
                                            .arg(displaySymbol)
                                            .arg(clickedState->getLabel()), 3000);
                                }
                            }
                        } else {
                            QMessageBox msgBox(this);
                            msgBox.setStyleSheet(
                                "QMessageBox { background-color: white; }"
                                "QLabel { color: black; }"
                                "QPushButton { color: black; background-color: #e0e0e0; border: 1px solid #999; padding: 5px 15px; }"
                                );
                            msgBox.setWindowTitle("Invalid Transition");
                            msgBox.setText(errorMsg);
                            msgBox.setIcon(QMessageBox::Warning);
                            msgBox.exec();
                        }
                    }
                }

                selectedStateId = "";
                isDrawingTransition = false;
                update();
            }
        } else {
            if (isDrawingTransition) {
                selectedStateId = "";
                isDrawingTransition = false;
                update();
            }
        }
        break;
    }

    case DrawMode::Delete: {
        if (clickedState) {
            QString stateIdToDelete = clickedState->getId();

             
            if (selectedStateId == stateIdToDelete) selectedStateId = "";
            if (hoverStateId == stateIdToDelete) hoverStateId = "";
            if (currentSelectedStateForPropertiesId == stateIdToDelete) currentSelectedStateForPropertiesId = "";
            if (draggedStateId == stateIdToDelete) draggedStateId = "";

            if (currentAutomaton->removeState(stateIdToDelete)) {
                emit stateRemoved(stateIdToDelete);
                emit automatonModified();
                update();
            }
        }
        break;
    }

    case DrawMode::Select: {
        if (clickedState) {
            draggedStateId = clickedState->getId();
            isDragging = true;

            currentSelectedStateForPropertiesId = clickedState->getId();
            emit stateSelected(clickedState->getId());
        } else {
            currentSelectedStateForPropertiesId = "";
            emit stateSelected("");
        }
        update();
        break;
    }
    }
}

void AutomatonCanvas::mouseMoveEvent(QMouseEvent *event) {
    QPointF mousePos = event->pos();

    State* hovered = findStateAtPosition(mousePos);
    hoverStateId = hovered ? hovered->getId() : "";

    if (isDrawingTransition) {
        tempTransitionEnd = mousePos;
        update();
    }

    if (isDragging && !draggedStateId.isEmpty()) {
        State* dragged = currentAutomaton->getState(draggedStateId);
        if (dragged) {  
            dragged->setPosition(mousePos);
            update();
        } else {
             
            draggedStateId = "";
            isDragging = false;
        }
    }

    if (hovered || isDrawingTransition || isDragging) {
        update();
    }
}

void AutomatonCanvas::mouseReleaseEvent(QMouseEvent *event) {
    if (isDragging) {
        isDragging = false;
        draggedStateId = "";
        emit automatonModified();
    }
}

void AutomatonCanvas::mouseDoubleClickEvent(QMouseEvent *event) {
    if (!currentAutomaton || currentMode != DrawMode::Select) return;

    State* clickedState = findStateAtPosition(event->pos());
    if (clickedState) {
        QDialog dialog(this);
        dialog.setWindowTitle("State Properties: " + clickedState->getLabel());

        dialog.setStyleSheet(
            "QDialog { background-color: #f0f0f0; }"
            "QLabel { color: black; background-color: transparent; }"
            "QLineEdit { color: black; background-color: white; border: 1px solid #999; padding: 3px; }"
            "QCheckBox { color: black; background-color: transparent; }"
            "QPushButton { color: black; background-color: #e0e0e0; border: 1px solid #999; padding: 5px 15px; }"
            "QPushButton:hover { background-color: #d0d0d0; }"
            );

        QVBoxLayout* layout = new QVBoxLayout(&dialog);

        QHBoxLayout* labelLayout = new QHBoxLayout();
        QLabel* labelLabel = new QLabel("Label:");
        labelLabel->setStyleSheet("color: black; font-weight: bold;");
        labelLayout->addWidget(labelLabel);

        QLineEdit* labelEdit = new QLineEdit(clickedState->getLabel());
        labelLayout->addWidget(labelEdit);
        layout->addLayout(labelLayout);

        QCheckBox* initialCheck = new QCheckBox("Initial State (Entry Point)");
        initialCheck->setChecked(clickedState->getIsInitial());
        initialCheck->setStyleSheet("color: black; font-weight: bold;");
        layout->addWidget(initialCheck);

        QCheckBox* finalCheck = new QCheckBox("Final/Accepting State");
        finalCheck->setChecked(clickedState->getIsFinal());
        finalCheck->setStyleSheet("color: black; font-weight: bold;");
        layout->addWidget(finalCheck);

        QLabel* warningLabel = new QLabel();
        warningLabel->setStyleSheet("color: #cc0000; font-style: italic; background-color: transparent;");
        warningLabel->setWordWrap(true);
        layout->addWidget(warningLabel);

        connect(initialCheck, &QCheckBox::toggled, [&](bool checked) {
            if (checked && !clickedState->getIsInitial()) {
                for (const auto& state : currentAutomaton->getStates()) {
                    if (state.getIsInitial() && state.getId() != clickedState->getId()) {
                        warningLabel->setText("⚠ Warning: Current initial state will be changed.");
                        break;
                    }
                }
            } else {
                warningLabel->clear();
            }
        });

        QHBoxLayout* btnLayout = new QHBoxLayout();
        QPushButton* okBtn = new QPushButton("OK");
        QPushButton* cancelBtn = new QPushButton("Cancel");
        btnLayout->addWidget(okBtn);
        btnLayout->addWidget(cancelBtn);
        layout->addLayout(btnLayout);

        connect(okBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
        connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

        if (dialog.exec() == QDialog::Accepted) {
            QString newLabel = labelEdit->text().trimmed();
            if (!newLabel.isEmpty()) {
                clickedState->setLabel(newLabel);
            }

            if (initialCheck->isChecked() && !clickedState->getIsInitial()) {
                currentAutomaton->setInitialState(clickedState->getId());
            } else if (!initialCheck->isChecked() && clickedState->getIsInitial()) {
                clickedState->setIsInitial(false);
                if (currentAutomaton->getInitialStateId() == clickedState->getId()) {
                    currentAutomaton->setInitialState("");
                }
            }

            clickedState->setIsFinal(finalCheck->isChecked());

            emit automatonModified();
            update();
        }
    }
}

void AutomatonCanvas::contextMenuEvent(QContextMenuEvent *event) {
    if (!currentAutomaton) return;

    State* clickedState = findStateAtPosition(event->pos());
    if (clickedState) {
        QMenu menu(this);

        menu.setStyleSheet(
            "QMenu { background-color: white; color: black; border: 1px solid #999; }"
            "QMenu::item { background-color: white; color: black; padding: 5px 25px; }"
            "QMenu::item:selected { background-color: #0078d7; color: white; }"
            "QMenu::separator { height: 1px; background-color: #ccc; margin: 3px 0px; }"
            );

        QAction* propsAction = menu.addAction("Properties...");
        menu.addSeparator();

        QString initialText = clickedState->getIsInitial() ?
                                  "✓ Remove as Initial" : "Set as Initial State";
        QAction* setInitialAction = menu.addAction(initialText);

        QString finalText = clickedState->getIsFinal() ?
                                "✓ Remove as Final" : "Set as Final State";
        QAction* setFinalAction = menu.addAction(finalText);

        menu.addSeparator();
        QAction* deleteAction = menu.addAction("Delete State");

        QAction* selected = menu.exec(event->globalPos());

        if (selected == propsAction) {
            QMouseEvent fakeEvent(QEvent::MouseButtonDblClick, event->pos(),
                                  Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            mouseDoubleClickEvent(&fakeEvent);
        }
        else if (selected == setInitialAction) {
            if (clickedState->getIsInitial()) {
                clickedState->setIsInitial(false);
                currentAutomaton->setInitialState("");
            } else {
                currentAutomaton->setInitialState(clickedState->getId());
            }
            emit automatonModified();
            update();
        }
        else if (selected == setFinalAction) {
            clickedState->setIsFinal(!clickedState->getIsFinal());
            emit automatonModified();
            update();
        }
        else if (selected == deleteAction) {
            QString stateIdToDelete = clickedState->getId();

             
            if (selectedStateId == stateIdToDelete) selectedStateId = "";
            if (hoverStateId == stateIdToDelete) hoverStateId = "";
            if (currentSelectedStateForPropertiesId == stateIdToDelete) currentSelectedStateForPropertiesId = "";
            if (draggedStateId == stateIdToDelete) draggedStateId = "";

            currentAutomaton->removeState(stateIdToDelete);
            emit automatonModified();
            update();
        }
    }
}

State* AutomatonCanvas::findStateAtPosition(const QPointF& pos) {
    if (!currentAutomaton) return nullptr;

    for (auto& state : currentAutomaton->getStates()) {
        if (state.containsPoint(pos)) {
            return &state;
        }
    }
    return nullptr;
}

void AutomatonCanvas::setActiveStates(const QSet<QString>& stateIds) {
    activeStateIds = stateIds;
    update();
}

void AutomatonCanvas::setActiveTransitions(const QSet<QString>& transitionKeys) {
    activeTransitionKeys = transitionKeys;
    update();
}

void AutomatonCanvas::clearActiveElements() {
    activeStateIds.clear();
    activeTransitionKeys.clear();
    update();
}

QString AutomatonCanvas::generateStateId() {
    if (!currentAutomaton) return "q0";

    int count = currentAutomaton->getStateCount();
    QString id;

    do {
        id = QString("q%1").arg(count++);
    } while (currentAutomaton->getState(id) != nullptr);

    return id;
}

QPointF AutomatonCanvas::calculateEdgePoint(const QPointF& center, const QPointF& target, double radius) {
    double angle = calculateAngle(center, target);
    return center + QPointF(radius * cos(angle), radius * sin(angle));
}

double AutomatonCanvas::calculateAngle(const QPointF& from, const QPointF& to) {
    return atan2(to.y() - from.y(), to.x() - from.x());
}

QPointF AutomatonCanvas::calculateBezierPoint(double t, const QPointF& p0, const QPointF& p1,
                                              const QPointF& p2, const QPointF& p3) {
    double u = 1 - t;
    double tt = t * t;
    double uu = u * u;
    double uuu = uu * u;
    double ttt = tt * t;

    QPointF p = uuu * p0;
    p += 3 * uu * t * p1;
    p += 3 * u * tt * p2;
    p += ttt * p3;

    return p;
}
