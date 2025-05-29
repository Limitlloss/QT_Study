#ifndef OUTPUTPANEL_H
#define OUTPUTPANEL_H

#include "gutil/qgui.h"
#include <QSpinBox>

class OutputPanel : public QWidget {
    Q_OBJECT
public:
    explicit OutputPanel(QWidget *parent = 0);

    TreeWidget *tree;
    QLineEdit *edName;
    QSpinBox *edX, *edY, *edW, *edH;
    bool enCurChanged = true;
protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void changeEvent(QEvent *) override;
    void transUi();
};

class OutputItem : public TreeWidgetItem {
public:
    using TreeWidgetItem::TreeWidgetItem;

    QPoint pos;
    QSize size{1920, 1080};
};

#endif // OUTPUTPANEL_H
