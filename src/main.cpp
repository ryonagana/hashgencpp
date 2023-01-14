#include "appwindow.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    AppWindow window;

    if(window.getError_counter() > 0){
        exit(-1);
    }

    window.show();
    return application.exec();
}
