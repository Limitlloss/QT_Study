#ifndef QNETWORK_H
#define QNETWORK_H

#include "qjson.h"
#include <QNetworkReply>
#include <QJsonDocument>
#include <QEventLoop>
#include <QTimerEvent>
#include <QCoreApplication>

extern const char* const FormBoundary;

// 获取全局网络访问管理器实例
QNetworkAccessManager* netAccess();

// 自定义网络请求类，继承自QNetworkRequest
class NetReq : public QNetworkRequest {
public:
#if(QT_VERSION_MAJOR > 5)
    using QNetworkRequest::QNetworkRequest;
    // 构造函数，设置请求的URL
    explicit NetReq(const QString& url) : QNetworkRequest{ url } {};
    // 构造函数，设置网络访问管理器
    explicit NetReq(QNetworkAccessManager* access) : mAccess(access) {};
#else
    NetReq() { init(); };
    // 构造函数，设置请求的URL
    explicit NetReq(const QString& url) : QNetworkRequest{ url } { init(); };
    // 构造函数，设置请求的URL
    explicit NetReq(const QUrl& url) : QNetworkRequest{ url } { init(); };
    // 构造函数，设置网络访问管理器
    explicit NetReq(QNetworkAccessManager* access) : mAccess(access) { init(); };
    // 初始化网络请求属性
    inline void init() {
        setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    }
#endif
    using QNetworkRequest::url;
    // 设置请求的URL
    inline NetReq& url(const QUrl& url) {
        setUrl(url);
        return *this;
    }
    // 设置网络访问管理器
    inline NetReq& access(QNetworkAccessManager* access) {
        mAccess = access;
        return *this;
    }
    // 设置自定义请求头
    inline NetReq& header(const QByteArray& headerName, const QByteArray& value) {
        setRawHeader(headerName, value);
        return *this;
    }
    // 设置已知类型的请求头
    inline NetReq& header(KnownHeaders header, const QVariant& value) {
        setHeader(header, value);
        return *this;
    }

    // 设置内容类型请求头
    inline NetReq& type(const QByteArray& value) {
        setRawHeader("Content-Type", value);
        return *this;
    }
    // 设置内容类型为application/json
    inline NetReq& typeJson() {
        setRawHeader("Content-Type", "application/json");
        return *this;
    }
    // 设置请求超时时间
    inline NetReq& timeout(int timeout) {
        setTransferTimeout(timeout);
        return *this;
    }

    // 发送GET请求
    inline QNetworkReply* get() {
        if (mAccess == 0) mAccess = netAccess();
        return mAccess->get(*this);
    }
    // 发送POST请求，数据为QByteArray
    inline QNetworkReply* post(const QByteArray& data) {
        if (mAccess == 0) mAccess = netAccess();
        return mAccess->post(*this, data);
    }
    // 发送POST请求，数据为char指针
    inline QNetworkReply* post(const char* data) {
        if (mAccess == 0) mAccess = netAccess();
        return mAccess->post(*this, data);
    }

    // 发送POST请求，数据为JValue对象
    inline QNetworkReply* post(const JValue& json) {
        setRawHeader("Content-Type", "application/json");
        return post(JToBytes(json));
    }
    // 发送POST请求，数据为QJsonDocument对象
    inline QNetworkReply* post(const QJsonDocument& json) {
        setRawHeader("Content-Type", "application/json");
        return post(json.toJson(QJsonDocument::Compact));
    }
    // 发送POST请求，数据为QJsonObject对象
    inline QNetworkReply* post(const QJsonObject& json) {
        return post(QJsonDocument{ json });
    }
    // 发送POST请求，数据为QJsonArray对象
    inline QNetworkReply* post(const QJsonArray& json) {
        return post(QJsonDocument{ json });
    }
    // 发送POST请求，数据为QHttpMultiPart对象
    inline QNetworkReply* post(QHttpMultiPart* multiPart) {
        if (mAccess == 0) mAccess = netAccess();
        return mAccess->post(*this, multiPart);
    }
    // 网络访问管理器指针
    QNetworkAccessManager* mAccess{ 0 };
};

// 获取网络回复的错误信息
QString errStr(QNetworkReply*);
// 获取网络回复的错误信息，并附加JSON数据
QString errStrWithData(QNetworkReply*, JValue* = 0);
// 获取网络回复的错误信息，并附加QJsonDocument数据
QString errStrWithData(QNetworkReply*, QJsonDocument*);

// 等待网络回复完成
inline int waitFinished(QNetworkReply* reply, QObject* context, bool excludeUser = false) {
    if (reply->isFinished()) return 0;
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    if (context) QObject::connect(context, &QObject::destroyed, &loop, [&] {
        reply->deleteLater();
        loop.exit(1);
        });
    return excludeUser ? loop.exec(QEventLoop::ExcludeUserInputEvents) : loop.exec();
}

// 连接网络回复的finished信号到context对象的槽函数
#define ConnReply(reply, context)\
    QObject::connect(context, &QObject::destroyed, reply, &QNetworkReply::deleteLater);\
    QObject::connect(reply, &QNetworkReply::finished, context,

// 根据错误码获取错误关键字
const char* socketErrKey(int value);

// 自定义TCP套接字类，继承自QTcpSocket
class TcpSocket : public QTcpSocket {
    Q_OBJECT
public:
    using QTcpSocket::QTcpSocket;
    // 析构函数，停止计时器
    ~TcpSocket() {
        if (timerId) killTimer(timerId);
    };

    // 中断连接
    void abort() {
        stopTimer();
        QTcpSocket::abort();
    }
    // 关闭套接字连接
    void close() override {
        stopTimer();
        QTcpSocket::close();
    }
    // 等待套接字连接到服务器
    bool waitForConnected(int msecs = 30000) override;
    // 等待套接字断开连接
    bool waitForDisconnected(int msecs = 30000) override;
    // 等待所有数据被写入套接字
    bool waitForBytesWritten(int msecs = 30000) override;
    // 等待套接字可读
    bool waitForReadyRead(int msecs = 30000) override;
    // 启动计时器
    void startTimer(int interval, Qt::TimerType timerType = Qt::CoarseTimer) {
        if (timerId) killTimer(timerId);
        timerId = QTcpSocket::startTimer(interval, timerType);
    }
    // 停止计时器
    void stopTimer() {
        if (timerId == 0) return;
        killTimer(timerId);
        timerId = 0;
    }
    // 计时器ID
    int timerId{ 0 };

protected:
    // 处理计时器事件
    void timerEvent(QTimerEvent* e) override {
        if (e->timerId() != timerId) QTcpSocket::timerEvent(e);
        else {
            abort();
            setSocketError(SocketTimeoutError);
            setErrorString(QCoreApplication::translate("QAbstractSocket", "Socket operation timed out"));
            emit errorOccurred(QAbstractSocket::SocketTimeoutError);
        }
    };
    // 连接并执行事件循环
    bool connAndExec(int msecs, QEventLoop* loop);
};

#endif // QNETWORK_H

