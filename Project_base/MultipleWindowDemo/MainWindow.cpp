#include "MainWindow.h"
#include "SubWindow.h"
#include <QVBoxLayout>
#include <QWidget>
#include "MainWindow.h"
#include "SubWindow.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QApplication>  // 关键修复！必须添加此头文件

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QPushButton* createBtn = new QPushButton("创建新窗口", centralWidget);
    QPushButton* exitBtn = new QPushButton("退出程序", centralWidget);

    QVBoxLayout* layout = new QVBoxLayout(centralWidget);
    layout->addWidget(createBtn);
    layout->addWidget(exitBtn);

    // 使用新式信号槽语法[7](@ref)
    connect(createBtn, &QPushButton::clicked, this, &MainWindow::createNewWindow);
    connect(exitBtn, &QPushButton::clicked, qApp, &QApplication::quit);

    setWindowTitle("多窗口管理Demo");
    resize(300, 200);
}

void MainWindow::createNewWindow() {
    SubWindow* newWindow = new SubWindow(++windowCount);
    newWindow->show();  // 非模态显示[9](@ref)
}