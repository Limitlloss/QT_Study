#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "displayWindow.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void closeEvent(QCloseEvent *event);
    void on_radioButton_red_clicked();
    void on_radioButton_blue_clicked() ;
    void on_radioButton_green_clicked() ;
    void on_radioButton_yellow_clicked() ;
    void on_radioButton_light_blue_clicked() ;
    void on_radioButton_orange_clicked() ;
    void on_radioButton_purple_clicked() ;
    void on_btnRed_clicked();
    void onColorChanged();

private:
    Ui::MainWindow *ui;
    DisplayWindow *displayWindow =nullptr;
};
#endif // MAINWINDOW_H
