#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "AgingFeature.h"

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	{
		displayWindow = new DisplayWindow(this);
		ledManager = new LedEffectManager(this);
		displayWindow->setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
		displayWindow->show();

		displayWindow->setEnableDoubleClickFullScreen(ui->checkBox_double_fullScreen->isChecked());
		displayWindow->setTopMost(ui->checkBox_windowTopMost->isChecked());
		displayWindow->setLockGeometry(ui->checkBox_lock->isChecked());
		displayWindow->setUseCurrentColor(true);
		displayWindow->setLineRendererEnabled(true);
		displayWindow->setLineRendererSpacing(35);
		ui->horizontalSlider_speed->setValue(50);     // 初始速度为50ms
		ui->spinBox_speed->setValue(50);
		ui->horizontalSlider_speed->setRange(10, 500); // 间隔1~500ms
		ui->spinBox_speed->setRange(10, 500);
		mainTimer = new QTimer(this);
		mainTimer->setInterval(10);

		agingFeature = new AgingFeature(displayWindow, this);
		connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabSwitched);

		// 示例：启用 aging 模块
		//connect(ui->checkBox_agingEnable, &QCheckBox::toggled, this, [=](bool enabled) {
		//	agingFeature->setEnabled(enabled);
		//	});

		autoBrightnessFeature = new AutoBrightnessFeature(
			ui->horizontalSlider_brightness,
			ui->spinBox_brightness,
			this
		);
		featureModules.push_back(autoBrightnessFeature);
		featureModules.push_back(agingFeature);
		//featureModules.push_back(lineRenderer);
	}

	connect(displayWindow, &DisplayWindow::visibilityChangedExternally,
		this, &MainWindow::handleDisplayVisibilityChanged);

	connect(mainTimer, &QTimer::timeout, this, &MainWindow::onMainTimerTick);

    connect(ui->checkBox_HorizontalLine, &QCheckBox::toggled, this, [=](bool checked){
        displayWindow->setLinePattern(checked,
                                      ui->checkBox_VerticalLine->isChecked(),
                                      ui->checkBox_DiagonalLeftLine->isChecked(),
                                      ui->checkBox_DiagonalRightLine->isChecked());
    });

	connect(ledManager, &LedEffectManager::colorChanged, displayWindow, &DisplayWindow::setBackgroundColor);

	setupUiLogic();
	setupColorButtons();

	syncSpinBoxesWithDisplayWindow();
	onBrightnessChanged(ui->horizontalSlider_brightness->value());
	connect(ui->pushButton_color_change_auto, &QPushButton::clicked, this, [=]() {
		autoBrightnessFeature->setEnabled(true);
		ui->pushButton_color_change_fix->setChecked(false);
		ui->pushButton_color_change_auto->setChecked(true);
		});

	connect(ui->pushButton_color_change_fix, &QPushButton::clicked, this, [=]() {
		autoBrightnessFeature->setEnabled(false);
		ui->pushButton_color_change_fix->setChecked(true);
		ui->pushButton_color_change_auto->setChecked(false);
		});

}

MainWindow::~MainWindow() {
	delete ui;
}


void MainWindow::onTabSwitched(int index) {
	// 通用关闭所有 Feature 模块
	for (auto* f : featureModules) {
		f->setEnabled(false);
	}

	// 清除状态
	displayWindow->setUseCurrentColor(false);
	displayWindow->setLineRendererEnabled(false);
	ledManager->stop();  // 渐变模块

	// 根据当前 tab 激活功能
	switch (index) {
	case 0: // 颜色固定
		displayWindow->setUseCurrentColor(false); // 使用 bgColor
		updateDisplayColor();
		break;

	case 1: // 渐变（LedEffectManager）
		ledManager->start();
		break;

	case 2: // 老化（AgingFeature）
		if (agingFeature)
			agingFeature->setEnabled(true);
		break;

	default:
		break;
	}

	displayWindow->update();
}


void MainWindow::showEvent(QShowEvent* event) {
	QMainWindow::showEvent(event);
	if (displayWindow) {
		displayWindow->move(this->geometry().topRight() + QPoint(20, 0));
	}
}

void MainWindow::setupUiLogic() {
	connect(ui->pushButton_run_space, &QPushButton::clicked, this, &MainWindow::toggleRunState);


	connect(ui->horizontalSlider_speed, &QSlider::valueChanged, this, [=](int val) {
		if (autoBrightnessFeature)
			autoBrightnessFeature->setSpeedInterval(val);
		});
	connect(ui->spinBox_speed, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int val) {
		if (autoBrightnessFeature)
			autoBrightnessFeature->setSpeedInterval(val);
		});


	connect(ui->checkBox_hide, &QCheckBox::stateChanged, this, &MainWindow::on_checkBox_hide_stateChanged);

	connect(ui->checkBox_lock, &QCheckBox::toggled, this, [=](bool locked) {
		displayWindow->setLockGeometry(locked);
		ui->spinBox_x_value->setEnabled(!locked);
		ui->spinBox_y_value->setEnabled(!locked);
		ui->spinBox_width->setEnabled(!locked);
		ui->spinBox_hight->setEnabled(!locked);
		});

	// 在 setupUiLogic 或构造函数中添加：
	connect(ui->horizontalSlider_speed, &QSlider::valueChanged,
		ui->spinBox_speed, &QSpinBox::setValue);

	connect(ui->spinBox_speed, QOverload<int>::of(&QSpinBox::valueChanged),
		ui->horizontalSlider_speed, &QSlider::setValue);

	connect(ui->checkBox_double_fullScreen, &QCheckBox::toggled, displayWindow, &DisplayWindow::setEnableDoubleClickFullScreen);
	connect(ui->checkBox_windowTopMost, &QCheckBox::toggled, displayWindow, &DisplayWindow::setTopMost);

	connect(ui->horizontalSlider_brightness, &QSlider::valueChanged, this, &MainWindow::onBrightnessChanged);
	connect(ui->horizontalSlider_brightness, &QSlider::valueChanged, ui->spinBox_brightness, &QSpinBox::setValue);
	connect(ui->spinBox_brightness, QOverload<int>::of(&QSpinBox::valueChanged), ui->horizontalSlider_brightness, &QSlider::setValue);

	connect(ui->spinBox_x_value, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int val) {
		QRect geom = displayWindow->geometry();
		geom.moveLeft(val);
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

	connect(ui->spinBox_hight, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int val) {
		QRect geom = displayWindow->geometry();
		geom.setHeight(val);
		displayWindow->setGeometry(geom);
		});

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
}

void MainWindow::setupColorButtons() {
	connect(ui->radioButton_red, &QRadioButton::clicked, this, &MainWindow::on_radioButton_red_clicked);
	connect(ui->radioButton_blue, &QRadioButton::clicked, this, &MainWindow::on_radioButton_blue_clicked);
	connect(ui->radioButton_green, &QRadioButton::clicked, this, &MainWindow::on_radioButton_green_clicked);
	connect(ui->radioButton_yellow, &QRadioButton::clicked, this, &MainWindow::on_radioButton_yellow_clicked);
	connect(ui->radioButton_cyan, &QRadioButton::clicked, this, &MainWindow::on_radioButton_cyan_clicked);
	connect(ui->radioButton_purple, &QRadioButton::clicked, this, &MainWindow::on_radioButton_purple_clicked);
	connect(ui->radioButton_white, &QRadioButton::clicked, this, &MainWindow::on_radioButton_white_clicked);
}

void MainWindow::on_radioButton_red_clicked() {
	baseColor = Qt::red;
	updateDisplayColor();
}
void MainWindow::on_radioButton_blue_clicked() {
	baseColor = Qt::blue;
	updateDisplayColor();
}
void MainWindow::on_radioButton_green_clicked() {
	baseColor = Qt::green;
	updateDisplayColor();
}
void MainWindow::on_radioButton_yellow_clicked() {
	baseColor = Qt::yellow;
	updateDisplayColor();
}
void MainWindow::on_radioButton_cyan_clicked() {
	baseColor = Qt::cyan;
	updateDisplayColor();
}
void MainWindow::on_radioButton_purple_clicked() {
	baseColor = Qt::magenta;
	updateDisplayColor();
}
void MainWindow::on_radioButton_white_clicked() {
	baseColor = Qt::white;
	updateDisplayColor();
}

void MainWindow::updateDisplayColor() {
	int r = baseColor.red() == 255 ? brightnessValue : 0;
	int g = baseColor.green() == 255 ? brightnessValue : 0;
	int b = baseColor.blue() == 255 ? brightnessValue : 0;
	displayWindow->setBackgroundColor(QColor(r, g, b));
}

void MainWindow::onBrightnessChanged(int value) {
	brightnessValue = value;
	updateDisplayColor();
}

void MainWindow::toggleRunState() {
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

void MainWindow::onMainTimerTick() {
	for (auto* feature : featureModules) {
		if (feature->isEnabled())
			feature->tick();
	}
	qDebug() << "Tick ON...";
}

void MainWindow::on_checkBox_hide_stateChanged(int state) {
	if (displayWindow) {
		displayWindow->setVisible(state != Qt::Checked);
	}
}

void MainWindow::handleDisplayVisibilityChanged(bool isVisible) {
	ui->checkBox_hide->blockSignals(true);
	ui->checkBox_hide->setChecked(!isVisible);
	ui->checkBox_hide->blockSignals(false);
}

void MainWindow::syncSpinBoxesWithDisplayWindow() {
	QRect geom = displayWindow->geometry();
	ui->spinBox_x_value->setValue(geom.x());
	ui->spinBox_y_value->setValue(geom.y());
	ui->spinBox_width->setValue(geom.width());
	ui->spinBox_hight->setValue(geom.height());
}
