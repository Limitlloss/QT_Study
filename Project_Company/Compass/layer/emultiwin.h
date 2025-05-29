#ifndef EMULTIWIN_H
#define EMULTIWIN_H

#include "ebase.h"
#include "pagelistitem.h"

class EMultiWin : public EBase {
    Q_OBJECT
public:
    explicit EMultiWin(PageListItem *pageItem);
    explicit EMultiWin(const JObj &json, PageListItem *pageItem);
    ~EMultiWin();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    int type() const override { return EBase::Window; }
    QWidget* attrWgt() override;
    bool save(const QString &) override;
    JObj attrJson() const override;

    void setCur(EBase *);

    PageListItem *mPageItem = 0;
    std::vector<EBase*> inners;
    int index{-1};
};

#endif // EMULTIWIN_H
