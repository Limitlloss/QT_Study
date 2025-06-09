#include "pageeditor.h"
#include "main.h"
#include "ebase.h"
#include <QKeyEvent>
#include <QMessageBox>
#include <QToolBar>
#include <QVBoxLayout>

PageEditor::PageEditor(QWidget *parent) : QWidget(parent) {
    auto vBox = new QVBoxLayout(this);
    vBox->setContentsMargins(0, 0, 0, 0);
    vBox->setSpacing(0);

    //主区域编辑工具栏
    auto toolBar = new QToolBar();
    toolBar->setStyleSheet("QToolBar{spacing: 2px;}");
    toolBar->setIconSize(QSize(18, 18));
    toolBar->addSeparator();
    auto actScaleUp = new QAction(QIcon(":/res/program/ScaleUp.png"), tr("Zoom In"));
    toolBar->addAction(actScaleUp); //放大
    connect(actScaleUp, &QAction::triggered, this, [this] {
        onScale(10);
    });

    fdScale = new QLabel("100");
    auto pal = fdScale->palette();
    pal.setBrush(QPalette::Button, Qt::white);
    fdScale->setPalette(pal);
    fdScale->setAutoFillBackground(true);
    fdScale->setFixedWidth(32);
    fdScale->setAlignment(Qt::AlignCenter);
    toolBar->addWidget(fdScale);//当前视图比例

    auto actScaleDown = new QAction(QIcon(":/res/program/ScaleDown.png"),    tr("Zoom Out"));
    toolBar->addAction(actScaleDown);//缩小
    connect(actScaleDown, &QAction::triggered, this, [this] {
        onScale(-10);
    });

    QAction *actScaleOrg = new QAction(QIcon(":/res/program/ScaleOrg.png"),     tr("Original size"));
    connect(actScaleOrg, &QAction::triggered, this, [this] {
        curScale = 100;
        fdScale->setText("100");
        graphicsView->resetTransform();
    });
    toolBar->addAction(actScaleOrg);//1:1恢复视图
    toolBar->addSeparator();

    QAction *actDelete       = new QAction(QIcon(":/res/program/Delete.png"),       tr("Delete the secect media"));
    QAction *actClean        = new QAction(QIcon(":/res/program/Clean.png"),        tr("Clear all media"));
    //QAction *actCut          = new QAction(QIcon(":/res/program/Cut.png"),          tr("Cut media"));
    QAction *actLayerUp      = new QAction(QIcon(":/res/program/LayerUp.png"),      tr("Move layer up"));
    QAction *actLayerDown    = new QAction(QIcon(":/res/program/LayerDown.png"),    tr("Send backward"));
    QAction *actLayerTop     = new QAction(QIcon(":/res/program/LayerTop.png"),     tr("Bring to front"));
    QAction *actLayerBottom  = new QAction(QIcon(":/res/program/LayerBottom.png"),  tr("Move to the bottom layer"));
    QAction *actTileFull     = new QAction(QIcon(":/res/program/TileFull.png"),     tr("Fill the entire screen"));
    QAction *actTileH        = new QAction(QIcon(":/res/program/TileH.png"),        tr("Fill the screen horizontally"));
    QAction *actTileV        = new QAction(QIcon(":/res/program/TileV.png"),        tr("Fill the screen vertically"));
    QAction *actArrayTop     = new QAction(QIcon(":/res/program/ArrayTop.png"),     tr("Align top"));
    QAction *actArrayHCenter = new QAction(QIcon(":/res/program/ArrayHCenter.png"), tr("Center vertically"));
    QAction *actArrayBottom  = new QAction(QIcon(":/res/program/ArrayBottom.png"),  tr("Bottom align"));
    QAction *actArrayLeft    = new QAction(QIcon(":/res/program/ArrayLeft.png"),    tr("Align left"));
    QAction *actArrayVCenter = new QAction(QIcon(":/res/program/ArrayVCenter.png"), tr("Center horizontally"));
    QAction *actArrayRight   = new QAction(QIcon(":/res/program/ArrayRight.png"),   tr("Align right"));
    toolBar->addAction(actDelete);
    toolBar->addAction(actClean);
    toolBar->addSeparator();
    toolBar->addAction(actLayerUp);
    toolBar->addAction(actLayerDown);
    toolBar->addAction(actLayerTop);
    toolBar->addAction(actLayerBottom);
    toolBar->addSeparator();
    toolBar->addAction(actTileFull);
    toolBar->addAction(actTileH);
    toolBar->addAction(actTileV);
    toolBar->addSeparator();
    toolBar->addAction(actArrayTop);
    toolBar->addAction(actArrayHCenter);
    toolBar->addAction(actArrayBottom);
    toolBar->addAction(actArrayLeft);
    toolBar->addAction(actArrayVCenter);
    toolBar->addAction(actArrayRight);
    connect(actDelete,       SIGNAL(triggered(bool)),   this,   SLOT(onDelete()));
    connect(actClean,        SIGNAL(triggered(bool)),   this,   SLOT(onClean()));
    connect(actLayerUp,      SIGNAL(triggered(bool)),   this,   SLOT(onLayerUp()));
    connect(actLayerDown,    SIGNAL(triggered(bool)),   this,   SLOT(onLayerDown()));
    connect(actLayerTop,     SIGNAL(triggered(bool)),   this,   SLOT(onLayerTop()));
    connect(actLayerBottom,  SIGNAL(triggered(bool)),   this,   SLOT(onLayerBottom()));
    connect(actTileFull,     SIGNAL(triggered(bool)),   this,   SLOT(onTileFull()));
    connect(actTileH,        SIGNAL(triggered(bool)),   this,   SLOT(onTileH()));
    connect(actTileV,        SIGNAL(triggered(bool)),   this,   SLOT(onTileV()));
    connect(actArrayTop,     SIGNAL(triggered(bool)),   this,   SLOT(onArrayTop()));
    connect(actArrayHCenter, SIGNAL(triggered(bool)),   this,   SLOT(onArrayHCenter()));
    connect(actArrayBottom,  SIGNAL(triggered(bool)),   this,   SLOT(onArrayBottom()));
    connect(actArrayLeft,    SIGNAL(triggered(bool)),   this,   SLOT(onArrayLeft()));
    connect(actArrayVCenter, SIGNAL(triggered(bool)),   this,   SLOT(onArrayVCenter()));
    connect(actArrayRight,   SIGNAL(triggered(bool)),   this,   SLOT(onArrayRight()));
    vBox->addWidget(toolBar);

    graphicsView = new QGraphicsView;
    pal = graphicsView->palette();
    pal.setBrush(QPalette::Base, QColor(0xbbbbbb));
    graphicsView->setPalette(pal);
    vBox->addWidget(graphicsView);
}

void PageEditor::keyReleaseEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Delete:   onDelete();      break;
    case Qt::Key_Home:     onLayerTop();    break;
    case Qt::Key_End:      onLayerBottom(); break;
    case Qt::Key_PageUp:   onLayerUp();     break;
    case Qt::Key_PageDown: onLayerDown();   break;
    case Qt::Key_Left:     onSelectionLeft(); break;
    case Qt::Key_Right:    onSelectionRight(); break;
    case Qt::Key_Up:       onSelectionTop(); break;
    case Qt::Key_Down:     onSelectionBottom(); break;
    default:break;
    }
}

QList<EBase*> PageEditor::sortedEles() {
    QList<EBase*> eles;
    auto scene = graphicsView->scene();
    if(nullptr == scene) return eles;
    auto items = scene->items(Qt::AscendingOrder);
    foreach(auto item, items) {
        auto ele = static_cast<EBase*>(item);
        if(ele->mMultiWin == nullptr) eles.append(ele);
    }
    return eles;
}

EBase* PageEditor::getElementSelected(){
    auto scene = graphicsView->scene();
    if(nullptr == scene) return nullptr;
    auto selectedItems = scene->selectedItems();
    if(selectedItems.count() != 1) return nullptr;
    return static_cast<EBase*>(selectedItems.at(0));
}

void PageEditor::onScale(int angle) {
    if(angle>0) {
        if(curScale >= 800) return;
    } else if(curScale <= 10) return;
    curScale += angle;
    fdScale->setText(QString::number(curScale));
    graphicsView->resetTransform();
    qreal scale = curScale / 100.0;
    graphicsView->scale(scale, scale);
}
void PageEditor::onDelete() {
    auto scene = graphicsView->scene();
    if(0==scene) return;
    auto selectedItems = scene->selectedItems();
    if(selectedItems.count() == 0) return;
    foreach(QGraphicsItem *selectedItem, selectedItems) if(selectedItem->type() >= QGraphicsItem::UserType) {
        scene->removeItem(selectedItem);
        static_cast<EBase*>(selectedItem)->freeFiles();
        delete selectedItem;
    }
    auto eles = sortedEles();
    for(int i=0; i<eles.size(); i++) eles[i]->setZValue(i);
}

void PageEditor::onClean() {
    auto res = QMessageBox::information(this, translate("","Tip"),tr("Clear all medias?"), QMessageBox::Ok, QMessageBox::Cancel);
    if(res == QMessageBox::Ok) {
        auto eles = sortedEles();
        foreach(auto ele, eles) {
            graphicsView->scene()->removeItem(ele);
            delete ele;
        }
    }
}
void PageEditor::onLayerUp(){
    EBase *element = getElementSelected();
    if(nullptr == element) return;
    QList<EBase*> list = sortedEles();
    int n = list.count();
    int o = static_cast<int>(element->zValue());
    if(o < n-1) {
        element->setZValue(o+1);
        list.at(o+1)->setZValue(o);
    }
}

void PageEditor::onSelectionLeft()
{
    EBase *element = getElementSelected();
    if(nullptr == element) return;
    element->setX(x()-1);
}
void PageEditor::onSelectionRight()
{
    EBase *element = getElementSelected();
    if(nullptr == element) return;
    element->setX(x()+1);
}
void PageEditor::onSelectionTop()
{
    EBase *element = getElementSelected();
    if(nullptr == element) return;
    element->setY(y()-1);
}
void PageEditor::onSelectionBottom()
{
    EBase *element = getElementSelected();
    if(nullptr == element) return;
    element->setY(y()+1);
}
void PageEditor::onLayerDown()
{
    EBase *element = getElementSelected();
    if(nullptr == element) return;
    QList<EBase*> list = sortedEles();
    int o = static_cast<int>(element->zValue());
    if(o > 0) {
        element->setZValue(o-1);
        list.at(o-1)->setZValue(o);
    }
}
void PageEditor::onLayerTop()
{
    EBase *element = getElementSelected();
    if(nullptr == element) return;
    QList<EBase*> list = sortedEles();
    int n = list.count();
    int o = static_cast<int>(element->zValue());
    if(o < n-1) {
        for(int i=o+1; i<n; i++)
            list.at(i)->setZValue(i-1);
        element->setZValue(n-1);
    }
}

void PageEditor::onLayerBottom()
{
    EBase *element = getElementSelected();
    if(nullptr == element) return;
    QList<EBase*> list = sortedEles();
    int o = static_cast<int>(element->zValue());
    if(o > 0) {
        for(int i=0; i<o; i++)
            list.at(i)->setZValue(i+1);
        element->setZValue(0);
    }
}

void PageEditor::onTileFull() {
    EBase *element = getElementSelected();
    if(nullptr == element) return;
    element->setPos(0,0);
    element->setSize(gProgItem->mWidth, gProgItem->mHeight);
}

void PageEditor::onTileH(){
    EBase *element = getElementSelected();
    if(nullptr == element) return;
    element->setX(0);
    element->setSize(gProgItem->mWidth, element->mHeight);
}

void PageEditor::onTileV() {
    EBase *element = getElementSelected();
    if(nullptr == element) return;
    element->setY(0);
    element->setSize(element->mWidth, gProgItem->mHeight);
}

void PageEditor::onArrayTop() {
    EBase *element = getElementSelected();
    if(nullptr == element) return;
    element->setY(0);
}

void PageEditor::onArrayHCenter() {
    EBase *element = getElementSelected();
    if(nullptr == element) return;
    element->setY(floor((gProgItem->mHeight - element->mHeight) / 2));
}

void PageEditor::onArrayBottom(){
    EBase *element = getElementSelected();
    if(nullptr == element) return;
    element->setY(gProgItem->mHeight - element->mHeight);
}

void PageEditor::onArrayLeft(){
    EBase *element = getElementSelected();
    if(nullptr == element) return;
    element->setX(0);
}

void PageEditor::onArrayVCenter() {
    EBase *element = getElementSelected();
    if(nullptr == element) return;
    element->setX(floor((gProgItem->mWidth - element->mWidth) / 2));
}

void PageEditor::onArrayRight() {
    EBase *element = getElementSelected();
    if(nullptr == element) return;
    element->setX(gProgItem->mWidth - element->mWidth);
}
