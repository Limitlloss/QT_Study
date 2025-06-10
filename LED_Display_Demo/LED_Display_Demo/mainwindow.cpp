#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "displayWindow.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    if(!displayWindow){
        displayWindow = new DisplayWindow();  // 创建显示窗口
        QPoint mainPos=this->pos();
        QSize mainSize=this->size();
        displayWindow->move(mainPos.x() + mainSize.width() + 20, mainPos.y()); // 右边偏移 20 像素
        displayWindow->show();
    }
    displayWindow->setWindowTitle("LED 显示窗口");
    displayWindow->show();                // 弹出显示窗口（独立）
    connect(ui->radioButton_red, &QRadioButton::clicked, this, &MainWindow::on_radioButton_red_clicked);
    connect(ui->radioButton_blue, &QRadioButton::clicked, this, &MainWindow::on_radioButton_blue_clicked);
    connect(ui->radioButton_green, &QRadioButton::clicked, this, &MainWindow::on_radioButton_green_clicked);
    connect(ui->radioButton_light_blue, &QRadioButton::clicked, this, &MainWindow::on_radioButton_light_blue_clicked);
    connect(ui->radioButton_purple, &QRadioButton::clicked, this, &MainWindow::on_radioButton_purple_clicked);
    connect(ui->radioButton_yellow, &QRadioButton::clicked, this, &MainWindow::on_radioButton_yellow_clicked);
    connect(ui->radioButton_orange, &QRadioButton::clicked, this, &MainWindow::on_radioButton_orange_clicked);

}
void MainWindow::closeEvent(QCloseEvent *event) {
    if (displayWindow)
        displayWindow->close();
    QMainWindow::closeEvent(event);
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::on_radioButton_red_clicked() {
    if (displayWindow)
        displayWindow->setBackgroundColor(Qt::red);
}

void MainWindow::on_radioButton_blue_clicked() {
    if (displayWindow)
        displayWindow->setBackgroundColor(Qt::blue);
}

void MainWindow::on_radioButton_green_clicked() {
    if (displayWindow)
        displayWindow->setBackgroundColor(Qt::green);
}

void MainWindow::on_radioButton_yellow_clicked() {
    if (displayWindow)
        displayWindow->setBackgroundColor(Qt::yellow);
}

void MainWindow::on_radioButton_light_blue_clicked() {
    if (displayWindow)
        displayWindow->setBackgroundColor(Qt::cyan); // 浅蓝色
}

void MainWindow::on_radioButton_orange_clicked() {
    if (displayWindow)
        displayWindow->setBackgroundColor(QColor(255, 127, 0)); // 自定义橙色
}

void MainWindow::on_radioButton_purple_clicked() {
    if (displayWindow)
        displayWindow->setBackgroundColor(QColor(128, 0, 128)); // 自定义紫色
}

