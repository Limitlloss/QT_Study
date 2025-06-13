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
		playlistTimer = new QTimer(this);
		connect(playlistTimer, &QTimer::timeout, this, &MainWindow::playNextStep);
		buildProgramPlaylist();
		playlistTimer->start(ui->spinBox_interval->value());
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
	connect(ui->spinBox_interval, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int ms) {
		playlistTimer->setInterval(ms);
		});
	connect(ui->checkBox_aging_red, &QCheckBox::toggled, this, [=](bool) { buildProgramPlaylist(); });
	connect(ui->checkBox_aging_green, &QCheckBox::toggled, this, [=](bool) { buildProgramPlaylist(); });
	connect(ui->checkBox_aging_blue, &QCheckBox::toggled, this, [=](bool) { buildProgramPlaylist(); });
	connect(ui->checkBox_yellow, &QCheckBox::toggled, this, [=](bool) { buildProgramPlaylist(); });
	connect(ui->checkBox_cyan, &QCheckBox::toggled, this, [=](bool) { buildProgramPlaylist(); });
	connect(ui->checkBox_purple, &QCheckBox::toggled, this, [=](bool) { buildProgramPlaylist(); });
	connect(ui->checkBox_white, &QCheckBox::toggled, this, [=](bool) { buildProgramPlaylist(); });
	//connect(ui->checkBox_black, &QCheckBox::toggled, this, [=](bool) { buildProgramPlaylist(); });
	connect(ui->checkBox_HorizontalLine, &QCheckBox::toggled, this, [=](bool) { buildProgramPlaylist(); });
	connect(ui->checkBox_VerticalLine, &QCheckBox::toggled, this, [=](bool) { buildProgramPlaylist(); });
	connect(ui->checkBox_DiagonalLeftLine, &QCheckBox::toggled, this, [=](bool) { buildProgramPlaylist(); });
	connect(ui->checkBox_DiagonalRightLine, &QCheckBox::toggled, this, [=](bool) { buildProgramPlaylist(); });

}

MainWindow::~MainWindow() {
	delete ui;
}
void MainWindow::buildProgramPlaylist() {
	playlist.clear();

	// 加入颜色项
	if (ui->checkBox_aging_red->isChecked())     playlist.append(ProgramStep(Qt::red));
	if (ui->checkBox_aging_green->isChecked())   playlist.append(ProgramStep(Qt::green));
	if (ui->checkBox_aging_blue->isChecked())    playlist.append(ProgramStep(Qt::blue));
	if (ui->checkBox_yellow->isChecked())        playlist.append(ProgramStep(Qt::yellow));
	if (ui->checkBox_cyan->isChecked())          playlist.append(ProgramStep(Qt::cyan));
	if (ui->checkBox_purple->isChecked())        playlist.append(ProgramStep(Qt::magenta));
	if (ui->checkBox_white->isChecked())         playlist.append(ProgramStep(Qt::white));

	// 加入线条组合项
	if (ui->checkBox_HorizontalLine->isChecked())
		playlist.append(ProgramStep(true, false, false, false));
	if (ui->checkBox_VerticalLine->isChecked())
		playlist.append(ProgramStep(false, true, false, false));
	if (ui->checkBox_DiagonalLeftLine->isChecked())
		playlist.append(ProgramStep(false, false, true, false));
	if (ui->checkBox_DiagonalRightLine->isChecked())
		playlist.append(ProgramStep(false, false, false, true));

	playlistIndex = 0;
}

void MainWindow::playNextStep() {
	if (playlist.isEmpty()) return;

	const ProgramStep& step = playlist[playlistIndex % playlist.size()];

	if (step.type == ProgramStep::COLOR) {
		displayWindow->setUseCurrentColor(false);
		displayWindow->setBackgroundColor(step.color);
		displayWindow->setLinePattern(false, false, false, false);
		displayWindow->setLineRendererEnabled(false);
		displayWindow->enableAutoScan(false);  // ✅ 禁用滚动
	}
	else {
		displayWindow->setUseCurrentColor(false);
		displayWindow->setBackgroundColor(Qt::black);
		displayWindow->setLinePattern(
			step.linePattern[0],
			step.linePattern[1],
			step.linePattern[2],
			step.linePattern[3]);
		displayWindow->setLineRendererEnabled(true);
		displayWindow->enableAutoScan(true);   // ✅ 启用滚动
	}

	displayWindow->update();
	playlistIndex++;
}

void MainWindow::onTabSwitched(int index) {
	// 关闭所有 Feature
	for (auto* f : featureModules)
		f->setEnabled(false);

	displayWindow->setUseCurrentColor(false);
	displayWindow->setLineRendererEnabled(false);
	ledManager->stop();

	// 停止老化播放列表
	playlistTimer->stop();   // ✅ 必须添加这一句！

	switch (index) {
	case 0: // 固定颜色
		displayWindow->setUseCurrentColor(false);
		updateDisplayColor();
		break;
	case 1: // 渐变
		ledManager->start();
		break;
	case 2: // 老化测试
		emit ui->checkBox_HorizontalLine->toggled(ui->checkBox_HorizontalLine->isChecked());
		emit ui->checkBox_VerticalLine->toggled(ui->checkBox_VerticalLine->isChecked());
		emit ui->checkBox_DiagonalLeftLine->toggled(ui->checkBox_DiagonalLeftLine->isChecked());
		emit ui->checkBox_DiagonalRightLine->toggled(ui->checkBox_DiagonalRightLine->isChecked());

		emit ui->checkBox_aging_red->toggled(ui->checkBox_aging_red->isChecked());
		emit ui->checkBox_aging_green->toggled(ui->checkBox_aging_green->isChecked());
		emit ui->checkBox_aging_blue->toggled(ui->checkBox_aging_blue->isChecked());
		emit ui->checkBox_yellow->toggled(ui->checkBox_yellow->isChecked());
		emit ui->checkBox_cyan->toggled(ui->checkBox_cyan->isChecked());
		emit ui->checkBox_purple->toggled(ui->checkBox_purple->isChecked());
		emit ui->checkBox_white->toggled(ui->checkBox_white->isChecked());
		buildProgramPlaylist();
		currentProgramIndex = 0;
		programPlaylist = playlist;  // 添加这一行以更新播放列表
		if (!playlistTimer->isActive())
			playlistTimer->start(ui->spinBox_interval->value());  // 用你的时间间隔控件

		// 立刻手动执行一次显示更新（否则等下一轮才显示）
		if (!programPlaylist.isEmpty()) {
			const ProgramStep& step = programPlaylist[currentProgramIndex];

			if (step.type == ProgramStep::COLOR) {
				displayWindow->setUseCurrentColor(false);
				displayWindow->setBackgroundColor(step.color);
				displayWindow->setLineRendererEnabled(false);
			}
			else {
				displayWindow->setUseCurrentColor(false);
				displayWindow->setBackgroundColor(Qt::black);
				displayWindow->setLinePattern(
					step.linePattern[0],
					step.linePattern[1],
					step.linePattern[2],
					step.linePattern[3]
				);
				displayWindow->setLineRendererEnabled(true);
			}

			displayWindow->update();
		}
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
	connect(ui->spinBox_spin, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int val) {
		displayWindow->setLineRendererSpacing(val);
		});

	connect(ui->spinBox_interval, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int val) {
		ui->horizontalSlider_aging_speed->blockSignals(true);
		ui->horizontalSlider_aging_speed->setValue(val);
		ui->horizontalSlider_aging_speed->blockSignals(false);

		playlistTimer->setInterval(val);  // 更新轮播间隔
		});

	connect(ui->horizontalSlider_aging_speed, &QSlider::valueChanged, this, [=](int val) {
		ui->spinBox_interval->blockSignals(true);
		ui->spinBox_interval->setValue(val);
		ui->spinBox_interval->blockSignals(false);

		playlistTimer->setInterval(val);  // 关键：补上这一句
		});

	connect(ui->spinBox_interval, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int val) {
		ui->horizontalSlider_aging_speed->blockSignals(true);
		ui->horizontalSlider_aging_speed->setValue(val);
		ui->horizontalSlider_aging_speed->blockSignals(false);
		});

	connect(ui->horizontalSlider_aging_speed, &QSlider::valueChanged, this, [=](int val) {
		ui->spinBox_interval->blockSignals(true);
		ui->spinBox_interval->setValue(val);
		ui->spinBox_interval->blockSignals(false);
		});

	connect(ui->pushButton_run_space, &QPushButton::clicked, this, &MainWindow::toggleRunState);

	connect(ui->checkBox_HorizontalLine, &QCheckBox::toggled, this, [=](bool) {
		buildProgramPlaylist();
		});

	connect(ui->checkBox_VerticalLine, &QCheckBox::toggled, this, [=](bool) {
		buildProgramPlaylist();
		});

	connect(ui->checkBox_DiagonalLeftLine, &QCheckBox::toggled, this, [=](bool) {
		buildProgramPlaylist();
		});

	connect(ui->checkBox_DiagonalRightLine, &QCheckBox::toggled, this, [=](bool) {
		buildProgramPlaylist();
		});

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
	setAllFeatureRunning(!isRunning);
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

void MainWindow::setAllFeatureRunning(bool enable) {
	isRunning = enable;

	if (enable) {
		mainTimer->start();
		if (ui->tabWidget->currentIndex() == 2)
			playlistTimer->start(ui->spinBox_interval->value());
	}
	else {
		mainTimer->stop();
		playlistTimer->stop();
	}

	ui->pushButton_run_space->setText(enable ? "暂停" : "运行");
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
