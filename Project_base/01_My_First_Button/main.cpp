#include "_My_First_Button.h"
#include <QtWidgets/QApplication>
#include <QPushButton>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QWidget window;
    QPushButton *quit = new QPushButton(&window);
    quit->setText("Quit");
    quit->setParent(&window);
    window.show();
    //_My_First_Button w;
    //w.show();
    return a.exec();
}
