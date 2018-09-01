#include "mainwidget.h"
#include <QApplication>
#include <QDateTime>
#include <QDir>

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

    QDir qDir;
    qDir.cd(".");
    cout << qDir.absolutePath();

    return a.exec();
}
