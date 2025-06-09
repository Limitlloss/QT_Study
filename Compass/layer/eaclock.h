#ifndef EACLOCK_H
#define EACLOCK_H

#include "ebase.h"
#include <QTimeZone>
#include "gutil/qgui.h"

class EAClock : public EBase {
    Q_OBJECT
public:
    struct Data {
        QTimeZone timeZone;
        int hourMark{0};//时针
        int hourMarkSize{5};//时针大小
        QColor hourMarkColor{Qt::green};//时针颜色
        int minMark{1};//分针
        int minMarkSize{2};//分针大小
        QColor minMarkColor{Qt::yellow};//分针颜色
        QColor hourHandColor;//秒针
        QColor minHandColor;//秒针大小
        QColor secHandColor;//秒针颜色
        int hhLen{50}, mhLen{75}, shLen{100};
        int hhWidth{15}, mhWidth{10}, shWidth{5};
        QString text;//标题
        QFont textFont = qfont("Arial", 12);//标题字体
        QColor textColor;//标题字体颜色
        QString path;
        QString name;
        bool hasDialImg{false};
        bool showSecHand{true};
    };

    explicit EAClock(EBase *multiWin = nullptr);
    explicit EAClock(const JObj &json, EBase *multiWin = nullptr);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    int type() const override { return EBase::AClock; }
    QWidget* attrWgt() override;
    bool save(const QString &pRoot) override;
    JObj attrJson() const override;

protected:
    void timerEvent(QTimerEvent *) override;
    void drawMarkCircular (QPainter *painter, const QPointF &pos, const QColor &color, qreal diameter);
    void drawMarkRectangle(QPainter *painter, const QPointF &pos, const QColor &color, qreal len, qreal rotate);
    void drawMarkNumber   (QPainter *painter, const QPointF &pos, const QColor &color, qreal len, int num);
    void paintDial(QPainter *painter);
    void paintText(QPainter *painter);

    void cal();

    Data m_attr;
    QImage dial_img;
    double hAngle, mAngle, sAngle;
    int timerId{0};
};

#endif // EACLOCK_H
