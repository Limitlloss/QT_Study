#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QByteArray>

class Protocol {
public:
    static const quint8 BOOTLOADER_CMD = 0x06;
    static const quint8 UPGRADE_DATA_CMD = 0x07;
    static const quint8 UPGRADE_DONE_CMD = 0x08;

    static const QByteArray FRAME_HEADER;
    static const QByteArray PROTOCOL_TYPE;

    static quint32 crc32(const QByteArray &data);
    static QByteArray buildPacket(quint8 cmd, const QByteArray &payload = QByteArray(), bool needAck = true);
};


#endif // PROTOCOL_H
