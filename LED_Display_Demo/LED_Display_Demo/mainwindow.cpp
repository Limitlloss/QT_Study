#include "MainWindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    displayWindow = new DisplayWindow();
    ledManager = new LedEffectManager(this);

    displayWindow->move(this->pos().x() + this->width() + 20, this->pos().y());
    displayWindow->show();

    displayWindow->setEnableDoubleClickFullScreen(ui->checkBox_double_fullScreen->isChecked());
    displayWindow->setTopMost(ui->checkBox_windowTopMost->isChecked());
    displayWindow->setLockGeometry(ui->checkBox_lock->isChecked());

    connect(displayWindow, &DisplayWindow::visibilityChangedExternally,
        this, &MainWindow::handleDisplayVisibilityChanged);

    mainTimer = new QTimer(this);
    mainTimer->setInterval(100);
    connect(mainTimer, &QTimer::timeout, this, &MainWindow::onMainTimerTick);

    autoRunTimer = new QTimer(this);
    autoRunTimer->setInterval(50);
    connect(autoRunTimer, &QTimer::timeout, this, [=]() {
        if (!autoColorChangeEnabled) return;
        currentBrightness = (currentBrightness + 1) % 256;
        ui->horizontalSlider_brightness->setValue(currentBrightness);
        });

    connect(ledManager, &LedEffectManager::colorChanged, displayWindow, &DisplayWindow::setBackgroundColor);

    setupUiLogic();
    setupColorButtons();
    syncSpinBoxesWithDisplayWindow();
    onBrightnessChanged(ui->horizontalSlider_brightness->value());
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupUiLogic() {
    connect(ui->pushButton_run_space, &QPushButton::clicked, this, &MainWindow::toggleRunState);

    connect(ui->pushButton_color_change_auto, &QPushButton::clicked, this, [=]() {
        autoColorChangeEnabled = true;
        autoRunTimer->start();
        });

    connect(ui->pushButton_color_change_fix, &QPushButton::clicked, this, [=]() {
        autoColorChangeEnabled = false;
        autoRunTimer->stop();
        });

    connect(ui->checkBox_hide, &QCheckBox::stateChanged, this, &MainWindow::on_checkBox_hide_stateChanged);

    connect(ui->checkBox_lock, &QCheckBox::toggled, this, [=](bool locked) {
        displayWindow->setLockGeometry(locked);
        ui->spinBox_x_value->setEnabled(!locked);
        ui->spinBox_y_value->setEnabled(!locked);
        ui->spinBox_width->setEnabled(!locked);
        ui->spinBox_hight->setEnabled(!locked);
        });

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
