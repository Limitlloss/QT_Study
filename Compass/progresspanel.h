#ifndef PROGRESSPANEL_H
#define PROGRESSPANEL_H

#include "gutil/qgui.h"
#include <QGraphicsItem>
#include <QMediaPlayer>

class ProgressPanel : public QWidget {
    Q_OBJECT
public:
    explicit ProgressPanel(QWidget *parent = 0);

    void addRow();
    void play();

    VBox *vBox;
};

#endif // PROGRESSPANEL_H
