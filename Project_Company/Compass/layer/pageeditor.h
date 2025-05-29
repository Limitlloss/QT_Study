#ifndef PAGEEDITOR_H
#define PAGEEDITOR_H

#include <QLabel>
#include <QGraphicsView>

class EBase;
class PageListItem;
class PageEditor : public QWidget {
    Q_OBJECT
public:
    explicit PageEditor(QWidget *parent = nullptr);
    virtual void keyReleaseEvent(QKeyEvent *event) override;

    QList<EBase*> sortedEles();
    EBase* getElementSelected();

    QGraphicsView *graphicsView;
    QLabel *fdScale;
    int curScale{100};

public slots:
    void onScale(int);
    void onDelete();
    void onClean();
    void onLayerUp();
    void onLayerDown();
    void onSelectionLeft();
    void onSelectionRight();
    void onSelectionTop();
    void onSelectionBottom();
    void onLayerTop();
    void onLayerBottom();
    void onTileFull();
    void onTileH();
    void onTileV();
    void onArrayBottom();
    void onArrayHCenter();
    void onArrayTop();
    void onArrayRight();
    void onArrayVCenter();
    void onArrayLeft();
};

#endif // PAGEEDITOR_H
