#include "UpgradeWorker.h"
#include <QFile>
#include <QThread>

UpgradeWorker::UpgradeWorker(const QString &binPath, const QString &portName, QObject *parent)
    : QObject(parent), binPath(binPath), portName(portName) {}

void UpgradeWorker::startUpgrade() {
    QSerialPort serial;
    serial.setPortName(portName);
    serial.setBaudRate(2000000);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);

    if (!serial.open(QIODevice::ReadWrite)) {
        log("[ERROR] 打开串口失败");
        emit finished(false);
        return;
    }

    log("[INFO] 进入 Bootloader ...");
    if (!sendPacket(serial, Protocol::BOOTLOADER_CMD, QByteArray(1, '\x01'), true))
        log("[WARN] Bootloader 未确认");

    QThread::sleep(1);

    QFile file(binPath);
    if (!file.open(QIODevice::ReadOnly)) {
        log("[ERROR] 打开BIN文件失败");
        emit finished(false);
        return;
    }

    QByteArray fileData = file.readAll();
    file.close();

    int totalPackets = (fileData.size() + 2048 - 1) / 2048;
    log(QString("[INFO] 发送 %1 包").arg(totalPackets));

    for (int i = 0; i < totalPackets; ++i) {
        QByteArray chunk = fileData.mid(i * 2048, 2048);
        QByteArray payload;
        payload.append(static_cast<char>((i+1) & 0xFF));
        payload.append(static_cast<char>(((i+1) >> 8) & 0xFF));
        payload.append(chunk);
        bool expectAck = ((i+1) % 10 == 0) || ((i+1) == totalPackets);
        if (!sendPacket(serial, Protocol::UPGRADE_DATA_CMD, payload, expectAck)) {
            log(QString("[ERROR] 第 %1 包发送失败").arg(i+1));
            emit finished(false);
            return;
        }
        log(QString("[%1/%2] 发送成功").arg(i+1).arg(totalPackets));
    }

    QByteArray donePayload;
    donePayload.append(static_cast<char>(totalPackets & 0xFF));
    donePayload.append(static_cast<char>((totalPackets >> 8) & 0xFF));

    if (sendPacket(serial, Protocol::UPGRADE_DONE_CMD, donePayload, true))
        emit finished(true);
    else
        emit finished(false);
}

bool UpgradeWorker::sendPacket(QSerialPort &serial, quint8 cmd, const QByteArray &payload, bool expectAck) {
    QByteArray pkt = Protocol::buildPacket(cmd, payload, expectAck);
    serial.write(pkt);
    return serial.waitForBytesWritten(1000);
}
