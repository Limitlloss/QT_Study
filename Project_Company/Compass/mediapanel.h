#ifndef PROGPANEL_H
#define PROGPANEL_H

#include "gutil/qgui.h"
#include <QPushButton>
#include <QSettings>
#include <QTableWidget>
#include <QDialog>

class MediaTree : public TreeWidget {
    Q_OBJECT
public:
    using TreeWidget::TreeWidget;

protected:
    void dropEvent(QDropEvent *event) override;
};

extern MediaTree *gMediaTree;

class MediaPanel : public QWidget {
    Q_OBJECT
public:
    explicit MediaPanel(QWidget *parent = 0);

protected:
    void changeEvent(QEvent *) override;
    void transUi();

private:
    QString mProgsDir;
};

class MediaItem : public TreeWidgetItem {
public:
    explicit MediaItem(const QString &file, TreeWidget *parent) : TreeWidgetItem(parent), file(file) {}

    QString file;
    QImage profile;
};

#endif // PROGPANEL_H
