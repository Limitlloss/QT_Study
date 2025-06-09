#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtDialog.h"

class QtDialog : public QMainWindow
{
    Q_OBJECT

public:
    QtDialog(QWidget *parent = nullptr);
    ~QtDialog();

private:
    Ui::QtDialogClass ui;
};

