#pragma once

#include <QtWidgets/QWidget>
#include "ui__My_First_Button.h"

class _My_First_Button : public QWidget
{
    Q_OBJECT

public:
    _My_First_Button(QWidget *parent = nullptr);
    ~_My_First_Button();

private:
    Ui::_My_First_ButtonClass ui;
};
