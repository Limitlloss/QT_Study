#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "displayWindow.h"
MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	if (!displayWindow) {
		displayWindow = new DisplayWindow();  // 创建显示窗口
		QPoint mainPos = this->pos();
		QSize mainSize = this->size();
		displayWindow->move(mainPos.x() + mainSize.width() + 20, mainPos.y()); // 右边偏移 20 像素
		displayWindow->show();
	}
	displayWindow->setWindowTitle("LED 显示窗口");
	displayWindow->show();                // 弹出显示窗口（独立）
	ui->horizontalSlider_brightness->setRange(0, 255);
	ui->spinBox_brightness->setRange(0, 255);

	connect(ui->horizontalSlider_brightness, &QSlider::valueChanged,
		ui->spinBox_brightness, &QSpinBox::setValue);

	connect(ui->spinBox_brightness, QOverload<int>::of(&QSpinBox::valueChanged),
		ui->horizontalSlider_brightness, &QSlider::setValue);

	connect(ui->radioButton_red, &QRadioButton::clicked, this, &MainWindow::on_radioButton_red_clicked);
	connect(ui->radioButton_blue, &QRadioButton::clicked, this, &MainWindow::on_radioButton_blue_clicked);
	connect(ui->radioButton_green, &QRadioButton::clicked, this, &MainWindow::on_radioButton_green_clicked);
	connect(ui->radioButton_cyan, &QRadioButton::clicked, this, &MainWindow::on_radioButton_cyan_clicked);
	connect(ui->radioButton_purple, &QRadioButton::clicked, this, &MainWindow::on_radioButton_purple_clicked);
	connect(ui->radioButton_yellow, &QRadioButton::clicked, this, &MainWindow::on_radioButton_yellow_clicked);
	connect(ui->radioButton_white, &QRadioButton::clicked, this, &MainWindow::on_radioButton_white_clicked);
	connect(ui->horizontalSlider_brightness, &QSlider::valueChanged, this, &MainWindow::onBrightnessChanged);


}

void MainWindow::closeEvent(QCloseEvent* event) {
	if (displayWindow)
		displayWindow->close();
	QMainWindow::closeEvent(event);
}
void MainWindow::onBrightnessChanged(int value) {
	brightnessValue = value;
	updateDisplayColor();
}

void MainWindow::updateDisplayColor() {
	int r = baseColor.red() == 255 ? brightnessValue : 0;
	int g = baseColor.green() == 255 ? brightnessValue : 0;
	int b = baseColor.blue() == 255 ? brightnessValue : 0;

	displayWindow->setBackgroundColor(QColor(r, g, b));
}


MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_radioButton_red_clicked() {
	baseColor = QColor(255, 0, 0); // 红
	updateDisplayColor();
}

void MainWindow::on_radioButton_blue_clicked() {
	baseColor = QColor(0, 0, 255); // 蓝
	updateDisplayColor();
}

void MainWindow::on_radioButton_green_clicked() {
	baseColor = QColor(0, 255, 0); // 绿
	updateDisplayColor();
}

void MainWindow::on_radioButton_yellow_clicked() {
	baseColor = QColor(255, 255, 0); // 黄
	updateDisplayColor();
}

void MainWindow::on_radioButton_cyan_clicked() {
	baseColor = QColor(0, 255, 255); // 青（浅蓝）
	updateDisplayColor();
}

void MainWindow::on_radioButton_purple_clicked() {
	baseColor = QColor(255, 0, 255); // 紫（品红）
	updateDisplayColor();
}

void MainWindow::on_radioButton_white_clicked() {
	baseColor = QColor(255, 255, 255); // 橙
	updateDisplayColor();
}


