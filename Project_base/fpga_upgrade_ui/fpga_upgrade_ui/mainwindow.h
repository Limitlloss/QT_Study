#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "UpgradeWorker.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void selectFile();
    void startUpgrade();
    void appendLog(const QString &text);
    void upgradeFinished(bool success);

private:
    Ui::MainWindow *ui;
    QString binFilePath;
    QThread workerThread;
    UpgradeWorker *worker = nullptr;
};
#endif // MAINWINDOW_H
