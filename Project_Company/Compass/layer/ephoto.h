#ifndef EPHOTO_H
#define EPHOTO_H

#include "ebase.h"
#include "pagelistitem.h"

class EPhoto : public EBase {
    Q_OBJECT
public:
    static QString filters() { return tr("Images (*.png *.jpg *.jpeg *.bmp)"); }
    static EPhoto *create(const QString &file, PageListItem *pageItem, EBase *multiWin = nullptr);
    static EPhoto *create(const JObj &, PageListItem *pageItem, EBase *multiWin = nullptr);

    explicit EPhoto(const QImage&, const QString &dir, const QString &name, const JObj &json, PageListItem *pageItem, EBase *multiWin = nullptr);

    void scaleImgIfNeed();

    int type() const override { return EBase::Image; }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    JObj attrJson() const override;
    void freeFiles() override;
    void loadFiles() override;
    bool save(const QString &pageDir) override;
    QWidget* attrWgt() override;

    QImage img;
    QString mDir, mName, direct;
    int speed = 0, tailSpacing = 0;
protected:
    PageListItem *mPageItem;
};

#endif // EPHOTO_H
