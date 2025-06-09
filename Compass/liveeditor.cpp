#include "liveeditor.h"
#include "gutil/qgui.h"
#include "main.h"
#include "layer.h"
#include <QMessageBox>
#include <QToolBar>
#include <QLineEdit>
#include <QMouseEvent>
#include <QOpenGLWidget>

LiveEditor::LiveEditor(QWidget *parent) : QWidget(parent) {
    auto hBox = new HBox(this);
    hBox->setContentsMargins(0, 0, 0, 0);
    hBox->setSpacing(0);

    auto format = QSurfaceFormat::defaultFormat();
    format.setSwapBehavior(QSurfaceFormat::SingleBuffer);
    QSurfaceFormat::setDefaultFormat(format);

    gEditView = new EditView;
    gEditView->setViewport(new QOpenGLWidget);
    gEditView->setBackgroundBrush(Qt::black);
    gEditView->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    gEditView->setTransformationAnchor(QGraphicsView::NoAnchor);
    gEditView->setInteractive(false);
    gEditView->setSceneRect({-gOrigin / gScale, QSizeF(1, 1)});
    gEditView->setScene(gScene = new QGraphicsScene(this));
    gEditView->scale(gScale, gScale);

    gEditView->originWgt = new OriginWgt(gEditView);
    gEditView->originWgt->setGeometry(gOrigin.x()-10, gOrigin.y()-10, 20, 20);

    hBox->addWidget(gEditView);

    auto toolBar = new QToolBar;
    toolBar->setOrientation(Qt::Vertical);
    toolBar->setStyleSheet("QToolBar{spacing: 2px;}");
    toolBar->setIconSize({18, 18});

    auto actScaleAdd = new QAction(QIcon(":/res/program/ScaleUp.png"), tr("Zoom In"));
    toolBar->addAction(actScaleAdd);
    connect(actScaleAdd, &QAction::triggered, this, [this] {
        if(gScale >= 8) return;
        gScale *= 1.1;
        scaleChanged();
    });

    edScale = new QLabel("0.1");
    edScale->setMaximumWidth(24);
    edScale->setAlignment(Qt::AlignCenter);
    toolBar->addWidget(edScale);

    auto actScaleMinus = new QAction(QIcon(":/res/program/ScaleDown.png"), tr("Zoom Out"));
    toolBar->addAction(actScaleMinus);
    connect(actScaleMinus, &QAction::triggered, this, [this] {
        if(gScale <= 0.01) return;
        gScale *= 0.909090909090;
        scaleChanged();
    });

    auto actScaleOrg = new QAction(QIcon(":/res/program/ScaleOrg.png"), tr("Original Size"));
    connect(actScaleOrg, &QAction::triggered, this, [this] {
        gScale = 0.1;
        scaleChanged();
    });
    toolBar->addAction(actScaleOrg);

    auto actFull = new QAction(QIcon(":/res/program/TileFull.png"), tr("Full Screen"));
    connect(actFull, &QAction::triggered, this, &LiveEditor::onTileFull);
    toolBar->addAction(actFull);
    hBox->addWidget(toolBar);
}

void LiveEditor::scaleChanged() {
    gEditView->setSceneRect({-gOrigin / gScale, QSizeF(1, 1)});
    QTransform tran;
    tran.scale(gScale, gScale);
    gEditView->setTransform(tran);
    for(auto layer : gEditView->layers) layer->updateGeo();
    edScale->setText(QString::number(gScale));
}

void LiveEditor::onTileFull() {
    // EBase *element = getElementSelected();
    // if(nullptr == element) return;
    // element->setPos(0,0);
}

void EditView::select(Layer *layer) {
    if(selected==layer) return;
    if(selected) selected->update();
    selected = layer;
    if(selected==0) return;
    selected->update();
    if(selected->item) {
        gOutPanel->edName->setText(selected->item->text("name"**gOutPanel->tree));
        gOutPanel->edX->blockSignals(true);
        gOutPanel->edY->blockSignals(true);
        gOutPanel->edW->blockSignals(true);
        gOutPanel->edH->blockSignals(true);
        gOutPanel->edX->setValue(selected->sPos.x());
        gOutPanel->edY->setValue(selected->sPos.y());
        gOutPanel->edW->setValue(selected->sSize.width());
        gOutPanel->edH->setValue(selected->sSize.height());
        gOutPanel->edX->blockSignals(false);
        gOutPanel->edY->blockSignals(false);
        gOutPanel->edW->blockSignals(false);
        gOutPanel->edH->blockSignals(false);
    }
}
void EditView::mousePressEvent(QMouseEvent *e) {
    QWidget::mousePressEvent(e);
    if(e->button() != Qt::LeftButton) return;
    pressRel = gOrigin - e->globalPosition().toPoint();
}
void EditView::mouseReleaseEvent(QMouseEvent *event) {
    QWidget::mouseReleaseEvent(event);
    if(Qt::LeftButton == event->button()) {
        pressRel.setX(INT_MAX);
    }
}
void EditView::mouseMoveEvent(QMouseEvent *e) {
    if(! (e->buttons() & Qt::LeftButton)) return;
    if(pressRel.x()==INT_MAX) return;
    auto mousePos = e->globalPosition().toPoint();
    gOrigin = pressRel + mousePos;
    originWgt->move(gOrigin - QPoint{10,10});
    gEditView->setSceneRect({-gOrigin / gScale, QSizeF(1, 1)});
    for(auto child : children()) {
        auto layer = dynamic_cast<Layer *>(child);
        if(layer) layer->move(layer->sPos*gScale+gOrigin);
    }
}
