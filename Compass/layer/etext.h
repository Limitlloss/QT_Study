#ifndef ETEXT_H
#define ETEXT_H

#include "ebase.h"

class EText : public EBase {
    Q_OBJECT
public:
    explicit EText(EBase *multiWin = 0);
    explicit EText(const JObj &json, EBase *multiWin = 0);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    int type() const override { return EBase::Text; }
    QWidget* attrWgt() override;
    bool save(const QString &pRoot) override;
    JObj attrJson() const override;

    QString text;
    Qt::Alignment align;
    QColor backColor = Qt::transparent;
    QString lastFont = "黑体";
    QString playMode = "Flip";
    QString direction = "left";
    int speed = 60;
    int tailSpacing = 10;
    bool useNewFmt = false;

public slots:
    void updImg();
signals:
    void updPageCnt();

private:
    void alignV(QImage &, int h=0);
    QImage copy(QImage &img, int x, int y, int w, int h);

    QList<QImage> mImgs;
    int curIdx{0};
};

#endif // ETEXT_H
