#include "planpanel.h"
#include "mediapanel.h"
#include "main.h"
#include "layer.h"
#include <QDropEvent>
#include <QMimeData>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QGraphicsVideoItem>
#include <QMessageBox>
#include <QScrollBar>
#include <QButtonGroup>
#include <QPainter>

PlanPanel::PlanPanel(QWidget *parent) : QWidget{parent} {
    auto grid = new Grid(this);
    grid->setContentsMargins(0,0,0,0);
    grid->setHorizontalSpacing(0);
    grid->setVerticalSpacing(0);
    auto btnAddRow = new QPushButton("+");
    connect(btnAddRow, &QPushButton::clicked, this, &PlanPanel::addRow);
    grid->addWidget(btnAddRow, 0, 0);

    gTableH = new PlanTableH(1, 50);
    gTableH->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gTableH->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gTableH->horizontalHeader()->hide();
    gTableH->verticalHeader()->hide();
    gTableH->setMaximumHeight(30);
    gTableH->setColWidth(CellWidth)->setRowHeight(30)->setRowResize(QHeaderView::Fixed);
    gTableH->setDragEnabled(true);
    gTableH->setAcceptDrops(true);
    gTableH->setDropIndicatorShown(true);
    gTableH->setEditTriggers(QAbstractItemView::NoEditTriggers);
    auto grp = new QButtonGroup(this);
    for(int c=0; c<gTableH->columnCount(); ++c) {
        auto wgt = new QWidget;
        auto hBox = new HBox(wgt);
        hBox->setContentsMargins(0,0,0,0);
        hBox->setSpacing(0);
        auto btn = new QPushButton("⏵");
        btn->setMaximumWidth(24);
        grp->addButton(btn);
        hBox->addWidget(btn);
        hBox->addLabel(QString("P%1").arg(c+1));
        gTableH->setCellWidget(0, c, wgt);
    }
    connect(grp, &QButtonGroup::buttonClicked, this, &PlanPanel::play);
    grid->addWidget(gTableH, 0, 1);

    gTableV = new PlanTableV(0, 1);
    gTableV->horizontalScrollBar()->hide();
    gTableV->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    gTableV->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gTableV->horizontalHeader()->hide();
    gTableV->verticalHeader()->hide();
    gTableV->setMaximumWidth(CellWidth);
    gTableV->setColWidth(CellWidth)->setRowHeight(CellHeight)->setRowResize(QHeaderView::Fixed);
    gTableV->setDragEnabled(true);
    gTableV->setAcceptDrops(true);
    gTableV->setDropIndicatorShown(true);
    gTableV->setEditTriggers(QAbstractItemView::NoEditTriggers);
    grid->addWidget(gTableV, 1, 0);

    gTable = new PlanTable(0, 50);
    gTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    gTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gTable->horizontalHeader()->hide();
    gTable->verticalHeader()->hide();
    gTable->setColWidth(CellWidth)->setRowHeight(CellHeight)->setRowResize(QHeaderView::Fixed);
    gTable->setDragEnabled(true);
    gTable->setAcceptDrops(true);
    gTable->setDropIndicatorShown(true);
    gTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    grid->addWidget(gTable, 1, 1);

    addRow();

    connect(gTableH->horizontalScrollBar(), &QScrollBar::valueChanged, gTable->horizontalScrollBar(), &QScrollBar::setValue);
    connect(gTable->horizontalScrollBar(), &QScrollBar::valueChanged, gTableH->horizontalScrollBar(), &QScrollBar::setValue);
    connect(gTableV->verticalScrollBar(), &QScrollBar::valueChanged, gTable->verticalScrollBar(), &QScrollBar::setValue);
    connect(gTable->verticalScrollBar(), &QScrollBar::valueChanged, gTableV->verticalScrollBar(), &QScrollBar::setValue);
}

void PlanPanel::addRow() {
    gTableV->appendRow();
    gTable->appendRow();
    int r = gTable->rowCount() - 1;
    auto wgt = new QWidget;
    auto vBox = new VBox(wgt);
    vBox->setContentsMargins(0,0,0,0);
    vBox->setSpacing(0);
    auto lb = vBox->addLabel(QString("Layer %1").arg(r+1));
    lb->setFixedHeight(CellHeight - TitleHeight);
    lb->setAlignment(Qt::AlignCenter);
    lb = vBox->addLabel();
    lb->setFixedHeight(TitleHeight);
    gTableV->setCellWidget(r, 0, wgt);

    auto layer = new Layer(r+1, "Layer", gEditView);
    gEditView->layers.push_back(layer);
    layer->updateGeo();
    layer->show();
    gTableV->setData(r, 0, (quint64) layer);

    auto ft = gTable->font();
    ft.setPixelSize(10);
    for(int c=0; c<gTable->columnCount(); ++c) {
        auto wgt = new QWidget;
        auto vBox = new VBox(wgt);
        vBox->setContentsMargins(0,0,0,0);
        vBox->setSpacing(0);
        auto lb = vBox->addLabel();
        lb->setFixedHeight(CellHeight - TitleHeight);
        lb->setScaledContents(true);
        lb = vBox->addLabel();
        lb->setFixedHeight(TitleHeight);
        lb->setFont(ft);
        gTable->setCellWidget(r, c, wgt);
    }
}

void PlanPanel::play(QAbstractButton *btn) {
    auto wgt = btn->parentWidget();
    gPlayinC = 0;
    for(; gPlayinC<gTableH->columnCount(); ++gPlayinC) if(gTableH->cellWidget(0, gPlayinC)==wgt) break;
    auto items = gScene->items();
    for(auto item : items) gScene->removeItem(item);
    Cell *cell;
    for(int r=0; r<gTable->rowCount(); ++r) if((cell = (Cell*) gTable->data(r, gPlayinC).toULongLong())) {
        auto layer = (Layer*) gTableV->data(r, 0).toULongLong();
        cell->wgt->setPos(layer->sPos);
        if(cell->type=='V') {
            ((QGraphicsVideoItem*) cell->wgt)->setSize(layer->sSize);
            cell->player->play();
        } else if(cell->type=='I') {
            ((ImgItem*) cell->wgt)->size = layer->sSize;
        }
        gScene->addItem(cell->wgt);
    }
}

void ImgItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->drawPixmap(boundingRect(), img, QRectF(0, 0, img.width(), img.height()));
}

void PlanTable::dragEnterEvent(QDragEnterEvent *event) {
    if(event->source()==this || event->source()==gMediaTree) QTableWidget::dragEnterEvent(event);
}
void PlanTable::dropEvent(QDropEvent *event) {
    if(event->source()==this) {
        auto idxFrom = currentIndex();
        auto cell = data(idxFrom.row(), idxFrom.column());
        if(cell==0) goto end;
        auto idx = indexAt(event->position().toPoint());
        auto wgtFrom = cellWidget(idxFrom.row(), idxFrom.column());
        auto wgt = cellWidget(idx.row(), idx.column());
        auto lb1 = (QLabel*) wgtFrom->children().at(1);
        auto lb2 = (QLabel*) wgtFrom->children().at(2);
        ((QLabel*) wgt->children().at(1))->setPixmap(lb1->pixmap());
        ((QLabel*) wgt->children().at(2))->setText(lb2->text());
        lb1->clear();
        lb2->clear();
        auto cellOld = data(idxFrom.row(), idxFrom.column());
        if(cellOld.isValid() && cellOld!=cell) delete (Cell*) cellOld.toULongLong();
        setData(idx.row(), idx.column(), cell);
        event->accept();
    } else if(event->source()==gMediaTree) {
        auto idx = indexAt(event->position().toPoint());
        auto wgt = cellWidget(idx.row(), idx.column());
        auto item = (MediaItem*) gMediaTree->currentItem();
        auto lb = (QLabel*) wgt->children().at(2);
        lb->setText(item->text("name"**gMediaTree));
        lb = (QLabel*) wgt->children().at(1);
        lb->setPixmap(QPixmap::fromImage(item->profile));
        auto type = item->text("type"**gMediaTree);
        if(type=="Video") {
            auto wgt = new QGraphicsVideoItem;
            wgt->setAspectRatioMode(Qt::IgnoreAspectRatio);
            auto player = new QMediaPlayer;
            player->setSource(QUrl::fromLocalFile(item->file));
            player->setVideoOutput(wgt);
            player->setAudioOutput(new QAudioOutput);
            connect(wgt, &QGraphicsVideoItem::parentChanged, player, [=]() {
                if(wgt->parent()!=gScene) player->stop();
            });
            setData(idx.row(), idx.column(), (quint64) new Cell('V', wgt, player));
        } else if(type=="Image") {
            QImageReader reader(item->file);
            reader.setAutoTransform(true);
            auto img = reader.read();
            if(img.isNull()) {
                QMessageBox::critical(this, "Image Read Error", QString::number(reader.error())+" "+reader.errorString());
                return;
            }
            auto wgt = new ImgItem;
            wgt->img = QPixmap::fromImage(img);
            setData(idx.row(), idx.column(), (quint64) new Cell('I', wgt, 0));
        }
        event->accept();
    }
    end: QTableWidget::dropEvent(event);
}
void PlanTableH::dropEvent(QDropEvent *event) {

    QTableWidget::dropEvent(event);
}
void PlanTableV::dropEvent(QDropEvent *event) {

    QTableWidget::dropEvent(event);
}
