#ifndef OPENDLG_H
#define OPENDLG_H

#include "gutil/qjson.h"
#include <QDialog>
#include <QLabel>
#include <QListWidgetItem>

class OpenDlg : public QDialog {
    Q_OBJECT
public:
    explicit OpenDlg(QWidget *parent = nullptr);

};

class RecentListItem : public QObject, public QListWidgetItem {
    Q_OBJECT
public:
    explicit RecentListItem(const JObj &attr, const QString &pageDir);

    // void updateJson();
    // bool saveFiles();
    // QWidget *itemWgt();
    // QWidget *attrWgt();

    JObj mAttr;
    QString mPageDir;
    QWidget *mAttrWgt{0};
    QListWidget *mAudiosList{0};
    QLabel *fdIdx{0}, *fdPlayTimes{0};
    qreal scale{1};
    int viewW{120}, viewH{80};
};

#endif // OPENDLG_H
