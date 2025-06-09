#ifndef PROGRESSPANEL_H
#define PROGRESSPANEL_H

#include "gutil/qgui.h"
#include <QGraphicsItem>
#include <QMediaPlayer>

// 进度面板类，继承自QWidget
class ProgressPanel : public QWidget {
    Q_OBJECT
public:
    // 构造函数，接受一个QWidget指针作为父窗口，默认为0
    explicit ProgressPanel(QWidget* parent = 0);

    // 添加一行的方法
    void addRow();

    // 播放的方法
    void play();

    // 垂直布局指针
    VBox* vBox;
};

#endif // PROGRESSPANEL_H

