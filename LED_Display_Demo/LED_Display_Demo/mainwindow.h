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
	MainWindow(QWidget* parent = nullptr);
	~MainWindow();
	void toggleRunState();
	void onMainTimerTick();

	void updateDisplayColor();
	void closeEvent(QCloseEvent* event);
	void on_radioButton_red_clicked();
	void on_radioButton_blue_clicked();
	void on_radioButton_green_clicked();
	void on_radioButton_yellow_clicked();
	void on_radioButton_cyan_clicked();
	void on_radioButton_white_clicked();
	void on_radioButton_purple_clicked();
    void onBrightnessChanged(int value);
	void syncSpinBoxesWithDisplayWindow();
    //QColor getBaseColor() const;

private:
	QTimer* mainTimer = nullptr;
	bool isRunning = false;  // 初始状态为未运行

	QTimer* autoRunTimer = nullptr;
	bool autoColorChangeEnabled = false;

	QTimer* brightnessTimer = nullptr;
	int currentBrightness = 0;
	bool autoBrightnessRunning = false;

	QColor baseColor = Qt::red;
	int brightnessValue = 255;
	Ui::MainWindow* ui;
	DisplayWindow* displayWindow = nullptr;
private slots:
	void on_checkBox_hide_stateChanged(int state);
	void handleDisplayVisibilityChanged(bool isVisible);

};
#endif // MAINWINDOW_H
