#ifndef EVIDEO_H
#define EVIDEO_H

#include "ebase.h"
#include "pagelistitem.h"
extern "C"{
#include <libavcodec/avcodec.h>
}
class ProgItem;
class EVideo : public EBase {
    Q_OBJECT
public:
    static QString filters() {
        static const QString filters = tr("Video")+"(*.mp4 *.mkv *.ts *.m2ts *.tp *.rm *.rmvb *.avi *.wmv *.mov);;"+tr("Flash")+"(*.flv *.swf)";
        return filters;
    }
    static const QImage &mask() {
        static const QImage mask = QImage(":/res/video-pre.png");
        return mask;
    }
    static QString transcoding(QWidget *parent, QString rawFile, QString rawName, QString dir, int rawW, int rawH, AVCodecID codec_id);
    static EVideo *create(const QString &file, PageListItem *pageItem, EBase *multiWin = nullptr);
    static EVideo *create(const JObj &, PageListItem *pageItem, EBase *multiWin = nullptr);

    explicit EVideo(const QString &, const QString &, const QString &, const QString &, QImage &img, PageListItem *pageItem, EBase *multiWin = nullptr);

    int type() const override { return EBase::Video; }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QWidget* attrWgt() override;
    bool save(const QString &pRoot) override;
    JObj attrJson() const override;

    QString mDir;
    QString mName;
    QString mRawDir;
    QString mRawName;
    QImage mCoverImg;
protected:
    int aspectRatioMode = Qt::IgnoreAspectRatio;
    int playTimes = 1;
    bool useSW = false;
    PageListItem *mPageItem;
};

#endif // EVIDEO_H
