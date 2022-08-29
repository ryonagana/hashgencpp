#include "appwindow.h"

#include <QApplication>
#include "bin_check.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AppWindow w;

    if(w.getError_counter() > 0){
        exit(-1);
    }

    w.show();
    return a.exec();
}
