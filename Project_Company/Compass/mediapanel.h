#ifndef PROGPANEL_H
#define PROGPANEL_H

#include "gutil/qgui.h"
#include <QPushButton>
#include <QSettings>
#include <QTableWidget>
#include <QDialog>

// MediaTree 类继承自 TreeWidget，重写了 dropEvent 方法以支持拖放操作
class MediaTree : public TreeWidget {
    Q_OBJECT
public:
    using TreeWidget::TreeWidget;

protected:
    void dropEvent(QDropEvent* event) override;
};

// 声明一个 MediaTree 指针的全局变量
extern MediaTree* gMediaTree;

// MediaPanel 类继承自 QWidget，用于媒体管理的面板
class MediaPanel : public QWidget {
    Q_OBJECT
public:
    explicit MediaPanel(QWidget* parent = 0);

protected:
    void changeEvent(QEvent*) override;
    void transUi();

private:
    QString mProgsDir;
};

// MediaItem 类继承自 TreeWidgetItem，表示媒体项，包含文件路径和缩略图
class MediaItem : public TreeWidgetItem {
public:
    explicit MediaItem(const QString& file, TreeWidget* parent) : TreeWidgetItem(parent), file(file) {}

    QString file;
    QImage profile;
};

#endif // PROGPANEL_H
