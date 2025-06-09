#include "UI_Learn.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    UI_Learn window;
    window.show();
    return app.exec();
}
