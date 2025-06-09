#ifndef EGIF_H
#define EGIF_H
#include "ebase.h"
#include "pagelistitem.h"
#include "synctimer.h"

class EGif : public EBase {
    Q_OBJECT
public:
    static QString filters() { return "Gif(*.gif)"; }
    static EGif *create(const QString &file, PageListItem *pageItem, EBase *multiWin = nullptr);
    static EGif *create(const JObj &, PageListItem *pageItem, EBase *multiWin = nullptr);

    explicit EGif(QMovie*, const QString &dir, const QString &name, PageListItem *pageItem, EBase *multiWin = nullptr);
    ~EGif();
    virtual int type() const override { return EBase::Gif; }
    void paint(QPainter*, const QStyleOptionGraphicsItem *, QWidget *) override;
    QWidget* attrWgt() override;
    bool save(const QString &pRoot) override;
    JObj attrJson() const override;
    void loadFiles() override;
    void freeFiles() override;

    QMovie *mMovie;
    QString mDir;
    QString mName;
public slots:
    void sltNext();

protected:
    PageListItem *mPageItem;
    SyncTimer* timer = 0;
};

#endif // EGIF_H
