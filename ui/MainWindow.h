#ifndef MAINWINDOW_H  
#define MAINWINDOW_H

#include <QMainWindow>    
#include <QToolBar>       
#include <QMenuBar>       
#include <QStatusBar>     
#include <QDockWidget>    
#include <QListWidget>    
#include <QTextEdit>      
#include <QPushButton>    
#include <QLineEdit>      
#include <QLabel>         
#include <QVBoxLayout>    
#include <QHBoxLayout>    
#include <QGroupBox>      
#include <QRadioButton>   
#include <QTableWidget>   
#include <QMap>           
#include <QMessageBox>    
#include <QTabWidget>
#include <QTimer>
#include <QSlider>

 
#include "./src/ui/Automaton/AutomatonCanvas.h"           
#include "./src/models/Automaton/Automaton.h"             
#include "./src/ui/LexicalAnalysis/LexerWidget.h"         
#include "./src/utils/LexicalAnalysis/AutomatonManager.h"  
#include "./src/ui/Grammar/ParserWidget.h"                 
#include "./src/ui/Semantic/SemanticAnalyzerWidget.h"     

 
class MainWindow : public QMainWindow {
    Q_OBJECT  
              

private:
     
    QTabWidget* centralTabs;           
    QWidget* automatonTab;             
    LexerWidget* lexerWidget;          
    AutomatonManager* automatonManager;  
    ParserWidget* parserWidget;         
    SemanticAnalyzerWidget* semanticWidget;  

     
    AutomatonCanvas* canvas;           

     
    QMap<QString, Automaton*> automatons;  
    Automaton* currentAutomaton;            
    int automatonCounter;                   
    QString currentSelectedStateId;         

     
    QDockWidget* toolsDock;            
    QDockWidget* automatonListDock;    
    QDockWidget* propertiesDock;       
     

     
    QRadioButton* selectModeBtn;        
    QRadioButton* addStateModeBtn;      
    QRadioButton* addTransitionModeBtn;  
    QRadioButton* deleteModeBtn;        
    QPushButton* clearCanvasBtn;        

     
    QListWidget* automatonList;         
    QPushButton* newAutomatonBtn;       
    QPushButton* deleteAutomatonBtn;    
    QPushButton* renameAutomatonBtn;    

     
    QLabel* typeLabel;                  
    QLabel* stateCountLabel;            
    QLabel* transitionCountLabel;       
    QLabel* alphabetLabel;              
    QLabel* selectedStateLabel;         
    QPushButton* deleteStateBtn;        
    
    QTableWidget* transitionTable;      
    QPushButton* convertNFAtoDFABtn;    
    QPushButton* minimizeDFABtn;        
    QPushButton* fromRegexBtn;          
    QPushButton* testAutomatonBtn;      
    QPushButton* traceAutomatonBtn;     
     
     
     
     
     

     
    QAction* newAction;                 
    QAction* openAction;                
    QAction* saveAction;                
    QAction* exitAction;                
    QAction* aboutAction;               

    QAction* selectAction;              
    QAction* addStateAction;            
    QAction* addTransitionAction;       
    QAction* deleteAction;              
    QAction* convertAction;             
    QAction* minimizeAction;            

     
    QAction* playAction;
    QAction* stepAction;
    QAction* stopAction;
    QAction* resetAction;
    QSlider* speedSlider;
    QLabel* speedLabel;

     
    QTimer* simulationTimer;
    bool isSimulating;
    QString simulationInput;
    int simulationStepIndex;
    QSet<QString> currentSimulationStates;
    bool simulationAccepted;
    bool simulationRejected;

public:
     
    explicit MainWindow(QWidget *parent = nullptr);

     
    ~MainWindow();

private slots:
     
    void onSelectMode();              
    void onAddStateMode();            
    void onAddTransitionMode();       
    void onDeleteMode();              

     
    void onNewAutomaton();            
    void onDeleteAutomaton();         
    void onRenameAutomaton();         
    void onAutomatonSelected(QListWidgetItem* item);  
    void onClearCanvas();             

     
    void onConvertNFAtoDFA();         
    void onMinimizeDFA();             

     
     
     
    void onTestAutomaton();           
    void onTraceAutomaton();          
    void onFromRegex();               

     
    void onPlaySimulation();
    void onStepSimulation();
    void onStopSimulation();
    void onResetSimulation();
    void onSimulationTimerTimeout();
    void onSpeedChanged(int value);
    void advanceSimulation();

     
    void onAutomatonModified();       
    void onStateSelected(const QString& stateId);  
    void onDeleteStateOrTransition();  

     
    void onNew();                     
    void onOpen();                    
    void onSave();                    
    void onExit();                    
     

     
    void onTabChanged(int index);     

private:
     
     
    void createToolbar();             
    void createDockWidgets();         
    void createToolsPanel();          
    void createAutomatonListPanel();  
    void createPropertiesPanel();     
     
    void setupCentralTabs();          

     
    void updateProperties();          
    void updateTransitionTable();     
    void updateAutomatonList();       

     
    QString generateAutomatonId();    
    void setCurrentAutomaton(Automaton* automaton);  

     
    void showStyledMessageBox(const QString& title, const QString& message,
                              QMessageBox::Icon icon = QMessageBox::Information);
};

#endif  
