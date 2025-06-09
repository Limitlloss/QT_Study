#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "liveeditor.h"
#include <QComboBox>
#include <QLabel>
#include <QMainWindow>
#include <QTextEdit>
#include <QNetworkReply>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow();
    LiveEditor *liveEditor;
    QTabWidget *tab;
};

#endif // MAINWINDOW_H
