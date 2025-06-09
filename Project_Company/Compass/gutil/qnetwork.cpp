#include "qnetwork.h"

#include <QCoreApplication>
#include <QMetaEnum>

// 定义表单边界字符串
const char* const FormBoundary = "----GangphonQtBoundary_.oOo._";

// 返回静态的 QNetworkAccessManager 实例
QNetworkAccessManager* netAccess() {
    static QNetworkAccessManager access;
    return &access;
};

// 根据 QNetworkReply 对象生成错误字符串
QString errStr(QNetworkReply* reply) {
    reply->deleteLater();
    auto error = reply->error();
    if (error != QNetworkReply::NoError) {
        if (error == QNetworkReply::OperationCanceledError) return "TimeoutError (" + QString::number(QNetworkReply::TimeoutError) + ") " + QCoreApplication::translate("Net", "Connection Timeout");
        auto errStr = reply->errorString();
        if (error != QNetworkReply::InternalServerError || !errStr.endsWith("replied: Unknown")) return QString(QMetaEnum::fromType<QNetworkReply::NetworkError>().valueToKey(error)) + " (" + QString::number(error) + ") " + errStr;
    }
    auto status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status != 200) return QString::number(status) + " " + reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    return "";
}

// 根据 QNetworkReply 对象生成错误字符串，并尝试解析 JSON 数据
QString errStrWithData(QNetworkReply* reply, JValue* outJson) {
    auto err = errStr(reply);
    if (!err.isEmpty()) {
        auto data = reply->readAll();
        if (!data.isEmpty()) err += "\n" + data;
        return err;
    }
    if (outJson) {
        auto data = reply->readAll();
        QString error;
        *outJson = JFrom(data, &error);
        if (!error.isEmpty()) return "JSON " + QCoreApplication::translate("Net", "Error") + ": " + error + "\n" + data;
    }
    return "";
}

// 根据 QNetworkReply 对象生成错误字符串，并尝试解析 QJsonDocument 数据
QString errStrWithData(QNetworkReply* reply, QJsonDocument* outJson) {
    auto err = errStr(reply);
    if (!err.isEmpty()) {
        auto data = reply->readAll();
        if (!data.isEmpty()) err += "\n" + data;
        return err;
    }
    if (outJson) {
        auto data = reply->readAll();
        QJsonParseError jsonErr;
        *outJson = QJsonDocument::fromJson(data, &jsonErr);
        if (jsonErr.error != QJsonParseError::NoError) return "JSON " + QCoreApplication::translate("Net", "Error") + ": " + jsonErr.errorString() + "\n" + data;
    }
    return "";
}

// 根据 QAbstractSocket::SocketError 的枚举值返回对应的键字符串
const char* socketErrKey(int value) {
    static auto metaEnum = QMetaEnum::fromType<QAbstractSocket::SocketError>();
    return metaEnum.valueToKey(value);
}

// 等待 TCP socket 连接到服务器
bool TcpSocket::waitForConnected(int msecs) {
    if (state() == ConnectedState) return true;
    QEventLoop loop;
    connect(this, &QTcpSocket::connected, &loop, &QEventLoop::quit);
    return connAndExec(msecs, &loop);
}

// 等待 TCP socket 断开连接
bool TcpSocket::waitForDisconnected(int msecs) {
    if (state() == UnconnectedState) return true;
    QEventLoop loop;
    connect(this, &QTcpSocket::disconnected, &loop, &QEventLoop::quit);
    return connAndExec(msecs, &loop);
}

// 等待所有字节写入完成
bool TcpSocket::waitForBytesWritten(int msecs) {
    if (bytesToWrite() == 0) return true;
    QEventLoop loop;
    connect(this, &QTcpSocket::bytesWritten, &loop, &QEventLoop::quit);
    return connAndExec(msecs, &loop);
}

// 等待有可读取的数据
bool TcpSocket::waitForReadyRead(int msecs) {
    if (bytesAvailable()) return true;
    QEventLoop loop;
    connect(this, &QTcpSocket::readyRead, &loop, &QEventLoop::quit);
    return connAndExec(msecs, &loop);
}

// 辅助函数，用于启动事件循环并处理定时器
bool TcpSocket::connAndExec(int msecs, QEventLoop* loop) {
    connect(this, &QTcpSocket::errorOccurred, loop, [loop] {
        loop->exit(1);
        });
    if (timerId) {
        killTimer(timerId);
        timerId = 0;
    }
    if (msecs > 0) startTimer(msecs);
    auto res = loop->exec();
    stopTimer();
    return res == 0;
}
