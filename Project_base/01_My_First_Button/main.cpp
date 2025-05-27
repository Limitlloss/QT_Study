#include "_My_First_Button.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    _My_First_Button w;
    w.show();
    return a.exec();
}
