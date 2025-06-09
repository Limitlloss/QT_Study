#include "outputpanel.h"
#include "main.h"
#include "layer.h"
#include <QApplication>
#include <QMessageBox>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>

OutputPanel::OutputPanel(QWidget *parent) : QWidget(parent) {
    gOutPanel = this;
    auto vBox = new VBox(this);
    vBox->setContentsMargins(0, 0, 0, 0);
    vBox->setSpacing(0);
    auto hBox = new HBox(vBox);

    auto bnAdd = new QPushButton("🞥");
    bnAdd->setMaximumWidth(50);
    hBox->addWidget(bnAdd);
    connect(bnAdd, &QPushButton::clicked, this, [=] {
        QString name;
        auto cnt = tree->topLevelItemCount();
        for(int i=1;;i++) {
            name = QString("Output %1").arg(i);
            for(int r=0; r<cnt; ++r) if(tree->topLevelItem(r)->text("name"**tree)==name) goto conti;
            break;
            conti:;
        }
        auto out = new Layer(0, name, gEditView);
        gEditView->layers.push_back(out);
        out->updateGeo();
        out->show();
        out->item = new OutputItem(tree);
        out->item->setText("name", out->name);
        out->item->setText("size", QString("%1×%2").arg(out->sSize.width()).arg(out->sSize.height()));
        out->item->setData(0, (quint64) out);
        tree->setCurrentItem(out->item);
        out->view = new OutputView;
        out->view->setGeometry({QPoint(), out->sSize});
        out->view->setSceneRect({out->sPos, out->sSize});
    });

    auto bnDelet = new QPushButton("🗑");
    bnDelet->setMaximumWidth(50);
    hBox->addWidget(bnDelet);
    connect(bnDelet, &QPushButton::clicked, this, [=] {
        // for(int i=0; i<tree->topLevelItemCount(); i++) if(tree->item(i)->checkState("check") == Qt::Checked) {
        //     auto item = (MediaItem*) tree->topLevelItem(i--);
        //     item->del();
        //     delete item;
        // }
    });

    hBox->addStretch();

    tree = new TreeWidget;
    tree->addCol("#", "", 20);
    tree->addCol("name", "", 140);
    tree->addCol("size", "", 120);
    tree->setDefs()->setHeaderAlignC();
    tree->minRowHeight = 26;
    connect(tree, &TreeWidget::currentItemChanged, this, [=](QTreeWidgetItem *current, QTreeWidgetItem *) {
        if(enCurChanged) gEditView->select((Layer*) current->data(0, Qt::UserRole).toULongLong());
    });
    vBox->addWidget(tree);

    vBox->addSpacing(8);
    vBox->addLabel("Output Area Properties");
    vBox->addSpacing(6);

    auto grid = new Grid(vBox);
    grid->setHorizontalSpacing(6);
    int r = 0;
    grid->addLabel("Name", r, 0);
    edName = new QLineEdit;
    grid->addWidget(edName, r, 1);
    r++;

    grid->addLabel("Position", r, 0);

    hBox = new HBox;
    hBox->addLabel("X:")->setMinimumWidth(20);

    edX = new QSpinBox;
    edX->setRange(-99999, 999999);
    connect(edX, &QSpinBox::valueChanged, this, [=](int value) {
        auto layer = gEditView->selected;
        if(! layer) return;
        layer->sPos.rx() = value;
        layer->view->move(layer->sPos);
        layer->updateGeo();
        layer->update();
    });
    hBox->addWidget(edX);

    hBox->addSpacing(10);

    hBox->addLabel("Y:")->setMinimumWidth(20);
    edY = new QSpinBox;
    edY->setRange(-99999, 999999);
    edY->setValue(y());
    connect(edY, &QSpinBox::valueChanged, this, [=](int value) {
        auto layer = gEditView->selected;
        if(! layer) return;
        layer->sPos.ry() = value;
        layer->view->move(layer->sPos);
        layer->updateGeo();
        layer->update();
    });
    hBox->addWidget(edY);
    hBox->addStretch();
    grid->addLayout(hBox, r, 1);
    r++;

    grid->addLabel("Size", r, 0);
    hBox = new HBox;
    hBox->addLabel("W:")->setMinimumWidth(20);

    edW = new QSpinBox;
    edW->setRange(10, 999999);
    connect(edW, &QSpinBox::valueChanged, this, [=](int value) {
        auto layer = gEditView->selected;
        if(! layer || ! layer->item) return;
        layer->sSize.rwidth() = value;
        layer->view->resize(layer->sSize);
        layer->updateGeo();
        layer->update();
        layer->item->setText("size"**tree, QString("%1×%2").arg(layer->sSize.rwidth()).arg(layer->sSize.rheight()));
    });
    hBox->addWidget(edW);

    hBox->addSpacing(10);
    hBox->addLabel("H:")->setMinimumWidth(20);
    edH = new QSpinBox;
    edH->setRange(10, 999999);
    connect(edH, &QSpinBox::valueChanged, this, [=](int value) {
        auto layer = gEditView->selected;
        if(! layer || ! layer->item) return;
        layer->sSize.rheight() = value;
        layer->view->resize(layer->sSize);
        layer->updateGeo();
        layer->update();
        layer->item->setText("size"**tree, QString("%1×%2").arg(layer->sSize.rwidth()).arg(layer->sSize.rheight()));
    });
    hBox->addWidget(edH);
    hBox->addStretch();
    grid->addLayout(hBox, r, 1);
    r++;

    auto edShow = new QCheckBox("Open");
    connect(edShow, &QCheckBox::checkStateChanged, this, [=](Qt::CheckState checkState) {
        auto area = gEditView->selected;
        if(! area || ! area->view) return;
        area->view->setVisible(checkState==Qt::Checked);
    });
    grid->addWidget(edShow, r, 1);
    r++;

    transUi();
}

void OutputPanel::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    for(auto layer : gEditView->layers) if(layer->item) layer->raise();
    auto item = tree->curItem();
    if(item) gEditView->select((Layer*) item->data(0).toULongLong());
    qDebug()<<"showEvent";
}
void OutputPanel::hideEvent(QHideEvent *event) {
    QWidget::hideEvent(event);
    for(auto layer : gEditView->layers) if(layer->item) layer->stackUnder(gEditView->originWgt);
    qDebug()<<"hideEvent";
}
void OutputPanel::changeEvent(QEvent *event) {
    QWidget::changeEvent(event);
    if(event->type() == QEvent::LanguageChange) transUi();
}
void OutputPanel::transUi() {
    tree->headerItem()->setText("name"**tree, tr("Name"));
    tree->headerItem()->setText("size"**tree, tr("Size"));
}
