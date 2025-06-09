#ifndef OUTPUTPANEL_H
#define OUTPUTPANEL_H

#include "gutil/qgui.h"
#include <QSpinBox>

// OutputPanel 类继承自 QWidget，用于显示输出设置的面板
class OutputPanel : public QWidget {
    Q_OBJECT
public:
    // 构造函数，初始化 OutputPanel 对象
    explicit OutputPanel(QWidget* parent = 0);

    TreeWidget* tree; // 树形控件，用于显示输出项
    QLineEdit* edName; // 文本编辑框，用于编辑输出项的名称
    QSpinBox* edX, * edY, * edW, * edH; // 旋钮控件，用于设置输出项的位置和大小
    bool enCurChanged = true; // 标志变量，指示当前输出项是否已更改
protected:
    // 重写显示事件处理函数，当面板显示时调用
    void showEvent(QShowEvent* event) override;
    // 重写隐藏事件处理函数，当面板隐藏时调用
    void hideEvent(QHideEvent* event) override;
    // 重写事件处理函数，当面板的语言或其他设置更改时调用
    void changeEvent(QEvent*) override;
    // 翻译界面元素的函数
    void transUi();
};

// OutputItem 类继承自 TreeWidgetItem，用于表示单个输出项
class OutputItem : public TreeWidgetItem {
public:
    // 使用 TreeWidgetItem 的构造函数
    using TreeWidgetItem::TreeWidgetItem;

    QPoint pos; // 输出项的位置
    QSize size{ 1920, 1080 }; // 输出项的大小，默认为 1920x1080
};

#endif // OUTPUTPANEL_H
