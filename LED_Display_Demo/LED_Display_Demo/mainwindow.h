#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "DisplayWindow.h"
#include "LedEffectManager.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow* ui;

    DisplayWindow* displayWindow = nullptr;
    LedEffectManager* ledManager = nullptr;

    QTimer* mainTimer = nullptr;
    QTimer* autoRunTimer = nullptr;
    bool autoColorChangeEnabled = false;
    bool isRunning = false;
    QColor baseColor = Qt::red;
    int brightnessValue = 255;
    int currentBrightness = 0;

    void setupUiLogic();
    void setupColorButtons();
    void syncSpinBoxesWithDisplayWindow();
    void updateDisplayColor();
    void showEvent(QShowEvent* event);
private slots:
    void toggleRunState();
    void onMainTimerTick();
    void onBrightnessChanged(int value);

    void on_checkBox_hide_stateChanged(int state);
    void on_radioButton_red_clicked();
    void on_radioButton_blue_clicked();
    void on_radioButton_green_clicked();
    void on_radioButton_yellow_clicked();
    void on_radioButton_cyan_clicked();
    void on_radioButton_white_clicked();
    void on_radioButton_purple_clicked();

    void handleDisplayVisibilityChanged(bool isVisible);
};
#endif // MAINWINDOW_H
