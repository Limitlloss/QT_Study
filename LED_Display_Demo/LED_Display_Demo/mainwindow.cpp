#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "displayWindow.h"
MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->horizontalSlider_brightness->setRange(10, 200);  // 默认100为原色，200更亮，10更暗
    ui->horizontalSlider_brightness->setValue(100);

	if (!displayWindow) {
		displayWindow = new DisplayWindow();  // 创建显示窗口
		QPoint mainPos = this->pos();
		QSize mainSize = this->size();
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
    connect(ui->horizontalSlider_brightness, &QSlider::valueChanged, this, &MainWindow::onBrightnessChanged);


}
// QColor DisplayWindow::getBaseColor() const {
//     return bgColor;  // 如果你有专门的原色记录变量，可以返回那个
// }
void MainWindow::closeEvent(QCloseEvent* event) {
	if (displayWindow)
		displayWindow->close();
	QMainWindow::closeEvent(event);
}
void MainWindow::onBrightnessChanged(int value) {
    if (!displayWindow) return;

    // QColor baseColor = displayWindow->getBaseColor();  // 你需要添加 getter 来获取原始颜色
    // QColor adjustedColor = baseColor;

    if (value > 100) {
        // adjustedColor = baseColor.lighter(value);  // >100：变亮
    } else {
        // adjustedColor = baseColor.darker(200 - value);  // <100：变暗
    }

    // displayWindow->setBackgroundColor(adjustedColor);
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

