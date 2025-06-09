#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Protocol.h"

#include <QFileDialog>
#include <QSerialPortInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->btnSelectFile, &QPushButton::clicked, this, &MainWindow::selectFile);
    connect(ui->btnStart, &QPushButton::clicked, this, &MainWindow::startUpgrade);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::selectFile() {
    binFilePath = QFileDialog::getOpenFileName(this, "选择升级文件", "", "BIN Files (*.bin)");
    if (!binFilePath.isEmpty()) {
        ui->labelFile->setText(binFilePath);
    }
}

void MainWindow::startUpgrade() {
    if (binFilePath.isEmpty()) {
        appendLog("[ERROR] 请选择升级文件");
        return;
    }

    QString portName;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        if (info.vendorIdentifier() == 0x28e9 && info.productIdentifier() == 0x018a) {
            portName = info.portName();
            break;
        }
    }

    if (portName.isEmpty()) {
        appendLog("[ERROR] 未找到串口设备");
        return;
    }

    appendLog("[INFO] 发现串口: " + portName);

    worker = new UpgradeWorker(binFilePath, portName);
    worker->moveToThread(&workerThread);

    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &UpgradeWorker::log, this, &MainWindow::appendLog);
    connect(worker, &UpgradeWorker::finished, this, &MainWindow::upgradeFinished);
    connect(this, &MainWindow::destroyed, &workerThread, &QThread::quit);
    connect(&workerThread, &QThread::started, worker, &UpgradeWorker::startUpgrade);

    workerThread.start();
}

void MainWindow::appendLog(const QString &text) {
    ui->textLog->append(text);
}

void MainWindow::upgradeFinished(bool success) {
    workerThread.quit();
    workerThread.wait();
    if (success)
        appendLog("[SUCCESS] 升级成功");
    else
        appendLog("[FAIL] 升级失败");
}
