#ifndef QCORE_H
#define QCORE_H
#include <QThread>
#include <QEventLoop>
#include <QTimer>

#define ToStr QString::number

inline QString toStr(int num, int base=10) {return QString::number(num, base);}
inline QString toStr(uint num, int base=10) {return QString::number(num, base);}
inline QString toStr(long num, int base=10) {return QString::number(num, base);}
inline QString toStr(ulong num, int base=10) {return QString::number(num, base);}
inline QString toStr(qlonglong num, int base=10) {return QString::number(num, base);}
inline QString toStr(qulonglong num, int base=10) {return QString::number(num, base);}
inline QString toStr(double num, char format='g', int precision=6) {return QString::number(num, format, precision);}

inline QString gUrlSuffix(const QString &url, int size, bool withDot = false) {
    auto ss = url.lastIndexOf('/')+1;
    if(ss==url.size()) return QString();
    ss++;
    auto ee = url.indexOf('?', ss);
    if(ee==-1) ee = url.size();
    int end = ee-size-1;
    if(end<ss) end = ss;
    for(int i=ee-1; i>=end; --i) if(url[i]=='.') return withDot ? url.mid(i, ee-i) : url.mid(i+1, ee-i-1);
    return QString();
}

inline QString byteSizeStr(double size) {
    const char *units[]{"B", "KB", "MB", "GB", "TB", "PB", "EB"};
    auto i = 0;
    for(; size >= 1024 && i < 7; i++) size /= 1024;
    return (size > 99 ? QString::number(size, 'f', 0) : QString::number(size, 'g', 3))+" "+units[i];
}

inline void wait(int msec, QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents) {
    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(msec);
    loop.exec(flags);
}

template <typename Func>
inline QThread *ThreadStart(Func &&f) {
    QThread* thread = QThread::create(f);
    QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
    return thread;
}

#endif
