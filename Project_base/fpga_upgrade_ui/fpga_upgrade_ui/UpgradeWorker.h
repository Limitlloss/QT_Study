#ifndef UPGRADEWORKER_H
#define UPGRADEWORKER_H

#include <QObject>
#include <QSerialPort>
#include "Protocol.h"

class UpgradeWorker : public QObject {
    Q_OBJECT

public:
    UpgradeWorker(const QString &binPath, const QString &portName, QObject *parent = nullptr);
public slots:
    void startUpgrade();

signals:
    void log(const QString &);
    void finished(bool success);

private:
    QString binPath;
    QString portName;
    bool sendPacket(QSerialPort &serial, quint8 cmd, const QByteArray &payload, bool expectAck);
};

#endif // UPGRADEWORKER_H
