#include "mainwidget.h"
#include <QApplication>

bool IsBig_Endian()
{
    unsigned short test = 0x1234;
    if(*( (unsigned char*) &test ) == 0x12)
       return true;
   else
       return false;
}//IsBig_Endian()

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWidget w;
    w.show();



    return a.exec();
}
