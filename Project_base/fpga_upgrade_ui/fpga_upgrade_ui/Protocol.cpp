#include "Protocol.h"

// 初始化静态成员变量
const QByteArray Protocol::FRAME_HEADER = QByteArray::fromHex("7E7E55");
const QByteArray Protocol::PROTOCOL_TYPE = QByteArray::fromHex("01");

// 正确补充成员函数定义
quint32 Protocol::crc32(const QByteArray& data)
{
    quint32 crc = 0xFFFFFFFF;
    for (auto b : data)
    {
        crc ^= static_cast<quint8>(b);
        for (int i = 0; i < 8; ++i)
            crc = (crc >> 1) ^ (-(crc & 1) & 0xEDB88320);
    }
    return ~crc;
}

QByteArray Protocol::buildPacket(quint8 cmd, const QByteArray &payload, bool needAck) {
    QByteArray frame = FRAME_HEADER + PROTOCOL_TYPE;
    frame.append(cmd);
    frame.append(needAck ? '\x01' : '\x00');

    quint16 length = payload.size();
    frame.append(static_cast<char>(length & 0xFF));
    frame.append(static_cast<char>((length >> 8) & 0xFF));
    frame.append(payload);

    quint32 crc = crc32(frame);
    frame.append(static_cast<char>(crc & 0xFF));
    frame.append(static_cast<char>((crc >> 8) & 0xFF));
    frame.append(static_cast<char>((crc >> 16) & 0xFF));
    frame.append(static_cast<char>((crc >> 24) & 0xFF));

    return frame;
}
