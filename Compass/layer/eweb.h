#ifndef EWEB_H
#define EWEB_H

#include "ebase.h"

class EWeb : public EBase {
    Q_OBJECT
public:
    static const QImage &holder() {
        static const QImage holder = QImage(":/res/program/Web.png");
        return holder;
    }

    explicit EWeb(EBase *multiWin = nullptr);
    explicit EWeb(const JObj &json, EBase *multiWin = nullptr);

    int type() const override {return EBase::Web;}
    void paint(QPainter*, const QStyleOptionGraphicsItem *, QWidget *) override;
    QWidget* attrWgt() override;
    bool save(const QString &) override {return true;};
    JObj attrJson() const override;

    QString url;
    int zoom = 100, refresh = 0, _x = 0, _y = 0, scaleX = 100, scaleY = 100;
};

#endif // EWEB_H
