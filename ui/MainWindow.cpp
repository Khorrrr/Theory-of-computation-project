#include "MainWindow.h"  
#include "./src/utils/Automaton/NFAtoDFA.h"  
#include "./src/utils/Automaton/DFAMinimizer.h"  
#include "./src/utils/Automaton/RegexToNFA.h"  
#include <QInputDialog>  
#include <QFileDialog>   
#include <QDialog>       
#include <QCheckBox>     
#include <QtMath>        
#include <QScrollArea>   
#include <QButtonGroup>  
#include <QHeaderView>   
#include <QDebug>        
#include <QShortcut>     
#include <QDialog>       

 
 
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
     
     
    currentAutomaton(nullptr), automatonCounter(0),
    currentSelectedStateId(""), automatonManager(nullptr), canvas(nullptr),
    centralTabs(nullptr), automatonTab(nullptr), lexerWidget(nullptr),
    toolsDock(nullptr), automatonListDock(nullptr), propertiesDock(nullptr),
    automatonList(nullptr),
    typeLabel(nullptr), stateCountLabel(nullptr), transitionCountLabel(nullptr),
    alphabetLabel(nullptr), selectedStateLabel(nullptr), deleteStateBtn(nullptr),
    transitionTable(nullptr), convertNFAtoDFABtn(nullptr), minimizeDFABtn(nullptr),
    selectModeBtn(nullptr), addStateModeBtn(nullptr), addTransitionModeBtn(nullptr),
    deleteModeBtn(nullptr), clearCanvasBtn(nullptr), newAutomatonBtn(nullptr),
    deleteAutomatonBtn(nullptr), renameAutomatonBtn(nullptr),
    newAction(nullptr), openAction(nullptr), saveAction(nullptr), exitAction(nullptr),
    convertAction(nullptr), minimizeAction(nullptr), aboutAction(nullptr),
    selectAction(nullptr), addStateAction(nullptr), addTransitionAction(nullptr),
    deleteAction(nullptr),
     
    playAction(nullptr), stepAction(nullptr), stopAction(nullptr), resetAction(nullptr),
    speedSlider(nullptr), speedLabel(nullptr), simulationTimer(nullptr),
    isSimulating(false), simulationStepIndex(0), simulationAccepted(false), simulationRejected(false) {

     
    setWindowTitle("Compiler Project");
    resize(1200, 700);
    setMinimumSize(1000, 650);  

     
    automatonManager = new AutomatonManager();
    if (!automatonManager) {
        qCritical() << "Failed to create AutomatonManager!";  
        return;  
    }

     
     
    setupCentralTabs();

     
    QShortcut* saveShortcut = new QShortcut(QKeySequence::Save, this);
    connect(saveShortcut, &QShortcut::activated, this, &MainWindow::onSave);

     
    createDockWidgets();
    createToolbar();  

     
    simulationTimer = new QTimer(this);
    connect(simulationTimer, &QTimer::timeout, this, &MainWindow::onSimulationTimerTimeout);

     
     
    if (canvas) {
        connect(canvas, &AutomatonCanvas::automatonModified,
                this, &MainWindow::onAutomatonModified);
        connect(canvas, &AutomatonCanvas::stateSelected,
                this, &MainWindow::onStateSelected);
    } else {
        qWarning() << "Canvas is null - connections failed";  
    }

     
    statusBar()->showMessage("Ready - Click 'New' or switch to Lexical Analyzer tab");
}

 
 
 
MainWindow::~MainWindow() {
     
     
     
    if (canvas) {
        canvas->disconnect();
    }

    if (centralTabs) {
        centralTabs->disconnect();
    }

     
     
    for (auto automaton : automatons) {
        if (automaton) {
            delete automaton;  
        }
    }
    automatons.clear();  

     
    currentAutomaton = nullptr;

     
     
     
     
    if (automatonManager) {
        delete automatonManager;
        automatonManager = nullptr;
    }

     
     
     
     
}

 

 

void MainWindow::createToolbar() {
    QToolBar* toolbar = addToolBar("Simulation");
    toolbar->setMovable(false);
    toolbar->setStyleSheet("QToolBar { spacing: 10px; padding: 5px; }");

     
    playAction = new QAction(QIcon(), "▶ Play", this);
    playAction->setToolTip("Start Simulation");
    connect(playAction, &QAction::triggered, this, &MainWindow::onPlaySimulation);
    toolbar->addAction(playAction);

     
    stepAction = new QAction(QIcon(), "⏯ Step", this);
    stepAction->setToolTip("Step Forward");
    connect(stepAction, &QAction::triggered, this, &MainWindow::onStepSimulation);
    toolbar->addAction(stepAction);

     
    stopAction = new QAction(QIcon(), "⏹ Stop", this);
    stopAction->setToolTip("Stop Simulation");
    stopAction->setEnabled(false);
    connect(stopAction, &QAction::triggered, this, &MainWindow::onStopSimulation);
    toolbar->addAction(stopAction);

     
    resetAction = new QAction(QIcon(), "⏮ Reset", this);
    resetAction->setToolTip("Reset Simulation");
    connect(resetAction, &QAction::triggered, this, &MainWindow::onResetSimulation);
    toolbar->addAction(resetAction);

    toolbar->addSeparator();

     
    QLabel* speedTitle = new QLabel("Speed:");
    toolbar->addWidget(speedTitle);

    speedSlider = new QSlider(Qt::Horizontal);
    speedSlider->setRange(100, 2000);  
    speedSlider->setValue(1000);
    speedSlider->setInvertedAppearance(true);  
    speedSlider->setFixedWidth(100);
    connect(speedSlider, &QSlider::valueChanged, this, &MainWindow::onSpeedChanged);
    toolbar->addWidget(speedSlider);

    speedLabel = new QLabel("1.0s");
    toolbar->addWidget(speedLabel);
}

void MainWindow::createDockWidgets() {
    createToolsPanel();
    createAutomatonListPanel();
    createPropertiesPanel();
     
}

void MainWindow::createToolsPanel() {
    toolsDock = new QDockWidget("Tools", this);
    toolsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget* toolsWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(5);
    layout->setContentsMargins(5, 5, 5, 5);

    QGroupBox* modeGroup = new QGroupBox("Drawing Mode");
    QVBoxLayout* modeLayout = new QVBoxLayout();
    modeLayout->setSpacing(3);

    selectModeBtn = new QRadioButton("Select");
    selectModeBtn->setChecked(true);
    connect(selectModeBtn, &QRadioButton::clicked, this, &MainWindow::onSelectMode);
    modeLayout->addWidget(selectModeBtn);

    addStateModeBtn = new QRadioButton("Add State");
    connect(addStateModeBtn, &QRadioButton::clicked, this, &MainWindow::onAddStateMode);
    modeLayout->addWidget(addStateModeBtn);

    addTransitionModeBtn = new QRadioButton("Add Transition");
    connect(addTransitionModeBtn, &QRadioButton::clicked, this, &MainWindow::onAddTransitionMode);
    modeLayout->addWidget(addTransitionModeBtn);

    deleteModeBtn = new QRadioButton("Delete");
    connect(deleteModeBtn, &QRadioButton::clicked, this, &MainWindow::onDeleteMode);
    modeLayout->addWidget(deleteModeBtn);

    modeGroup->setLayout(modeLayout);
    layout->addWidget(modeGroup);

    QGroupBox* actionsGroup = new QGroupBox("Actions");
    QVBoxLayout* actionsLayout = new QVBoxLayout();
    actionsLayout->setSpacing(3);

    clearCanvasBtn = new QPushButton("Clear Canvas");
    connect(clearCanvasBtn, &QPushButton::clicked, this, &MainWindow::onClearCanvas);
    actionsLayout->addWidget(clearCanvasBtn);

    actionsGroup->setLayout(actionsLayout);
    layout->addWidget(actionsGroup);

    layout->addStretch();

    toolsWidget->setLayout(layout);
    toolsWidget->setMaximumWidth(150);
    toolsDock->setWidget(toolsWidget);
    addDockWidget(Qt::LeftDockWidgetArea, toolsDock);
}

void MainWindow::createAutomatonListPanel() {
    automatonListDock = new QDockWidget("Automatons", this);
    automatonListDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget* listWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(5);
    layout->setContentsMargins(5, 5, 5, 5);

    automatonList = new QListWidget();
    automatonList->setMaximumHeight(120);
    connect(automatonList, &QListWidget::itemClicked,
            this, &MainWindow::onAutomatonSelected);
    layout->addWidget(automatonList);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(3);

    newAutomatonBtn = new QPushButton("New");
    newAutomatonBtn->setMaximumWidth(50);
    connect(newAutomatonBtn, &QPushButton::clicked, this, &MainWindow::onNewAutomaton);
    btnLayout->addWidget(newAutomatonBtn);

    deleteAutomatonBtn = new QPushButton("Del");
    deleteAutomatonBtn->setMaximumWidth(50);
    connect(deleteAutomatonBtn, &QPushButton::clicked, this, &MainWindow::onDeleteAutomaton);
    btnLayout->addWidget(deleteAutomatonBtn);

    renameAutomatonBtn = new QPushButton("Rename");
    connect(renameAutomatonBtn, &QPushButton::clicked, this, &MainWindow::onRenameAutomaton);
    btnLayout->addWidget(renameAutomatonBtn);

    layout->addLayout(btnLayout);

    listWidget->setLayout(layout);
    listWidget->setMaximumWidth(150);
    automatonListDock->setWidget(listWidget);
    addDockWidget(Qt::LeftDockWidgetArea, automatonListDock);
}

void MainWindow::createPropertiesPanel() {
    propertiesDock = new QDockWidget("Properties", this);
    propertiesDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);

    QWidget* propsWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(5);
    layout->setContentsMargins(5, 5, 5, 5);

     
    QGroupBox* infoGroup = new QGroupBox("Automaton Info");
    QVBoxLayout* infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(2);

    typeLabel = new QLabel("Type: N/A");
    typeLabel->setStyleSheet("font-weight: bold;");
    infoLayout->addWidget(typeLabel);

    stateCountLabel = new QLabel("States: 0");
    infoLayout->addWidget(stateCountLabel);

    transitionCountLabel = new QLabel("Transitions: 0");
    infoLayout->addWidget(transitionCountLabel);

    alphabetLabel = new QLabel("Alphabet: {}");
    alphabetLabel->setWordWrap(true);
    infoLayout->addWidget(alphabetLabel);

    infoGroup->setLayout(infoLayout);
    layout->addWidget(infoGroup);

     
    selectedStateLabel = new QLabel("No state selected");
    selectedStateLabel->setStyleSheet(
        "background-color: #f5f5f5; "
        "color: #999; "
        "padding: 8px; "
        "border: 1px solid #ddd; "
        "border-radius: 3px;"
        );
    selectedStateLabel->setAlignment(Qt::AlignCenter);
    selectedStateLabel->setWordWrap(true);
    layout->addWidget(selectedStateLabel);

     
    deleteStateBtn = new QPushButton("🗑 Delete Options");
    deleteStateBtn->setVisible(false);
    deleteStateBtn->setStyleSheet(
        "QPushButton { "
        "   background-color: #dc3545; "
        "   color: white; "
        "   border: none; "
        "   padding: 10px; "
        "   font-weight: bold; "
        "   border-radius: 3px;"
        "}"
        "QPushButton:hover { "
        "   background-color: #c82333; "
        "}"
        "QPushButton:pressed { "
        "   background-color: #bd2130; "
        "}"
        );
    connect(deleteStateBtn, &QPushButton::clicked, this, &MainWindow::onDeleteStateOrTransition);
    layout->addWidget(deleteStateBtn);

     
    QGroupBox* allTransGroup = new QGroupBox("All Transitions");
    QVBoxLayout* allTransLayout = new QVBoxLayout();
    allTransLayout->setSpacing(3);

    transitionTable = new QTableWidget();
    transitionTable->setColumnCount(3);
    transitionTable->setHorizontalHeaderLabels({"From", "Symbol", "To"});
    transitionTable->horizontalHeader()->setStretchLastSection(true);
    transitionTable->setMaximumHeight(120);
    transitionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    allTransLayout->addWidget(transitionTable);

    allTransGroup->setLayout(allTransLayout);
    layout->addWidget(allTransGroup);

     
    convertNFAtoDFABtn = new QPushButton("Convert NFA → DFA");
    convertNFAtoDFABtn->setStyleSheet(
        "QPushButton { background-color: #28a745; color: white; border: none; padding: 8px; font-weight: bold; border-radius: 3px; }"
        "QPushButton:hover { background-color: #218838; }"
        "QPushButton:disabled { background-color: #ccc; color: #666; }"
        );
    connect(convertNFAtoDFABtn, &QPushButton::clicked, this, &MainWindow::onConvertNFAtoDFA);
    layout->addWidget(convertNFAtoDFABtn);

    minimizeDFABtn = new QPushButton("⚡ Minimize DFA");
    minimizeDFABtn->setStyleSheet(
        "QPushButton { background-color: #ffc107; color: black; border: none; padding: 8px; font-weight: bold; border-radius: 3px; }"
        "QPushButton:hover { background-color: #ffb300; }"
        "QPushButton:disabled { background-color: #ccc; color: #666; }"
        );
    connect(minimizeDFABtn, &QPushButton::clicked, this, &MainWindow::onMinimizeDFA);
    layout->addWidget(minimizeDFABtn);

     
    fromRegexBtn = new QPushButton("🔤 From Regex");
    fromRegexBtn->setStyleSheet(
        "QPushButton { background-color: #ff9800; color: white; border: none; padding: 8px; font-weight: bold; border-radius: 3px; }"
        "QPushButton:hover { background-color: #fb8c00; }"
        "QPushButton:disabled { background-color: #ccc; color: #666; }"
        );
    connect(fromRegexBtn, &QPushButton::clicked, this, &MainWindow::onFromRegex);
    layout->addWidget(fromRegexBtn);

     
    testAutomatonBtn = new QPushButton("🧪 Test Automaton");
    testAutomatonBtn->setStyleSheet(
        "QPushButton { background-color: #17a2b8; color: white; border: none; padding: 8px; font-weight: bold; border-radius: 3px; }"
        "QPushButton:hover { background-color: #138496; }"
        "QPushButton:disabled { background-color: #ccc; color: #666; }"
        );
    connect(testAutomatonBtn, &QPushButton::clicked, this, &MainWindow::onTestAutomaton);
    layout->addWidget(testAutomatonBtn);

     
    traceAutomatonBtn = new QPushButton("🔍 Trace Execution");
    traceAutomatonBtn->setStyleSheet(
        "QPushButton { background-color: #6f42c1; color: white; border: none; padding: 8px; font-weight: bold; border-radius: 3px; }"
        "QPushButton:hover { background-color: #5a32a3; }"
        "QPushButton:disabled { background-color: #ccc; color: #666; }"
        );
    connect(traceAutomatonBtn, &QPushButton::clicked, this, &MainWindow::onTraceAutomaton);
    layout->addWidget(traceAutomatonBtn);

    layout->addStretch();

    propsWidget->setLayout(layout);
    propsWidget->setMaximumWidth(250);
    propertiesDock->setWidget(propsWidget);
    addDockWidget(Qt::RightDockWidgetArea, propertiesDock);
}

 

 
void MainWindow::onSelectMode() {
    if (!canvas) return;

    canvas->setDrawMode(DrawMode::Select);
    if (selectModeBtn) selectModeBtn->setChecked(true);
    if (selectAction) selectAction->setChecked(true);
    if (addStateModeBtn) addStateModeBtn->setChecked(false);
    if (addTransitionModeBtn) addTransitionModeBtn->setChecked(false);
    if (deleteModeBtn) deleteModeBtn->setChecked(false);
    if (addStateAction) addStateAction->setChecked(false);
    if (addTransitionAction) addTransitionAction->setChecked(false);
    if (deleteAction) deleteAction->setChecked(false);
    statusBar()->showMessage("Select/Move mode - Click states to see delete options");
}

void MainWindow::onAddStateMode() {
    if (!canvas) return;

    canvas->setDrawMode(DrawMode::AddState);
    if (addStateModeBtn) addStateModeBtn->setChecked(true);
    if (addStateAction) addStateAction->setChecked(true);
    if (selectModeBtn) selectModeBtn->setChecked(false);
    if (addTransitionModeBtn) addTransitionModeBtn->setChecked(false);
    if (deleteModeBtn) deleteModeBtn->setChecked(false);
    if (selectAction) selectAction->setChecked(false);
    if (addTransitionAction) addTransitionAction->setChecked(false);
    if (deleteAction) deleteAction->setChecked(false);
    statusBar()->showMessage("Add State mode - Click to add states");
}

void MainWindow::onAddTransitionMode() {
    if (!canvas) return;

    canvas->setDrawMode(DrawMode::AddTransition);
    if (addTransitionModeBtn) addTransitionModeBtn->setChecked(true);
    if (addTransitionAction) addTransitionAction->setChecked(true);
    if (selectModeBtn) selectModeBtn->setChecked(false);
    if (addStateModeBtn) addStateModeBtn->setChecked(false);
    if (deleteModeBtn) deleteModeBtn->setChecked(false);
    if (selectAction) selectAction->setChecked(false);
    if (addStateAction) addStateAction->setChecked(false);
    if (deleteAction) deleteAction->setChecked(false);
    statusBar()->showMessage("Add Transition mode - Click two states to connect");
}

void MainWindow::onDeleteMode() {
    if (!canvas) return;

    canvas->setDrawMode(DrawMode::Delete);
    if (deleteModeBtn) deleteModeBtn->setChecked(true);
    if (deleteAction) deleteAction->setChecked(true);
    if (selectModeBtn) selectModeBtn->setChecked(false);
    if (addStateModeBtn) addStateModeBtn->setChecked(false);
    if (addTransitionModeBtn) addTransitionModeBtn->setChecked(false);
    if (selectAction) selectAction->setChecked(false);
    if (addStateAction) addStateAction->setChecked(false);
    if (addTransitionAction) addTransitionAction->setChecked(false);
    statusBar()->showMessage("Delete mode - Click to delete states");
}

void MainWindow::onNewAutomaton() {
    QDialog dialog(this);
    dialog.setWindowTitle("Create New Automaton");
    dialog.setMinimumWidth(450);
    dialog.setStyleSheet(
        "QDialog { background-color: #2b2b2b; }"
        "QLabel { color: white; background-color: transparent; }"
        "QRadioButton { "
        "   color: white; "
        "   background-color: transparent; "
        "   spacing: 8px; "
        "   padding: 5px;"
        "}"
        "QRadioButton::indicator { "
        "   width: 18px; "
        "   height: 18px; "
        "   border-radius: 9px;"
        "   border: 2px solid #888;"
        "   background-color: #1e1e1e;"
        "}"
        "QRadioButton::indicator:checked { "
        "   background-color: #0078d7; "
        "   border: 2px solid #0078d7;"
        "}"
        "QRadioButton::indicator:hover { "
        "   border: 2px solid #0078d7;"
        "}"
        "QPushButton { "
        "   color: white; "
        "   background-color: #3e3e3e; "
        "   border: 1px solid #555; "
        "   padding: 8px 20px; "
        "   border-radius: 3px;"
        "}"
        "QPushButton:hover { "
        "   background-color: #4e4e4e; "
        "}"
        "QPushButton:pressed { "
        "   background-color: #2e2e2e; "
        "}"
        "QGroupBox { "
        "   color: white; "
        "   font-weight: bold; "
        "   border: 2px solid #555; "
        "   border-radius: 5px; "
        "   margin-top: 15px; "
        "   padding-top: 10px; "
        "   background-color: #1e1e1e;"
        "}"
        "QGroupBox::title { "
        "   subcontrol-origin: margin; "
        "   left: 10px; "
        "   padding: 0 5px; "
        "   background-color: #2b2b2b;"
        "}"
        );

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setSpacing(10);

    QLabel* titleLabel = new QLabel("Select Automaton Type:");
    titleLabel->setStyleSheet("color: white; font-size: 13pt; font-weight: bold; padding: 5px;");
    layout->addWidget(titleLabel);

    QGroupBox* typeGroup = new QGroupBox("Automaton Type");
    typeGroup->setStyleSheet(
        "QGroupBox { "
        "   color: white; "
        "   font-weight: bold; "
        "   border: 2px solid #0078d7; "
        "   border-radius: 5px; "
        "   margin-top: 15px; "
        "   padding: 15px; "
        "   background-color: #1e1e1e;"
        "}"
        "QGroupBox::title { "
        "   subcontrol-origin: margin; "
        "   left: 10px; "
        "   padding: 0 8px; "
        "   background-color: #2b2b2b;"
        "   color: #0078d7;"
        "}"
        );
    QVBoxLayout* typeLayout = new QVBoxLayout();
    typeLayout->setSpacing(15);

    QRadioButton* dfaRadio = new QRadioButton("DFA (Deterministic Finite Automaton)");
    dfaRadio->setStyleSheet(
        "QRadioButton { "
        "   color: white; "
        "   font-weight: bold; "
        "   font-size: 11pt;"
        "   spacing: 8px; "
        "   padding: 5px;"
        "}"
        "QRadioButton::indicator { "
        "   width: 20px; "
        "   height: 20px; "
        "   border-radius: 10px;"
        "   border: 2px solid #888;"
        "   background-color: #1e1e1e;"
        "}"
        "QRadioButton::indicator:checked { "
        "   background-color: #0078d7; "
        "   border: 3px solid #0078d7;"
        "}"
        "QRadioButton::indicator:hover { "
        "   border: 2px solid #0078d7;"
        "}"
        );
    dfaRadio->setChecked(true);
    typeLayout->addWidget(dfaRadio);

    QLabel* dfaDesc = new QLabel(
        "• Each state has exactly ONE transition per symbol\n"
        "• No epsilon (E) transitions allowed\n"
        "• Deterministic - predictable behavior"
        );
    dfaDesc->setStyleSheet(
        "color: #ccc; "
        "font-size: 10pt; "
        "margin-left: 30px; "
        "background-color: #252525; "
        "padding: 8px; "
        "border-left: 3px solid #0078d7;"
        );
    typeLayout->addWidget(dfaDesc);

    typeLayout->addSpacing(10);

    QRadioButton* nfaRadio = new QRadioButton("NFA (Non-deterministic Finite Automaton)");
    nfaRadio->setStyleSheet(
        "QRadioButton { "
        "   color: white; "
        "   font-weight: bold; "
        "   font-size: 11pt;"
        "   spacing: 8px; "
        "   padding: 5px;"
        "}"
        "QRadioButton::indicator { "
        "   width: 20px; "
        "   height: 20px; "
        "   border-radius: 10px;"
        "   border: 2px solid #888;"
        "   background-color: #1e1e1e;"
        "}"
        "QRadioButton::indicator:checked { "
        "   background-color: #28a745; "
        "   border: 3px solid #28a745;"
        "}"
        "QRadioButton::indicator:hover { "
        "   border: 2px solid #28a745;"
        "}"
        );
    typeLayout->addWidget(nfaRadio);

    QLabel* nfaDesc = new QLabel(
        "• States can have MULTIPLE transitions per symbol\n"
        "• Epsilon (E) transitions allowed\n"
        "• Non-deterministic - multiple possible paths"
        );
    nfaDesc->setStyleSheet(
        "color: #ccc; "
        "font-size: 10pt; "
        "margin-left: 30px; "
        "background-color: #252525; "
        "padding: 8px; "
        "border-left: 3px solid #28a745;"
        );
    typeLayout->addWidget(nfaDesc);

    typeGroup->setLayout(typeLayout);
    layout->addWidget(typeGroup);

    layout->addSpacing(10);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    QPushButton* createBtn = new QPushButton("✓ Create");
    createBtn->setStyleSheet(
        "QPushButton { "
        "   color: white; "
        "   background-color: #28a745; "
        "   border: none; "
        "   padding: 10px 25px; "
        "   font-weight: bold;"
        "   border-radius: 3px;"
        "}"
        "QPushButton:hover { "
        "   background-color: #218838; "
        "}"
        "QPushButton:pressed { "
        "   background-color: #1e7e34; "
        "}"
        );

    QPushButton* cancelBtn = new QPushButton("✗ Cancel");
    cancelBtn->setStyleSheet(
        "QPushButton { "
        "   color: white; "
        "   background-color: #dc3545; "
        "   border: none; "
        "   padding: 10px 25px; "
        "   font-weight: bold;"
        "   border-radius: 3px;"
        "}"
        "QPushButton:hover { "
        "   background-color: #c82333; "
        "}"
        "QPushButton:pressed { "
        "   background-color: #bd2130; "
        "}"
        );

    btnLayout->addWidget(createBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    connect(createBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        AutomatonType type = dfaRadio->isChecked() ? AutomatonType::DFA : AutomatonType::NFA;

        QString id = generateAutomatonId();
        QString name = QString("%1 %2").arg(type == AutomatonType::DFA ? "DFA" : "NFA").arg(automatonCounter);

        Automaton* newAutomaton = new Automaton(id, name, type);
        if (!newAutomaton) {
            qCritical() << "Failed to create new automaton";
            return;
        }

        automatons[id] = newAutomaton;

        updateAutomatonList();

        for (int i = 0; i < automatonList->count(); ++i) {
            QListWidgetItem* item = automatonList->item(i);
            if (item && item->data(Qt::UserRole).toString() == id) {
                automatonList->setCurrentItem(item);
                setCurrentAutomaton(newAutomaton);
                break;
            }
        }

        QString typeDesc = type == AutomatonType::DFA ?
                               "DFA created. Remember: Each state must have exactly one transition per symbol." :
                               "NFA created. You can add multiple transitions per symbol and use 'E' for epsilon.";

        statusBar()->showMessage(QString("%1 - %2").arg(name).arg(typeDesc), 5000);
    }
}

void MainWindow::onDeleteAutomaton() {
    if (!automatonList) return;

    QListWidgetItem* item = automatonList->currentItem();
    if (!item) {
        showStyledMessageBox("Warning", "Please select an automaton to delete.", QMessageBox::Warning);
        return;
    }

    QString id = item->data(Qt::UserRole).toString();

    QMessageBox msgBox(this);
    msgBox.setStyleSheet(
        "QMessageBox { background-color: white; }"
        "QLabel { color: black; }"
        "QPushButton { color: black; background-color: #e0e0e0; border: 1px solid #999; padding: 5px 15px; }"
        );
    msgBox.setWindowTitle("Confirm Delete");
    msgBox.setText("Are you sure you want to delete this automaton?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setIcon(QMessageBox::Question);

    if (msgBox.exec() == QMessageBox::Yes) {
        if (automatons.contains(id)) {
             
            if (currentAutomaton && currentAutomaton->getId() == id) {
                currentAutomaton = nullptr;
                currentSelectedStateId = "";
                if (canvas) {
                    canvas->setAutomaton(nullptr);
                }
            }

            delete automatons[id];
            automatons.remove(id);

            updateAutomatonList();
            updateProperties();
            statusBar()->showMessage("Automaton deleted");
        }
    }
}

void MainWindow::onRenameAutomaton() {
    if (!automatonList) return;

    QListWidgetItem* item = automatonList->currentItem();
    if (!item) {
        showStyledMessageBox("Warning", "Please select an automaton to rename.", QMessageBox::Warning);
        return;
    }

    QString id = item->data(Qt::UserRole).toString();
    Automaton* automaton = automatons.value(id);

    if (automaton) {
        bool ok;
        QString newName = QInputDialog::getText(this, "Rename Automaton",
                                                "Enter new name:", QLineEdit::Normal, automaton->getName(), &ok);

        if (ok && !newName.isEmpty()) {
            automaton->setName(newName);
            updateAutomatonList();
            statusBar()->showMessage(QString("Automaton renamed to: %1").arg(newName));
        }
    }
}

void MainWindow::onAutomatonSelected(QListWidgetItem* item) {
    if (!item) return;

    QString id = item->data(Qt::UserRole).toString();
    Automaton* automaton = automatons.value(id);

    if (automaton) {
        setCurrentAutomaton(automaton);
        statusBar()->showMessage(QString("Selected: %1").arg(automaton->getName()));
    }
}

void MainWindow::onClearCanvas() {
    if (!currentAutomaton) return;

    QMessageBox msgBox(this);
    msgBox.setStyleSheet(
        "QMessageBox { background-color: white; }"
        "QLabel { color: black; }"
        "QPushButton { color: black; background-color: #e0e0e0; border: 1px solid #999; padding: 5px 15px; }"
        );
    msgBox.setWindowTitle("Confirm Clear");
    msgBox.setText("Are you sure you want to clear the canvas?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setIcon(QMessageBox::Question);

    if (msgBox.exec() == QMessageBox::Yes) {
        currentAutomaton->clear();
        currentSelectedStateId = "";
        if (canvas) {
            canvas->update();
        }
        updateProperties();
        statusBar()->showMessage("Canvas cleared");
    }
}

void MainWindow::onDeleteStateOrTransition() {
    if (!currentAutomaton || currentSelectedStateId.isEmpty()) return;

     
    QString stateIdToDelete = currentSelectedStateId;

    const State* selectedState = currentAutomaton->getState(currentSelectedStateId);
    if (!selectedState) {
         
        currentSelectedStateId = "";
        updateProperties();
        return;
    }

     
    QDialog dialog(this);
    dialog.setWindowTitle("Delete Options");
    dialog.setMinimumWidth(400);
    dialog.setStyleSheet(
        "QDialog { background-color: #2b2b2b; }"
        "QLabel { color: white; background-color: transparent; }"
        "QRadioButton { color: white; background-color: transparent; padding: 5px; }"
        "QRadioButton::indicator { width: 16px; height: 16px; border-radius: 8px; "
        "border: 2px solid #888; background-color: #1e1e1e; }"
        "QRadioButton::indicator:checked { background-color: #dc3545; border: 2px solid #dc3545; }"
        "QRadioButton::indicator:hover { border: 2px solid #dc3545; }"
        "QPushButton { color: white; background-color: #3e3e3e; border: 1px solid #555; "
        "padding: 8px 20px; border-radius: 3px; }"
        "QPushButton:hover { background-color: #4e4e4e; }"
        "QPushButton:pressed { background-color: #2e2e2e; }"
        "QListWidget { background-color: #1e1e1e; color: white; border: 1px solid #555; }"
        "QListWidget::item { padding: 5px; }"
        "QListWidget::item:selected { background-color: #0078d7; }"
        "QListWidget::item:hover { background-color: #3e3e3e; }"
        "QGroupBox { color: white; border: 1px solid #555; border-radius: 3px; "
        "margin-top: 10px; padding-top: 10px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }"
        );

    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);

    QLabel* titleLabel = new QLabel(QString("Delete options for state: <b>%1</b>").arg(selectedState->getLabel()));
    titleLabel->setStyleSheet("color: white; font-size: 12pt; padding: 10px;");
    mainLayout->addWidget(titleLabel);

     
    QRadioButton* deleteNodeRadio = new QRadioButton("Delete entire state (node)");
    deleteNodeRadio->setStyleSheet("color: #ff6b6b; font-weight: bold; font-size: 11pt;");
    deleteNodeRadio->setChecked(true);
    mainLayout->addWidget(deleteNodeRadio);

    QLabel* nodeWarning = new QLabel("⚠ This will remove the state and ALL its transitions");
    nodeWarning->setStyleSheet("color: #ffa500; margin-left: 25px; font-size: 9pt; font-style: italic;");
    mainLayout->addWidget(nodeWarning);

    mainLayout->addSpacing(10);

    QRadioButton* deleteTransitionRadio = new QRadioButton("Delete specific transition(s)");
    deleteTransitionRadio->setStyleSheet("color: #4ec9b0; font-weight: bold; font-size: 11pt;");
    mainLayout->addWidget(deleteTransitionRadio);

     
    QVector<Transition> fromTransitions = currentAutomaton->getTransitionsFrom(currentSelectedStateId);

    QGroupBox* transitionsGroup = new QGroupBox("Select transitions to delete:");
    transitionsGroup->setEnabled(false);
    QVBoxLayout* transLayout = new QVBoxLayout();

    QListWidget* transitionsList = new QListWidget();
    transitionsList->setSelectionMode(QAbstractItemView::MultiSelection);

    if (fromTransitions.isEmpty()) {
        QListWidgetItem* noTransItem = new QListWidgetItem("(No outgoing transitions)");
        noTransItem->setFlags(Qt::NoItemFlags);
        transitionsList->addItem(noTransItem);
        deleteTransitionRadio->setEnabled(false);
        deleteTransitionRadio->setStyleSheet("color: #666; font-size: 11pt;");
    } else {
        for (const auto& trans : fromTransitions) {
            QString transText = QString("%1 --(%2)--> %3")
            .arg(trans.getFromStateId())
                .arg(trans.getSymbolsString())
                .arg(trans.getToStateId());

            QListWidgetItem* item = new QListWidgetItem(transText);
            item->setData(Qt::UserRole, trans.getToStateId());
            item->setData(Qt::UserRole + 1, trans.getSymbolsString());
            transitionsList->addItem(item);
        }
    }

    transLayout->addWidget(transitionsList);
    transitionsGroup->setLayout(transLayout);
    mainLayout->addWidget(transitionsGroup);

     
    connect(deleteTransitionRadio, &QRadioButton::toggled, [transitionsGroup](bool checked) {
        transitionsGroup->setEnabled(checked);
    });

    mainLayout->addSpacing(15);

     
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    QPushButton* deleteBtn = new QPushButton("🗑 Delete");
    deleteBtn->setStyleSheet(
        "QPushButton { color: white; background-color: #dc3545; border: none; "
        "padding: 10px 25px; font-weight: bold; border-radius: 3px; }"
        "QPushButton:hover { background-color: #c82333; }"
        "QPushButton:pressed { background-color: #bd2130; }"
        );

    QPushButton* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setStyleSheet(
        "QPushButton { color: white; background-color: #6c757d; border: none; "
        "padding: 10px 25px; font-weight: bold; border-radius: 3px; }"
        "QPushButton:hover { background-color: #5a6268; }"
        "QPushButton:pressed { background-color: #545b62; }"
        );

    btnLayout->addWidget(deleteBtn);
    btnLayout->addWidget(cancelBtn);
    mainLayout->addLayout(btnLayout);

    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    connect(deleteBtn, &QPushButton::clicked, [&, stateIdToDelete]() {  
        if (deleteNodeRadio->isChecked()) {
             
            QMessageBox confirmBox(&dialog);
            confirmBox.setStyleSheet(
                "QMessageBox { background-color: #2b2b2b; }"
                "QLabel { color: white; }"
                "QPushButton { color: white; background-color: #3e3e3e; border: 1px solid #555; padding: 5px 15px; }"
                );
            confirmBox.setWindowTitle("Confirm Delete State");
            confirmBox.setText(QString("Really delete state '%1' and all its transitions?").arg(selectedState->getLabel()));
            confirmBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            confirmBox.setIcon(QMessageBox::Warning);

            if (confirmBox.exec() == QMessageBox::Yes) {
                QString stateId = currentSelectedStateId;
                QString stateLabel = selectedState->getLabel();

                 
                currentSelectedStateId = "";

                 
                if (currentAutomaton->removeState(stateIdToDelete)) {
                    statusBar()->showMessage(QString("✓ State '%1' deleted").arg(stateLabel), 3000);
                    updateProperties();
                    if (canvas) {
                        canvas->update();
                    }
                    dialog.accept();
                } else {
                    statusBar()->showMessage("Failed to delete state", 3000);
                }
            }
        } else {
             
            QList<QListWidgetItem*> selected = transitionsList->selectedItems();

            if (selected.isEmpty()) {
                QMessageBox::warning(&dialog, "No Selection", "Please select at least one transition to delete.");
                return;
            }

            int deletedCount = 0;
            for (auto* item : selected) {
                QString toState = item->data(Qt::UserRole).toString();
                QString symbolsStr = item->data(Qt::UserRole + 1).toString();

                 
                QStringList symbols = symbolsStr.split(", ");
                for (const QString& symbol : symbols) {
                    QString actualSymbol = symbol;
                    if (actualSymbol == "ε") {
                        actualSymbol = "E";
                    }

                    if (currentAutomaton->removeTransition(currentSelectedStateId, toState, actualSymbol)) {
                        deletedCount++;
                    }
                }
            }

            if (deletedCount > 0) {
                statusBar()->showMessage(QString("✓ Deleted %1 transition(s)").arg(deletedCount), 3000);
                updateProperties();
                if (canvas) {
                    canvas->update();
                }
                dialog.accept();
            }
        }
    });

    dialog.exec();
}

void MainWindow::onConvertNFAtoDFA() {
    if (!currentAutomaton) {
        showStyledMessageBox("Warning", "No automaton selected.", QMessageBox::Warning);
        return;
    }

    if (currentAutomaton->isDFA()) {
        showStyledMessageBox("Info", "This automaton is already a DFA.", QMessageBox::Information);
        return;
    }

    if (!currentAutomaton->isValid()) {
        showStyledMessageBox("Warning",
                             "Current automaton is not valid. Please ensure it has an initial state.",
                             QMessageBox::Warning);
        return;
    }

    try {
        NFAtoDFA converter;
        Automaton* dfaAutomaton = converter.convert(currentAutomaton);

        if (dfaAutomaton) {
            QString id = generateAutomatonId();
            dfaAutomaton->setName(currentAutomaton->getName() + " (DFA)");

            int stateCount = dfaAutomaton->getStateCount();
            int cols = qCeil(qSqrt(stateCount));
            int row = 0, col = 0;

            for (auto& state : dfaAutomaton->getStates()) {
                state.setPosition(QPointF(100 + col * 120, 100 + row * 120));
                col++;
                if (col >= cols) {
                    col = 0;
                    row++;
                }
            }

            automatons[id] = dfaAutomaton;
            updateAutomatonList();

            for (int i = 0; i < automatonList->count(); ++i) {
                QListWidgetItem* item = automatonList->item(i);
                if (item && item->data(Qt::UserRole).toString() == id) {
                    automatonList->setCurrentItem(item);
                    setCurrentAutomaton(dfaAutomaton);
                    break;
                }
            }

            showStyledMessageBox("Success",
                                 QString("NFA converted to DFA successfully!\n\n"
                                         "Original NFA states: %1\n"
                                         "Resulting DFA states: %2")
                                     .arg(currentAutomaton->getStateCount())
                                     .arg(dfaAutomaton->getStateCount()),
                                 QMessageBox::Information);

            statusBar()->showMessage("NFA converted to DFA");
        }
    } catch (const std::exception& e) {
        showStyledMessageBox("Error",
                             QString("Failed to convert NFA to DFA: %1").arg(e.what()),
                             QMessageBox::Critical);
    }
}

void MainWindow::onMinimizeDFA() {
    if (!currentAutomaton) {
        showStyledMessageBox("Warning", "No automaton selected.", QMessageBox::Warning);
        return;
    }

     
    if (currentAutomaton->isNFA()) {
        showStyledMessageBox("Cannot Minimize NFA",
                             "DFA minimization can only be applied to DFAs.\n\n"
                             "This automaton is an NFA (Non-deterministic Finite Automaton).\n\n"
                             "💡 Tip: Convert it to a DFA first using 'Convert NFA → DFA' button, "
                             "then minimize the resulting DFA.",
                             QMessageBox::Warning);
        return;
    }

    if (!currentAutomaton->isValid()) {
        showStyledMessageBox("Warning",
                             "Current automaton is not valid. Please ensure it has an initial state.",
                             QMessageBox::Warning);
        return;
    }

     
    bool isActuallyDFA = true;
    for (const auto& t : currentAutomaton->getTransitions()) {
        if (t.isEpsilonTransition()) {
            isActuallyDFA = false;
            break;
        }
    }

    if (isActuallyDFA) {
        for (const auto& state : currentAutomaton->getStates()) {
            QMap<QString, int> symbolCount;
            for (const auto& t : currentAutomaton->getTransitions()) {
                if (t.getFromStateId() == state.getId()) {
                    for (const auto& sym : t.getSymbols()) {
                        symbolCount[sym]++;
                        if (symbolCount[sym] > 1) {
                            isActuallyDFA = false;
                            break;
                        }
                    }
                }
                if (!isActuallyDFA) break;
            }
            if (!isActuallyDFA) break;
        }
    }

    if (!isActuallyDFA) {
        showStyledMessageBox("Invalid DFA",
                             "This automaton is marked as DFA but violates DFA rules!\n\n"
                             "• It may have epsilon transitions\n"
                             "• It may have multiple transitions per symbol from the same state\n\n"
                             "Please fix the DFA or convert from NFA properly.",
                             QMessageBox::Warning);
        return;
    }

    try {
        DFAMinimizer minimizer;
        Automaton* minimizedDFA = minimizer.minimize(currentAutomaton);

        if (minimizedDFA) {
            QString id = generateAutomatonId();
            minimizedDFA->setName(currentAutomaton->getName() + " (Minimized)");

             
            int stateCount = minimizedDFA->getStateCount();
            int cols = qCeil(qSqrt(stateCount));
            int row = 0, col = 0;

            for (auto& state : minimizedDFA->getStates()) {
                state.setPosition(QPointF(100 + col * 120, 100 + row * 120));
                col++;
                if (col >= cols) {
                    col = 0;
                    row++;
                }
            }

            automatons[id] = minimizedDFA;
            updateAutomatonList();

             
            for (int i = 0; i < automatonList->count(); ++i) {
                QListWidgetItem* item = automatonList->item(i);
                if (item && item->data(Qt::UserRole).toString() == id) {
                    automatonList->setCurrentItem(item);
                    setCurrentAutomaton(minimizedDFA);
                    break;
                }
            }

            int originalStates = currentAutomaton->getStateCount();
            int minimizedStates = minimizedDFA->getStateCount();
            int reduction = originalStates - minimizedStates;

            QString resultMsg = QString(
                                    "DFA minimized successfully!\n\n"
                                    "Original states: %1\n"
                                    "Minimized states: %2\n"
                                    "States removed: %3\n\n"
                                    "The minimized DFA accepts the same language with fewer states."
                                    ).arg(originalStates).arg(minimizedStates).arg(reduction);

            showStyledMessageBox("Minimization Complete", resultMsg, QMessageBox::Information);

            statusBar()->showMessage(QString("DFA minimized: %1 → %2 states").arg(originalStates).arg(minimizedStates), 5000);
        } else {
            showStyledMessageBox("Error", "Failed to minimize DFA.", QMessageBox::Critical);
        }
    } catch (const std::exception& e) {
        showStyledMessageBox("Error",
                             QString("Failed to minimize DFA: %1").arg(e.what()),
                             QMessageBox::Critical);
    }
}

 

void MainWindow::onTestAutomaton() {
    if (!currentAutomaton) {
        QMessageBox::warning(this, "No Automaton", "Please create or select an automaton first.");
        return;
    }

     
    QDialog* testDialog = new QDialog(this);
    testDialog->setWindowTitle("🧪 Test Automaton");
    testDialog->resize(500, 400);

    QVBoxLayout* dialogLayout = new QVBoxLayout(testDialog);

     
    QLabel* infoLabel = new QLabel(QString("<b>Automaton:</b> %1 (%2)")
                                       .arg(currentAutomaton->getName())
                                       .arg(currentAutomaton->getType() == AutomatonType::DFA ? "DFA" : "NFA"));
    dialogLayout->addWidget(infoLabel);

    QHBoxLayout* inputLayout = new QHBoxLayout();
    QLabel* inputLabel = new QLabel("Test String:");
    QLineEdit* inputField = new QLineEdit();
    inputField->setPlaceholderText("Enter string to test, e.g., 010101");
    QPushButton* testBtn = new QPushButton("✓ Test");
    testBtn->setStyleSheet(
        "QPushButton { background-color: #28a745; color: white; padding: 6px 15px; font-weight: bold; border-radius: 3px; }"
        "QPushButton:hover { background-color: #218838; }"
    );

    inputLayout->addWidget(inputLabel);
    inputLayout->addWidget(inputField);
    inputLayout->addWidget(testBtn);
    dialogLayout->addLayout(inputLayout);

     
    QTextEdit* resultsText = new QTextEdit();
    resultsText->setReadOnly(true);
    resultsText->setStyleSheet("QTextEdit { background-color: #1e1e1e; color: white; font-family: 'Courier New'; }");
    dialogLayout->addWidget(resultsText);

     
    QPushButton* closeBtn = new QPushButton("Close");
    closeBtn->setStyleSheet("QPushButton { padding: 6px 20px; }");
    connect(closeBtn, &QPushButton::clicked, testDialog, &QDialog::accept);
    dialogLayout->addWidget(closeBtn);

     
    connect(testBtn, &QPushButton::clicked, [=]() {
        QString input = inputField->text();
        if (input.isEmpty()) {
            resultsText->append("<span style='color: orange;'>⚠ Please enter a string to test</span><br>");
            return;
        }

        bool accepted = currentAutomaton->accepts(input);
        QString result;
        if (accepted) {
            result = QString("<span style='color: #4caf50; font-weight: bold;'>✅ ACCEPTED</span> - Input: \"%1\"").arg(input);
        } else {
            result = QString("<span style='color: #f44336; font-weight: bold;'>❌ REJECTED</span> - Input: \"%1\"").arg(input);
        }
        resultsText->append(result + "<br>");
        inputField->clear();
    });

     
    connect(inputField, &QLineEdit::returnPressed, testBtn, &QPushButton::click);

    testDialog->exec();
    delete testDialog;
}

void MainWindow::onTraceAutomaton() {
    if (!currentAutomaton) {
        QMessageBox::warning(this, "No Automaton", "Please create or select an automaton first.");
        return;
    }

     
    QDialog* traceDialog = new QDialog(this);
    traceDialog->setWindowTitle("🔍 Trace Automaton Execution");
    traceDialog->resize(700, 500);

    QVBoxLayout* layout = new QVBoxLayout(traceDialog);

     
    QLabel* infoLabel = new QLabel(QString("<b>Automaton:</b> %1 (%2)")
                                       .arg(currentAutomaton->getName())
                                       .arg(currentAutomaton->getType() == AutomatonType::DFA ? "DFA" : "NFA"));
    layout->addWidget(infoLabel);

    QHBoxLayout* inputLayout = new QHBoxLayout();
    QLabel* inputLabel = new QLabel("Input String:");
    QLineEdit* inputField = new QLineEdit();
    inputField->setPlaceholderText("Enter string to trace, e.g., 010101");
    QPushButton* traceBtn = new QPushButton("▶ Start Trace");
    traceBtn->setStyleSheet(
        "QPushButton { background-color: #6f42c1; color: white; padding: 6px 15px; font-weight: bold; border-radius: 3px; }"
        "QPushButton:hover { background-color: #5a32a3; }"
    );

    inputLayout->addWidget(inputLabel);
    inputLayout->addWidget(inputField);
    inputLayout->addWidget(traceBtn);
    layout->addLayout(inputLayout);

     
    QTextEdit* traceText = new QTextEdit();
    traceText->setReadOnly(true);
    traceText->setStyleSheet("QTextEdit { background-color: #1e1e1e; color: white; font-family: 'Courier New'; font-size: 10pt; }");
    layout->addWidget(traceText);

     
    QPushButton* closeBtn = new QPushButton("Close");
    closeBtn->setStyleSheet("QPushButton { padding: 6px 20px; }");
    connect(closeBtn, &QPushButton::clicked, traceDialog, &QDialog::accept);
    layout->addWidget(closeBtn);

     
    connect(traceBtn, &QPushButton::clicked, [=]() {
        QString input = inputField->text();
        if (input.isEmpty()) {
            traceText->append("<span style='color: orange;'>⚠ Please enter a string to trace</span><br>");
            return;
        }

        traceText->clear();
        traceText->append(QString("<div style='color: #4ec9b0; font-weight: bold;'>═══ EXECUTION TRACE ═══</div>"));
        traceText->append(QString("<div style='color: #9cdcfe;'>Input: \"%1\"</div>").arg(input));
        traceText->append(QString("<div style='color: #9cdcfe;'>Length: %1 symbols</div><br>").arg(input.length()));

         
        QString startState = currentAutomaton->getInitialStateId();
        if (startState.isEmpty()) {
            traceText->append("<div style='color: #f44336;'>❌ ERROR: No initial state defined!</div>");
            return;
        }

         
        QSet<QString> currentStates;
        currentStates.insert(startState);
        currentStates = currentAutomaton->epsilonClosure(currentStates);

         
        auto formatStates = [](const QSet<QString>& states) {
            QStringList list = states.values();
            list.sort();
            return "{" + list.join(", ") + "}";
        };

        traceText->append(QString("<div style='color: #dcdcaa;'>➤ START at states: <b>%1</b></div><br>")
                              .arg(formatStates(currentStates)));

        bool valid = true;
        for (int i = 0; i < input.length(); i++) {
            QString symbol = QString(input[i]);
            QSet<QString> nextStates;

             
            for (const auto& stateId : currentStates) {
                for (const auto& trans : currentAutomaton->getTransitions()) {
                    if (trans.getFromStateId() == stateId && trans.hasSymbol(symbol)) {
                        nextStates.insert(trans.getToStateId());
                    }
                }
            }

             
            nextStates = currentAutomaton->epsilonClosure(nextStates);

            if (nextStates.isEmpty()) {
                traceText->append(QString("<div style='color: #f44336;'>Step %1: Read '%2' from states <b>%3</b> → <b>DEAD END</b> ❌</div>")
                                      .arg(i + 1).arg(symbol).arg(formatStates(currentStates)));
                valid = false;
                break;
            }

            traceText->append(QString("<div style='color: #4caf50;'>Step %1: Read '<b>%2</b>' from states <b>%3</b> → states <b>%4</b> ✓</div>")
                                  .arg(i + 1).arg(symbol).arg(formatStates(currentStates)).arg(formatStates(nextStates)));
            currentStates = nextStates;
        }

        traceText->append("<br>");

        if (valid) {
             
            bool isAccepted = false;
            QStringList finalStatesReached;
            
            for (const auto& stateId : currentStates) {
                const State* state = currentAutomaton->getState(stateId);
                if (state && state->getIsFinal()) {
                    isAccepted = true;
                    finalStatesReached.append(stateId);
                }
            }

            if (isAccepted) {
                traceText->append(QString("<div style='color: #4caf50; font-weight: bold; font-size: 12pt;'>✅ ACCEPTED - Ended in final state(s): %1</div>")
                                      .arg(finalStatesReached.join(", ")));
            } else {
                traceText->append(QString("<div style='color: #f44336; font-weight: bold; font-size: 12pt;'>❌ REJECTED - Ended in non-final state(s): %1</div>")
                                      .arg(formatStates(currentStates)));
            }
        } else {
            traceText->append("<div style='color: #f44336; font-weight: bold; font-size: 12pt;'>❌ REJECTED - No valid transition found</div>");
        }
    });

     
    connect(inputField, &QLineEdit::returnPressed, traceBtn, &QPushButton::click);

    traceDialog->exec();
    delete traceDialog;
}

void MainWindow::onFromRegex() {
    QDialog dialog(this);
    dialog.setWindowTitle("Create Automaton from Regular Expression");
    dialog.setMinimumWidth(600);
    dialog.setStyleSheet(
        "QDialog { background-color: #2b2b2b; }"
        "QLabel { color: white; }"
        "QLineEdit { "
        "   background-color: #1e1e1e; "
        "   color: white; "
        "   border: 2px solid #555; "
        "   padding: 8px; "
        "   font-size: 12pt; "
        "   font-family: 'Courier New';"
        "}"
        "QLineEdit:focus { border: 2px solid #0078d7; }"
        "QTextEdit { "
        "   background-color: #1e1e1e; "
        "   color: #ccc; "
        "   border: 1px solid #555; "
        "   padding: 8px;"
        "}"
        "QPushButton { "
        "   color: white; "
        "   background-color: #3e3e3e; "
        "   border: 1px solid #555; "
        "   padding: 8px 20px; "
        "   border-radius: 3px;"
        "}"
        "QPushButton:hover { background-color: #4e4e4e; }"
        "QPushButton:pressed { background-color: #2e2e2e; }"
        "QGroupBox { "
        "   color: white; "
        "   border: 2px solid #555; "
        "   border-radius: 5px; "
        "   margin-top: 15px; "
        "   padding-top: 10px; "
        "   font-weight: bold;"
        "}"
        "QGroupBox::title { "
        "   subcontrol-origin: margin; "
        "   left: 10px; "
        "   padding: 0 5px; "
        "   background-color: #2b2b2b;"
        "}"
        );

    QVBoxLayout* dialogLayout = new QVBoxLayout(&dialog);
    dialogLayout->setSpacing(15);

     
    QLabel* titleLabel = new QLabel("🔤 Regular Expression to NFA");
    titleLabel->setStyleSheet("font-size: 14pt; font-weight: bold; color: #ff9800; padding: 5px;");
    dialogLayout->addWidget(titleLabel);

     
    QLabel* inputLabel = new QLabel("Enter Regular Expression:");
    dialogLayout->addWidget(inputLabel);

    QLineEdit* regexInput = new QLineEdit();
    regexInput->setPlaceholderText("e.g., (a|b)*c, a+b*, (ab)*");
    dialogLayout->addWidget(regexInput);

     
    QLabel* validationLabel = new QLabel("");
    validationLabel->setWordWrap(true);
    validationLabel->setStyleSheet("padding: 5px;");
    dialogLayout->addWidget(validationLabel);

     
    QGroupBox* syntaxGroup = new QGroupBox("Supported Syntax");
    QVBoxLayout* syntaxLayout = new QVBoxLayout();
    
    QTextEdit* syntaxHelp = new QTextEdit();
    syntaxHelp->setReadOnly(true);
    syntaxHelp->setMaximumHeight(180);
    syntaxHelp->setHtml(
        "<style>table { width: 100%; } td { padding: 4px; } .op { color: #4ec9b0; font-weight: bold; } .desc { color: #ccc; }</style>"
        "<table>"
        "<tr><td class='op'>a, b, 0, 1, ...</td><td class='desc'>Literal characters</td></tr>"
        "<tr><td class='op'>ab</td><td class='desc'>Concatenation (implicit)</td></tr>"
        "<tr><td class='op'>a|b</td><td class='desc'>Union/Alternation</td></tr>"
        "<tr><td class='op'>a*</td><td class='desc'>Kleene Star (zero or more)</td></tr>"
        "<tr><td class='op'>a+</td><td class='desc'>Plus (one or more)</td></tr>"
        "<tr><td class='op'>a?</td><td class='desc'>Optional (zero or one)</td></tr>"
        "<tr><td class='op'>(...)</td><td class='desc'>Grouping</td></tr>"
        "<tr><td class='op'>E or ε</td><td class='desc'>Epsilon (empty string)</td></tr>"
        "</table>"
        );
    syntaxLayout->addWidget(syntaxHelp);
    syntaxGroup->setLayout(syntaxLayout);
    dialogLayout->addWidget(syntaxGroup);

     
    QGroupBox* examplesGroup = new QGroupBox("Examples");
    QVBoxLayout* examplesLayout = new QVBoxLayout();
    
    QTextEdit* examples = new QTextEdit();
    examples->setReadOnly(true);
    examples->setMaximumHeight(100);
    examples->setHtml(
        "<style>.ex { color: #4ec9b0; font-family: 'Courier New'; } .expl { color: #ccc; }</style>"
        "<div><span class='ex'>(a|b)*</span> <span class='expl'>- Any number of a's and b's</span></div>"
        "<div><span class='ex'>a+b*</span> <span class='expl'>- One or more a's followed by zero or more b's</span></div>"
        "<div><span class='ex'>(ab|cd)*</span> <span class='expl'>- Any number of ab or cd sequences</span></div>"
        "<div><span class='ex'>a(b|c)*d</span> <span class='expl'>- Starts with a, ends with d, b or c in middle</span></div>"
        );
    examplesLayout->addWidget(examples);
    examplesGroup->setLayout(examplesLayout);
    dialogLayout->addWidget(examplesGroup);

     
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    QPushButton* generateBtn = new QPushButton("✓ Generate NFA");
    generateBtn->setStyleSheet(
        "QPushButton { "
        "   background-color: #28a745; "
        "   border: none; "
        "   padding: 10px 25px; "
        "   font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #218838; }"
        "QPushButton:disabled { background-color: #666; color: #999; }"
        );
    generateBtn->setEnabled(false);

    QPushButton* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setStyleSheet(
        "QPushButton { "
        "   background-color: #6c757d; "
        "   border: none; "
        "   padding: 10px 25px;"
        "}"
        "QPushButton:hover { background-color: #5a6268; }"
        );

    btnLayout->addWidget(generateBtn);
    btnLayout->addWidget(cancelBtn);
    dialogLayout->addLayout(btnLayout);

     
    connect(regexInput, &QLineEdit::textChanged, [&, regexInput, validationLabel, generateBtn](const QString& text) {
        if (text.isEmpty()) {
            validationLabel->setText("");
            validationLabel->setStyleSheet("padding: 5px;");
            generateBtn->setEnabled(false);
            return;
        }

        RegexToNFA converter;
        QString error;
        bool valid = converter.isValidRegex(text, &error);

        if (valid) {
            validationLabel->setText("✓ Valid regular expression");
            validationLabel->setStyleSheet("color: #4caf50; padding: 5px; font-weight: bold;");
            generateBtn->setEnabled(true);
        } else {
            validationLabel->setText("✗ " + error);
            validationLabel->setStyleSheet("color: #f44336; padding: 5px; font-weight: bold;");
            generateBtn->setEnabled(false);
        }
    });

    connect(generateBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

     
    connect(regexInput, &QLineEdit::returnPressed, [generateBtn]() {
        if (generateBtn->isEnabled()) {
            generateBtn->click();
        }
    });

    if (dialog.exec() == QDialog::Accepted) {
        QString regex = regexInput->text();
        
        if (regex.isEmpty()) {
            return;
        }

         
        RegexToNFA converter;
        Automaton* nfa = converter.convert(regex);

        if (!nfa) {
            showStyledMessageBox("Error", 
                                 "Failed to convert regular expression to NFA.", 
                                 QMessageBox::Critical);
            return;
        }

         
        QString id = nfa->getId();
        automatons[id] = nfa;
        
        updateAutomatonList();

         
        for (int i = 0; i < automatonList->count(); ++i) {
            QListWidgetItem* item = automatonList->item(i);
            if (item && item->data(Qt::UserRole).toString() == id) {
                automatonList->setCurrentItem(item);
                setCurrentAutomaton(nfa);
                break;
            }
        }

        statusBar()->showMessage(QString("✓ Generated NFA from regex: /%1/").arg(regex), 5000);
    }
}

void MainWindow::onAutomatonModified() {
    updateProperties();
}

void MainWindow::onStateSelected(const QString& stateId) {
    currentSelectedStateId = stateId;
    updateProperties();
}

void MainWindow::onNew() {
    onNewAutomaton();
}

void MainWindow::onOpen() {
    showStyledMessageBox("Info", "Load functionality will be implemented in next phase.",
                         QMessageBox::Information);
}

void MainWindow::onSave() {
    if (!currentAutomaton) {
        showStyledMessageBox("Warning", "No automaton to save.", QMessageBox::Warning);
        return;
    }

    showStyledMessageBox("Info", "Save functionality will be implemented in next phase.",
                         QMessageBox::Information);
}

void MainWindow::onExit() {
    QMessageBox msgBox(this);
    msgBox.setStyleSheet(
        "QMessageBox { background-color: white; }"
        "QLabel { color: black; }"
        "QPushButton { color: black; background-color: #e0e0e0; border: 1px solid #999; padding: 5px 15px; }"
        );
    msgBox.setWindowTitle("Exit");
    msgBox.setText("Are you sure you want to exit?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setIcon(QMessageBox::Question);

    if (msgBox.exec() == QMessageBox::Yes) {
        qApp->quit();
    }
}


void MainWindow::updateProperties() {
     
    if (!typeLabel || !stateCountLabel || !transitionCountLabel ||
        !alphabetLabel || !transitionTable || !convertNFAtoDFABtn ||
        !minimizeDFABtn || !selectedStateLabel || !deleteStateBtn) {
        return;
    }

    if (!currentAutomaton) {
        typeLabel->setText("Type: N/A");
        stateCountLabel->setText("States: 0");
        transitionCountLabel->setText("Transitions: 0");
        alphabetLabel->setText("Alphabet: {}");
        transitionTable->setRowCount(0);
        convertNFAtoDFABtn->setEnabled(false);
        minimizeDFABtn->setEnabled(false);

        selectedStateLabel->setText("No state selected");
        selectedStateLabel->setStyleSheet(
            "background-color: #f5f5f5; color: #999; padding: 8px; "
            "border: 1px solid #ddd; border-radius: 3px;"
            );
        deleteStateBtn->setVisible(false);

        return;
    }

     
    QString typeText;
    bool isActuallyDFA = true;

    for (const auto& t : currentAutomaton->getTransitions()) {
        if (t.isEpsilonTransition()) {
            isActuallyDFA = false;
            break;
        }
    }

    if (isActuallyDFA) {
        for (const auto& state : currentAutomaton->getStates()) {
            QMap<QString, int> symbolCount;
            for (const auto& t : currentAutomaton->getTransitions()) {
                if (t.getFromStateId() == state.getId()) {
                    for (const auto& sym : t.getSymbols()) {
                        symbolCount[sym]++;
                        if (symbolCount[sym] > 1) {
                            isActuallyDFA = false;
                            break;
                        }
                    }
                }
                if (!isActuallyDFA) break;
            }
            if (!isActuallyDFA) break;
        }
    }

    if (currentAutomaton->isDFA()) {
        if (isActuallyDFA) {
            typeText = "Type: <span style='color: #0078d7; font-weight: bold;'>DFA ✓</span>";
        } else {
            typeText = "Type: <span style='color: #dc3545; font-weight: bold;'>DFA ⚠ (Invalid)</span>";
        }
    } else {
        typeText = "Type: <span style='color: #28a745; font-weight: bold;'>NFA</span>";
    }

    typeLabel->setText(typeText);

    QString initialStateId = currentAutomaton->getInitialStateId();
    QString stateText = QString("States: %1").arg(currentAutomaton->getStateCount());
    if (initialStateId.isEmpty() && currentAutomaton->getStateCount() > 0) {
        stateText += " <span style='color: #dc3545;'>⚠</span>";
    }
    stateCountLabel->setText(stateText);

    transitionCountLabel->setText(QString("Transitions: %1")
                                      .arg(currentAutomaton->getTransitionCount()));

    QSet<QString> alphabet = currentAutomaton->getAlphabet();
    QStringList alphList = alphabet.values();
    alphList.sort();

    QString alphText = QString("Alphabet: {%1}").arg(alphList.join(", "));
    if (alphabet.isEmpty()) {
        alphText = "Alphabet: <span style='color: #999;'>{empty}</span>";
    }
    alphabetLabel->setText(alphText);

     
    if (!currentSelectedStateId.isEmpty()) {
        const State* selectedState = currentAutomaton->getState(currentSelectedStateId);
        if (selectedState) {
            QString stateInfo = QString("Selected: <b>%1</b>").arg(selectedState->getLabel());
            if (selectedState->getIsInitial()) stateInfo += " [Initial]";
            if (selectedState->getIsFinal()) stateInfo += " [Final]";

             
            int transCount = currentAutomaton->getTransitionsFrom(currentSelectedStateId).size();
            stateInfo += QString("<br><small>%1 outgoing transition(s)</small>").arg(transCount);

            selectedStateLabel->setText(stateInfo);
            selectedStateLabel->setStyleSheet(
                "background-color: #e3f2fd; color: #0d47a1; padding: 8px; "
                "border: 1px solid #90caf9; border-radius: 3px; font-weight: bold;"
                );
            deleteStateBtn->setVisible(true);
        } else {
             
            currentSelectedStateId = "";
            selectedStateLabel->setText("No state selected");
            selectedStateLabel->setStyleSheet(
                "background-color: #f5f5f5; color: #999; padding: 8px; "
                "border: 1px solid #ddd; border-radius: 3px;"
                );
            deleteStateBtn->setVisible(false);
        }
    } else {
        selectedStateLabel->setText("Click a state to see delete options");
        selectedStateLabel->setStyleSheet(
            "background-color: #fff3cd; color: #856404; padding: 8px; "
            "border: 1px solid #ffc107; border-radius: 3px; font-style: italic;"
            );
        deleteStateBtn->setVisible(false);
    }

    updateTransitionTable();

     
    convertNFAtoDFABtn->setEnabled(currentAutomaton->isNFA() &&
                                   currentAutomaton->isValid());

    minimizeDFABtn->setEnabled(currentAutomaton->isDFA() &&
                               isActuallyDFA &&
                               currentAutomaton->isValid());
}

void MainWindow::updateTransitionTable() {
    if (!currentAutomaton || !transitionTable) return;

    const auto& transitions = currentAutomaton->getTransitions();
    transitionTable->setRowCount(transitions.size());

    int row = 0;
    for (const auto& trans : transitions) {
        transitionTable->setItem(row, 0,
                                 new QTableWidgetItem(trans.getFromStateId()));
        transitionTable->setItem(row, 1,
                                 new QTableWidgetItem(trans.getSymbolsString()));
        transitionTable->setItem(row, 2,
                                 new QTableWidgetItem(trans.getToStateId()));
        row++;
    }
}

void MainWindow::updateAutomatonList() {
    if (!automatonList) return;

    automatonList->clear();

    for (auto it = automatons.begin(); it != automatons.end(); ++it) {
        QString id = it.key();
        Automaton* automaton = it.value();

        QString displayName = automaton->getName();
        QString typeIndicator = automaton->isDFA() ? " 🔵" : " 🟢";

        QListWidgetItem* item = new QListWidgetItem(displayName + typeIndicator);
        item->setData(Qt::UserRole, id);

        QString tooltip = QString("%1\nType: %2\nStates: %3\nTransitions: %4")
                              .arg(automaton->getName())
                              .arg(automaton->isDFA() ? "DFA" : "NFA")
                              .arg(automaton->getStateCount())
                              .arg(automaton->getTransitionCount());
        item->setToolTip(tooltip);

        automatonList->addItem(item);
    }
}

QString MainWindow::generateAutomatonId() {
    return QString("auto_%1").arg(automatonCounter++);
}

void MainWindow::setCurrentAutomaton(Automaton* automaton) {
    currentAutomaton = automaton;
    currentSelectedStateId = "";
    if (canvas) {
        canvas->setAutomaton(automaton);
    }
    updateProperties();
}

void MainWindow::showStyledMessageBox(const QString& title, const QString& message,
                                      QMessageBox::Icon icon) {
    QMessageBox msgBox(this);
    msgBox.setStyleSheet(
        "QMessageBox { background-color: white; }"
        "QLabel { color: black; min-width: 300px; }"
        "QPushButton { color: black; background-color: #e0e0e0; border: 1px solid #999; "
        "padding: 5px 15px; min-width: 60px; }"
        "QPushButton:hover { background-color: #d0d0d0; }"
        );
    msgBox.setWindowTitle(title);
    msgBox.setText(message);
    msgBox.setIcon(icon);
    msgBox.exec();
}

void MainWindow::setupCentralTabs() {
     
    centralTabs = new QTabWidget(this);
    if (!centralTabs) {
        qCritical() << "Failed to create central tabs!";
        return;
    }

    centralTabs->setTabPosition(QTabWidget::North);
    centralTabs->setMovable(false);

     
    automatonTab = new QWidget();
    QVBoxLayout* automatonLayout = new QVBoxLayout(automatonTab);
    automatonLayout->setContentsMargins(0, 0, 0, 0);

     
    canvas = new AutomatonCanvas(automatonTab);
    if (!canvas) {
        qCritical() << "Failed to create AutomatonCanvas!";
        return;
    }
    automatonLayout->addWidget(canvas);

    centralTabs->addTab(automatonTab, "🤖 Automaton Designer");

     
    lexerWidget = new LexerWidget();
    if (!lexerWidget) {
        qCritical() << "Failed to create LexerWidget!";
        return;
    }

     
    if (automatonManager && lexerWidget) {
        lexerWidget->setAutomatonManager(automatonManager);
    } else {
        qWarning() << "AutomatonManager or LexerWidget is null, cannot set manager";
    }

    centralTabs->addTab(lexerWidget, "🔍 Lexical Analyzer");

    parserWidget = new ParserWidget();
    if (automatonManager && parserWidget) {
        parserWidget->setAutomatonManager(automatonManager);
    }
    centralTabs->addTab(parserWidget, "🌳 Parser & Parse Tree");

    semanticWidget = new SemanticAnalyzerWidget();
    if (automatonManager) semanticWidget->setAutomatonManager(automatonManager);
    centralTabs->addTab(semanticWidget, "🔬 Semantic Analysis");

     
    setCentralWidget(centralTabs);

     
    connect(centralTabs, &QTabWidget::currentChanged,
            this, &MainWindow::onTabChanged);
}

void MainWindow::onTabChanged(int index) {
     
    bool showDocks = (index == 0);

    if (toolsDock) toolsDock->setVisible(showDocks);
    if (automatonListDock) automatonListDock->setVisible(showDocks);
    if (propertiesDock) propertiesDock->setVisible(showDocks);
     

    switch(index) {
    case 0:
        statusBar()->showMessage("Automaton Designer", 3000);
        break;
    case 1:
        statusBar()->showMessage("Lexical Analyzer", 3000);
        break;
    case 2:
        statusBar()->showMessage("Parser & Parse Tree", 3000);
        break;
    case 3:
        statusBar()->showMessage("Semantic Analysis & Code Generation", 3000);
        break;
    }
}
 

void MainWindow::onPlaySimulation() {
    if (!currentAutomaton) return;

    if (!isSimulating) {
         
        bool ok;
        QString text = QInputDialog::getText(this, "Simulation Input",
                                             "Enter input string:", QLineEdit::Normal,
                                             "", &ok);
        if (!ok) return;

        simulationInput = text;
        simulationStepIndex = 0;
        currentSimulationStates.clear();
        currentSimulationStates.insert(currentAutomaton->getInitialStateId());
        
         
        if (currentAutomaton->getType() == AutomatonType::NFA) {
            currentSimulationStates = currentAutomaton->epsilonClosure(currentSimulationStates);
        }

        isSimulating = true;
        simulationAccepted = false;
        simulationRejected = false;

         
        playAction->setEnabled(false);
        stepAction->setEnabled(false);
        stopAction->setEnabled(true);
        resetAction->setEnabled(false);
        
         
        if (canvas) canvas->setEnabled(false);

         
        simulationTimer->start(speedSlider->value());
        
         
        if (canvas) {
            canvas->setActiveStates(currentSimulationStates);
            statusBar()->showMessage(QString("Simulation started. Input: '%1'").arg(simulationInput));
        }
    } else {
         
    }
}

void MainWindow::onStepSimulation() {
    if (!currentAutomaton) return;

    if (!isSimulating) {
         
        bool ok;
        QString text = QInputDialog::getText(this, "Simulation Input",
                                             "Enter input string:", QLineEdit::Normal,
                                             "", &ok);
        if (!ok) return;

        simulationInput = text;
        simulationStepIndex = 0;
        currentSimulationStates.clear();
        currentSimulationStates.insert(currentAutomaton->getInitialStateId());

         
        if (currentAutomaton->getType() == AutomatonType::NFA) {
            currentSimulationStates = currentAutomaton->epsilonClosure(currentSimulationStates);
        }

        isSimulating = true;
        simulationAccepted = false;
        simulationRejected = false;
        
         
        playAction->setEnabled(true);  
        stopAction->setEnabled(true);
        resetAction->setEnabled(true);
        
        if (canvas) {
            canvas->setActiveStates(currentSimulationStates);
            canvas->setEnabled(false);
            statusBar()->showMessage(QString("Step 0. Active states: %1").arg(currentSimulationStates.size()));
        }
    } else {
         
        advanceSimulation();
    }
}

void MainWindow::onStopSimulation() {
    isSimulating = false;
    simulationTimer->stop();
    
    playAction->setEnabled(true);
    stepAction->setEnabled(true);
    stopAction->setEnabled(false);
    resetAction->setEnabled(true);
    
    if (canvas) {
        canvas->clearActiveElements();
        canvas->setEnabled(true);
    }
    statusBar()->showMessage("Simulation stopped.");
}

void MainWindow::onResetSimulation() {
    onStopSimulation();
    if (canvas) canvas->update();
}

void MainWindow::onSimulationTimerTimeout() {
    advanceSimulation();
}

void MainWindow::onSpeedChanged(int value) {
    speedLabel->setText(QString("%1s").arg(value / 1000.0, 0, 'f', 1));
    if (simulationTimer->isActive()) {
        simulationTimer->setInterval(value);
    }
}

void MainWindow::advanceSimulation() {
    if (!currentAutomaton || !isSimulating) return;

    if (simulationStepIndex >= simulationInput.length()) {
         
        simulationTimer->stop();
        
         
        bool accepted = false;
        for (const auto& stateId : currentSimulationStates) {
            State* state = currentAutomaton->getState(stateId);
            if (state && state->getIsFinal()) {
                accepted = true;
                break;
            }
        }
        
        QString result = accepted ? "ACCEPTED" : "REJECTED";
        statusBar()->showMessage(QString("Simulation Finished: %1").arg(result));
        
        if (accepted) {
            showStyledMessageBox("Result", "String Accepted! ✓", QMessageBox::Information);
        } else {
            showStyledMessageBox("Result", "String Rejected ✗", QMessageBox::Warning);
        }
        
        onStopSimulation();
        return;
    }

     
    QChar inputChar = simulationInput[simulationStepIndex];
    QString symbol = QString(inputChar);
    
    QSet<QString> nextStates;
    QSet<QString> activeTransitions;

    for (const auto& currentStateId : currentSimulationStates) {
         
        for (const auto& trans : currentAutomaton->getTransitions()) {
            if (trans.getFromStateId() == currentStateId && trans.hasSymbol(symbol)) {
                nextStates.insert(trans.getToStateId());
                activeTransitions.insert(trans.getFromStateId() + "|" + trans.getToStateId());
            }
        }
    }

     
    if (currentAutomaton->getType() == AutomatonType::NFA) {
        nextStates = currentAutomaton->epsilonClosure(nextStates);
    }

     
    currentSimulationStates = nextStates;
    simulationStepIndex++;

     
    if (canvas) {
        canvas->setActiveStates(currentSimulationStates);
        canvas->setActiveTransitions(activeTransitions);
    }
    
    statusBar()->showMessage(QString("Step %1: Processed '%2'. Active states: %3")
                                 .arg(simulationStepIndex)
                                 .arg(symbol)
                                 .arg(currentSimulationStates.size()));
                                 
     
    if (currentSimulationStates.isEmpty() && currentAutomaton->getType() == AutomatonType::DFA) {
        simulationTimer->stop();
        showStyledMessageBox("Result", "String Rejected (Dead State) ✗", QMessageBox::Warning);
        onStopSimulation();
    }
}
