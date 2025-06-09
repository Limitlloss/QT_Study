#ifndef PROGITEM_H
#define PROGITEM_H

#include "base/loqtreewidget.h"
#include <QJsonObject>
#include <QPushButton>

class ProgPanel;
class ProgItem : public TreeWidgetItem {
public:
    using TreeWidgetItem::TreeWidgetItem;
    explicit ProgItem(const QString &progsDir, const QString &name, int w, int h, const QString & remarks, std::vector<int> &, int, bool isVer, LoQTreeWidget *parent);

    void save();
    void del();
    void onSetProgram();

    QString mName;
    int mWidth;
    int mHeight;
    QString mRemark;
    std::vector<int> partLens;
    int maxLen = 0;
    bool isInsert = false, isVer = false;
    QString mProgsDir;
    QString mProgDir;
    qint64 m_fsize = 0;
    QPushButton *btnName;
    void init();
    QString m_orgName;
};

#endif // PROGITEM_H
