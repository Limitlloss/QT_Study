#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "AgingFeature.h"

#include <QMainWindow>
#include <QTimer>
#include "DisplayWindow.h"
#include "LedEffectManager.h"
#include "ILedFeature.h"
#include <vector>
#include "autoBrightnessFeature.h"
#include "ProgramStep.h"
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
    void setAllFeatureRunning(bool enable);
private:

    Ui::MainWindow* ui;
    QVector<ProgramStep> playlist;
    int playlistIndex = 0;
    QTimer* playlistTimer = nullptr;
    QVector<ProgramStep> programPlaylist;  // 播放列表
    int currentProgramIndex = 0;      // 当前播放位置
    void buildProgramPlaylist();
    void playNextStep();
    AutoBrightnessFeature* autoBrightnessFeature = nullptr;
    AgingFeature* agingFeature = nullptr;
    LedEffectManager* ledManager = nullptr;
    DisplayWindow* displayWindow = nullptr;

    QTimer* mainTimer = nullptr;
    QTimer* autoRunTimer = nullptr;
    std::vector<ILedFeature*> featureModules;

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
    void onTabSwitched(int index);  
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
