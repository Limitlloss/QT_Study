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
	displayWindow->setEnableDoubleClickFullScreen(ui->checkBox_double_fullScreen->isChecked());
	displayWindow->setTopMost(ui->checkBox_windowTopMost->isChecked());
	// 初始化时设定锁定状态（可选）
	displayWindow->setLockGeometry(ui->checkBox_lock->isChecked());
	autoRunTimer = new QTimer(this);
	autoRunTimer->setInterval(50);  // 主调度器频率，可适当调整
	// 初始化定时器
	mainTimer = new QTimer(this);
	mainTimer->setInterval(100);  // 默认100ms，可后续绑定 spinBox_speed
	connect(mainTimer, &QTimer::timeout, this, &MainWindow::onMainTimerTick);

	// 默认按钮文本
	ui->pushButton_run_space->setText("运行");

	// 绑定按钮点击
	connect(ui->pushButton_run_space, &QPushButton::clicked, this, &MainWindow::toggleRunState);
	connect(autoRunTimer, &QTimer::timeout, this, [=]() {
		if (!isRunning) return;

		if (autoColorChangeEnabled) {
			currentBrightness += 1;
			if (currentBrightness > 255) currentBrightness = 0;
			ui->horizontalSlider_brightness->setValue(currentBrightness);
		}

		// 未来的其他自动功能也可以写在这里，如：
		// if (autoAgingEnabled) { agingStep(); }
		});


	connect(ui->pushButton_color_change_auto, &QPushButton::clicked, this, [=]() {
		autoColorChangeEnabled = true;
		});

	connect(ui->pushButton_color_change_fix, &QPushButton::clicked, this, [=]() {
		autoColorChangeEnabled = false;
		});

	connect(ui->horizontalSlider_speed, &QSlider::valueChanged, ui->spinBox_speed, &QSpinBox::setValue);
	connect(ui->spinBox_speed, QOverload<int>::of(&QSpinBox::valueChanged), ui->horizontalSlider_speed, &QSlider::setValue);

	connect(ui->spinBox_speed, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int value) {
		// 控制 brightness 增长速度
		autoRunTimer->setInterval(value);
		});



connect(displayWindow, &DisplayWindow::visibilityChangedExternally,
        this, &MainWindow::handleDisplayVisibilityChanged);

connect(ui->checkBox_hide, &QCheckBox::stateChanged,
        this, &MainWindow::on_checkBox_hide_stateChanged);

	connect(ui->checkBox_lock, &QCheckBox::toggled, this, [=](bool locked) {
		if (!displayWindow) return;

		displayWindow->setLockGeometry(locked);

		// 禁用或启用 SpinBox 控件
		ui->spinBox_x_value->setEnabled(!locked);
		ui->spinBox_y_value->setEnabled(!locked);
		ui->spinBox_width->setEnabled(!locked);
		ui->spinBox_hight->setEnabled(!locked);
		});

	connect(ui->horizontalSlider_brightness, &QSlider::valueChanged,
		ui->spinBox_brightness, &QSpinBox::setValue);

	connect(ui->spinBox_x_value, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int value) {
		QRect geom = displayWindow->geometry();
		geom.moveLeft(value);
		displayWindow->setGeometry(geom);
		});
	connect(ui->spinBox_y_value, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int val) {
		QRect geom = displayWindow->geometry();
		geom.moveTop(val);
		displayWindow->setGeometry(geom);
		});

	connect(ui->spinBox_width, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int val) {
		QRect geom = displayWindow->geometry();
		geom.setWidth(val);
		displayWindow->setGeometry(geom);
		});

	connect(ui->checkBox_double_fullScreen, &QCheckBox::toggled, this, [=](bool checked) {
		if (displayWindow)
			displayWindow->setEnableDoubleClickFullScreen(checked);
		});

	connect(ui->spinBox_hight, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int val) {
		QRect geom = displayWindow->geometry();
		geom.setHeight(val);
		displayWindow->setGeometry(geom);
		});
	connect(ui->checkBox_windowTopMost, &QCheckBox::toggled, this, [=](bool checked) {
		if (displayWindow)
			displayWindow->setTopMost(checked);
		});

	connect(ui->spinBox_brightness, QOverload<int>::of(&QSpinBox::valueChanged),
		ui->horizontalSlider_brightness, &QSlider::setValue);
	connect(displayWindow, &DisplayWindow::visibilityChangedExternally,
		this, &MainWindow::handleDisplayVisibilityChanged);

	connect(ui->checkBox_hide, &QCheckBox::stateChanged,
		this, &MainWindow::on_checkBox_hide_stateChanged);

	connect(ui->radioButton_red, &QRadioButton::clicked, this, &MainWindow::on_radioButton_red_clicked);
	connect(ui->radioButton_blue, &QRadioButton::clicked, this, &MainWindow::on_radioButton_blue_clicked);
	connect(ui->radioButton_green, &QRadioButton::clicked, this, &MainWindow::on_radioButton_green_clicked);
	connect(ui->radioButton_cyan, &QRadioButton::clicked, this, &MainWindow::on_radioButton_cyan_clicked);
	connect(ui->radioButton_purple, &QRadioButton::clicked, this, &MainWindow::on_radioButton_purple_clicked);
	connect(ui->radioButton_yellow, &QRadioButton::clicked, this, &MainWindow::on_radioButton_yellow_clicked);
	connect(ui->radioButton_white, &QRadioButton::clicked, this, &MainWindow::on_radioButton_white_clicked);
	connect(ui->horizontalSlider_brightness, &QSlider::valueChanged, this, &MainWindow::onBrightnessChanged);
	connect(displayWindow, &DisplayWindow::geometryChanged, this, [=](const QRect& geom) {
		ui->spinBox_x_value->blockSignals(true);
		ui->spinBox_y_value->blockSignals(true);
		ui->spinBox_width->blockSignals(true);
		ui->spinBox_hight->blockSignals(true);

		ui->spinBox_x_value->setValue(geom.x());
		ui->spinBox_y_value->setValue(geom.y());
		ui->spinBox_width->setValue(geom.width());
		ui->spinBox_hight->setValue(geom.height());

		ui->spinBox_x_value->blockSignals(false);
		ui->spinBox_y_value->blockSignals(false);
		ui->spinBox_width->blockSignals(false);
		ui->spinBox_hight->blockSignals(false);
		});
	syncSpinBoxesWithDisplayWindow();
	onBrightnessChanged(ui->horizontalSlider_brightness->value());
}
// MainWindow.cpp 中实现
void MainWindow::on_checkBox_hide_stateChanged(int state) {
	bool shouldHide = (state == Qt::Checked);
	if (displayWindow) {
		displayWindow->setVisible(!shouldHide);
	}
}

void MainWindow::handleDisplayVisibilityChanged(bool isVisible) {
	// 避免信号循环：阻止勾选框再次触发
	ui->checkBox_hide->blockSignals(true);
	ui->checkBox_hide->setChecked(!isVisible);
	ui->checkBox_hide->blockSignals(false);
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
void MainWindow::onMainTimerTick()
{
	// 未来这里调用各功能模块的 tick()
	qDebug() << "定时器启动 Tick ON...";
}

void MainWindow::toggleRunState()
{
	isRunning = !isRunning;

	if (isRunning) {
		mainTimer->start();
		ui->pushButton_run_space->setText("暂停");
	}
	else {
		mainTimer->stop();
		ui->pushButton_run_space->setText("运行");
	}
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

void MainWindow::syncSpinBoxesWithDisplayWindow() {
	QRect geom = displayWindow->geometry();
	ui->spinBox_x_value->setValue(geom.x());
	ui->spinBox_y_value->setValue(geom.y());
	ui->spinBox_width->setValue(geom.width());
	ui->spinBox_hight->setValue(geom.height());
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


