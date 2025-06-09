#ifndef QNETWORK_H
#define QNETWORK_H

#include "qjson.h"
#include <QNetworkReply>
#include <QJsonDocument>
#include <QEventLoop>
#include <QTimerEvent>
#include <QCoreApplication>

extern const char *const FormBoundary;

QNetworkAccessManager *netAccess();

class NetReq : public QNetworkRequest {
public:
#if(QT_VERSION_MAJOR > 5)
    using QNetworkRequest::QNetworkRequest;
    explicit NetReq(const QString &url) : QNetworkRequest{url} {};
    explicit NetReq(QNetworkAccessManager *access) : mAccess(access) {};
#else
    NetReq() {init();};
    explicit NetReq(const QString &url) : QNetworkRequest{url} {init();};
    explicit NetReq(const QUrl &url) : QNetworkRequest{url} {init();};
    explicit NetReq(QNetworkAccessManager *access) : mAccess(access) {init();};
    inline void init() {
        setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    }
#endif
    using QNetworkRequest::url;
    inline NetReq &url(const QUrl &url) {
        setUrl(url);
        return *this;
    }
    inline NetReq &access(QNetworkAccessManager *access) {
        mAccess = access;
        return *this;
    }
    inline NetReq &header(const QByteArray &headerName, const QByteArray &value) {
        setRawHeader(headerName, value);
        return *this;
    }
    inline NetReq &header(KnownHeaders header, const QVariant &value) {
        setHeader(header, value);
        return *this;
    }

    inline NetReq &type(const QByteArray &value) {
        setRawHeader("Content-Type", value);
        return *this;
    }
    inline NetReq &typeJson() {
        setRawHeader("Content-Type", "application/json");
        return *this;
    }
    inline NetReq &timeout(int timeout) {
        setTransferTimeout(timeout);
        return *this;
    }

    inline QNetworkReply *get() {
        if(mAccess==0) mAccess = netAccess();
        return mAccess->get(*this);
    }
    inline QNetworkReply *post(const QByteArray &data) {
        if(mAccess==0) mAccess = netAccess();
        return mAccess->post(*this, data);
    }
    inline QNetworkReply *post(const char *data) {
        if(mAccess==0) mAccess = netAccess();
        return mAccess->post(*this, data);
    }

    inline QNetworkReply *post(const JValue &json) {
        setRawHeader("Content-Type", "application/json");
        return post(JToBytes(json));
    }
    inline QNetworkReply *post(const QJsonDocument &json) {
        setRawHeader("Content-Type", "application/json");
        return post(json.toJson(QJsonDocument::Compact));
    }
    inline QNetworkReply *post(const QJsonObject &json) {
        return post(QJsonDocument{json});
    }
    inline QNetworkReply *post(const QJsonArray &json) {
        return post(QJsonDocument{json});
    }
    inline QNetworkReply *post(QHttpMultiPart *multiPart) {
        if(mAccess==0) mAccess = netAccess();
        return mAccess->post(*this, multiPart);
    }
    QNetworkAccessManager *mAccess{0};
};

QString errStr(QNetworkReply *);
QString errStrWithData(QNetworkReply *, JValue * = 0);
QString errStrWithData(QNetworkReply *, QJsonDocument *);

inline int waitFinished(QNetworkReply *reply, QObject *context, bool excludeUser = false) {
    if(reply->isFinished()) return 0;
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    if(context) QObject::connect(context, &QObject::destroyed, &loop, [&] {
        reply->deleteLater();
        loop.exit(1);
    });
    return excludeUser ? loop.exec(QEventLoop::ExcludeUserInputEvents) : loop.exec();
}

#define ConnReply(reply, context)\
    QObject::connect(context, &QObject::destroyed, reply, &QNetworkReply::deleteLater);\
    QObject::connect(reply, &QNetworkReply::finished, context,

const char* socketErrKey(int value);

class TcpSocket : public QTcpSocket {
    Q_OBJECT
public:
    using QTcpSocket::QTcpSocket;
    ~TcpSocket() {
        if(timerId) killTimer(timerId);
    };

    void abort() {
        stopTimer();
        QTcpSocket::abort();
    }
    void close() override {
        stopTimer();
        QTcpSocket::close();
    }
    bool waitForConnected(int msecs = 30000) override;
    bool waitForDisconnected(int msecs = 30000) override;
    bool waitForBytesWritten(int msecs = 30000) override;
    bool waitForReadyRead(int msecs = 30000) override;
    void startTimer(int interval, Qt::TimerType timerType = Qt::CoarseTimer) {
        if(timerId) killTimer(timerId);
        timerId = QTcpSocket::startTimer(interval, timerType);
    }
    void stopTimer() {
        if(timerId==0) return;
        killTimer(timerId);
        timerId = 0;
    }
    int timerId{0};

protected:
    void timerEvent(QTimerEvent *e) override {
        if(e->timerId()!=timerId) QTcpSocket::timerEvent(e);
        else {
            abort();
            setSocketError(SocketTimeoutError);
            setErrorString(QCoreApplication::translate("QAbstractSocket", "Socket operation timed out"));
            emit errorOccurred(QAbstractSocket::SocketTimeoutError);
        }
    };
    bool connAndExec(int msecs, QEventLoop *loop);
};

#endif // QNETWORK_H
