#ifndef PAGELISTITEM_H
#define PAGELISTITEM_H

#include "gutil/qjson.h"
#include <QGraphicsScene>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QTimeEdit>

class PageListItem : public QObject, public QListWidgetItem {
    Q_OBJECT
public:
    explicit PageListItem(const JObj &attr, const QString &pageDir);
    ~PageListItem() {
        if(mAttrWgt) delete mAttrWgt;
    }

    void updateJson();
    bool saveFiles();
    QWidget *itemWgt();
    QWidget *attrWgt();

    JObj mAttr;
    QString mPageDir;
    QWidget *mAttrWgt{0};
    QListWidget *mAudiosList{0};
    QGraphicsScene *mScene{0};
    QGraphicsView *mGraView{0};
    QLabel *fdIdx{0}, *fdPlayTimes{0};
    qreal scale{1};
    int viewW{120}, viewH{80};
};

struct AudioInfo {
    QString dir;
    QString name;
    int dur{0};
    int vol{100};
};
Q_DECLARE_METATYPE(AudioInfo)

class PlanItemWgt : public QWidget {
    Q_OBJECT
public:
    explicit PlanItemWgt(QListWidgetItem *item, const JObj * = 0);

    QLabel *fdIdx;
    QListWidgetItem *m_item;

    QTimeEdit *tStart;
    QTimeEdit *tEnd;
    QPushButton *bnDel;
    QPushButton *btnDays[7];
signals:
    void sigItemChanged();
};

#endif // PAGELISTITEM_H
