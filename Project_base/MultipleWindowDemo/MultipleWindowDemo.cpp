#include "MultipleWindowDemo.h"

MultipleWindowDemo::MultipleWindowDemo(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MultipleWindowDemoClass())
{
    ui->setupUi(this);
}

MultipleWindowDemo::~MultipleWindowDemo()
{
    delete ui;
}

