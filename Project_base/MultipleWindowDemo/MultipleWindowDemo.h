#pragma once

#include <QtWidgets/QWidget>
#include "ui_MultipleWindowDemo.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MultipleWindowDemoClass; };
QT_END_NAMESPACE

class MultipleWindowDemo : public QWidget
{
    Q_OBJECT

public:
    MultipleWindowDemo(QWidget *parent = nullptr);
    ~MultipleWindowDemo();

private:
    Ui::MultipleWindowDemoClass *ui;
};

