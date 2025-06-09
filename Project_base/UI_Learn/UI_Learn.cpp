#include "UI_Learn.h"

UI_Learn::UI_Learn(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    ui.actionNewFile->setIcon(QIcon(":/UI_Learn/ProjectResourceFile/Luffy.jpg"));
}

UI_Learn::~UI_Learn()
{}

