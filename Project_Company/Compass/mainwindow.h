#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "liveeditor.h"
#include <QComboBox>
#include <QLabel>
#include <QMainWindow>
#include <QTextEdit>
#include <QNetworkReply>

// 主窗口类，继承自QMainWindow
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    // 主窗口构造函数
    MainWindow();
    // 指向LiveEditor对象的指针
    LiveEditor* liveEditor;
    // 指向QTabWidget对象的指针
    QTabWidget* tab;
};

#endif // MAINWINDOW_H
