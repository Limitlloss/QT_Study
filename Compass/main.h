#ifndef MAIN_H
#define MAIN_H

#include "gutil/qgui.h"
#include "gutil/qjson.h"
#include "liveeditor.h"
#include "outputpanel.h"
#include <QCoreApplication>
#include "QImageReader"
#include <QJsonDocument>
#include <QNetworkReply>
#include <QTextEdit>
#include <QGraphicsView>

extern QString gFileHome;
extern QString gProgFile;
extern JObj gProg;
extern const QString UpdVerUrl;
extern QGraphicsScene *gScene;
extern EditView *gEditView;
extern double gScale;
extern QPoint gOrigin;
extern OutputPanel *gOutPanel;
extern TableWidget *gTable, *gTableH, *gTableV;
extern int gPlayinC;
//extern Tick *gTick;
extern int gProgWidth, gProgHeight;

#define PAGEDEL_SUFFIX "@D$E$L&20111005&"
#define RECTF_INVALID  QRectF(-9999, -9999, 0, 0)

inline QString readErrStr(QImageReader::ImageReaderError err) {
    if(err==QImageReader::UnknownError) return "UnknownError";
    if(err==QImageReader::FileNotFoundError) return "FileNotFoundError";
    if(err==QImageReader::DeviceError) return "DeviceError";
    if(err==QImageReader::UnsupportedFormatError) return "UnsupportedFormatError";
    if(err==QImageReader::InvalidDataError) return "InvalidDataError";
    return QString::number(err);
}

// class Tick : public QOpenGLWidget {
//     Q_OBJECT
// public:
//     Tick(QWidget *parent = 0);

//     void doFrame();

//     qint64 Sec;
// signals:
//     void secChanged(const QDateTime &);
// };
enum {
   MainPage_DeviceManager = 0,
   MainPage_ProgManager,
   MainPage_Setting,
   MainPage_LoraScreen,
   MainPage_End,
};


struct ST_ANSY_PROGRAM_PACKET {
    unsigned char SyncHead[3]{0x7e, 0x7e, 0x55};
    unsigned char ucCommType;
    int iBaoLiu;
    unsigned int iLength;
    unsigned char pDataBuffer[20];
};

QString programsDir();

extern quint64 dirFileSize(const QString &path);
extern bool copyDir(const QString &source, const QString &destination, bool override);
extern unsigned char GetCheckCodeIn8(unsigned char * pBuffer,unsigned int uiSize);

class DeviceItem;
extern DeviceItem *findItem(QString id);

inline QString translate(const char *ctx, const char *key) {
    return QCoreApplication::translate(ctx, key);
}
inline int verCompare(const QString& a, const QString& b) {
    auto aparts = a.split(".");
    auto bparts = b.split(".");
    int cnt = qMin(aparts.count(), bparts.count());
    for(int i=0; i<cnt; ++i) {
        int aaa = aparts[i].toInt();
        int bbb = bparts[i].toInt();
        if(aaa != bbb) return aaa - bbb;
    }
    if(aparts.count() == bparts.count()) return 0;
    return aparts.count() > bparts.count() ? aparts[cnt].toInt() : bparts[cnt].toInt();
}

class LocalObj : public QObject {
public :
    int cnt = 1;
    void reset() {
        cnt = 0;
    }
};

#endif // MAIN_H
