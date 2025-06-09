#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_UI_Learn.h"

class UI_Learn : public QMainWindow
{
    Q_OBJECT

public:
    UI_Learn(QWidget *parent = nullptr);
    ~UI_Learn();

private:
    Ui::UI_LearnClass ui;
};

