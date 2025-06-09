#ifndef OPENDLG_H
#define OPENDLG_H

#include "gutil/qjson.h"
#include <QDialog>
#include <QLabel>
#include <QListWidgetItem>

// 打开对话框类，继承自QDialog
class OpenDlg : public QDialog {
    Q_OBJECT
public:
    // 构造函数，初始化打开对话框，参数为父窗口指针，默认为nullptr
    explicit OpenDlg(QWidget* parent = nullptr);

};

// 最近使用的列表项类，继承自QObject和QListWidgetItem
class RecentListItem : public QObject, public QListWidgetItem {
    Q_OBJECT
public:
    // 构造函数，初始化最近使用的列表项，参数为属性对象和页面目录字符串
    explicit RecentListItem(const JObj& attr, const QString& pageDir);

    // void updateJson();
    // bool saveFiles();
    // QWidget *itemWgt();
    // QWidget *attrWgt();

    // 属性对象
    JObj mAttr;
    // 页面目录字符串
    QString mPageDir;
    // 属性窗口部件指针
    QWidget* mAttrWgt{ 0 };
    // 音频列表部件指针
    QListWidget* mAudiosList{ 0 };
    // 文件索引标签指针
    QLabel* fdIdx{ 0 };
    // 文件播放次数标签指针
    QLabel* fdPlayTimes{ 0 };
    // 缩放比例
    qreal scale{ 1 };
    // 视图宽度
    int viewW{ 120 };
    // 视图高度
    int viewH{ 80 };
};

#endif // OPENDLG_H

