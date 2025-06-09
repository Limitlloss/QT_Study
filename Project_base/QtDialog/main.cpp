#include "QtDialog.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QtDialog window;
    window.show();
    return app.exec();
}
