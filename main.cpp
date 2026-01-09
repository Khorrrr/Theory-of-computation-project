#include <QApplication>  
#include "MainWindow.h"  

 
 
 
int main(int argc, char *argv[]) {
     
     
     
    QApplication app(argc, argv);

     
     
    app.setApplicationName("Compiler Project");
     
    app.setApplicationVersion("1.0");
     
     
    app.setOrganizationName("University");

     
     
    MainWindow window;
     
     
    window.show();

     
     
     
     
    return app.exec();
}
