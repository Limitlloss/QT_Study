#include "eweb.h"
#include "gutil/qgui.h"
#include "main.h"
#include <QSpinBox>
#include <QLineEdit>
#include <QPainter>

EWeb::EWeb(EBase *multiWin) : EBase(multiWin) {
    mType = EBase::Web;
}
EWeb::EWeb(const JObj &json, EBase *multiWin) : EBase(multiWin) {
    mType = EBase::Web;
    setBaseAttr(json);
    url = json["url"].toString();
    zoom = json["zoom"].toInt(100);
    refresh = json["refreshSec"].toInt();
    _x = json["offX"].toInt();
    _y = json["offY"].toInt();
    scaleX = json["scaleX"].toInt(100);
    scaleY = json["scaleY"].toInt(100);
}

void EWeb::paint(QPainter *painter, const QStyleOptionGraphicsItem *a, QWidget *b) {
    auto inner = innerRect();
    painter->setRenderHints(QPainter::Antialiasing|QPainter::SmoothPixmapTransform);
    double maskW = holder().width(), maskH = holder().height();
    if(maskW>inner.width() || maskH>inner.height()) {
        double rate = qMin(inner.width() / maskW, inner.height() / maskH);
        maskW *= rate;
        maskH *= rate;
    }
    painter->drawImage(QRectF((inner.width() - maskW)/2, (inner.height() - maskH)/2, maskW, maskH), holder());
    EBase::paint(painter, a, b);
}

QWidget* EWeb::attrWgt() {
    auto wgtAttr = new QWidget;
    auto vBox = new VBox(wgtAttr);
    vBox->setContentsMargins(6, 0, 6, 0);
    if(mMultiWin) vBox->setSpacing(3);

    addBaseAttrWgt(vBox);

    auto hBox = new HBox(vBox);
    hBox->addLabel(translate("","Basic Properties"));

    auto line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    hBox->addWidget(line, 1);

    hBox = new HBox(vBox);
    hBox->addSpacing(6);
    hBox->addLabel("URL:");

    auto url_fd = new QLineEdit(url);
    hBox->addWidget(url_fd);
    connect(url_fd, &QLineEdit::textChanged, this, [this](const QString &text) {
        url = text;
    });

    hBox = new HBox(vBox);
    auto lb = hBox->addLabel(tr("Zoom")+":");
    lb->setMinimumWidth(70);
    lb->setAlignment(Qt::AlignVCenter|Qt::AlignRight);

    auto fdZoom = new QSpinBox;
    fdZoom->setRange(1, 99999);
    fdZoom->setValue(zoom);
    connect(fdZoom, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, [=](int value) {
        zoom = value;
    });
    hBox->addWidget(fdZoom);
    hBox->addSpacing(-3);
    hBox->addLabel("%");
    hBox->addStretch();

    hBox = new HBox(vBox);
    lb = hBox->addLabel(tr("Refresh every")+":");
    lb->setMinimumWidth(70);
    lb->setAlignment(Qt::AlignVCenter|Qt::AlignRight);

    auto edRefresh = new QSpinBox;
    edRefresh->setRange(0, 99999);
    edRefresh->setValue(refresh);
    connect(edRefresh, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, [=](int value) {
        refresh = value;
    });
    hBox->addWidget(edRefresh);
    hBox->addSpacing(-3);
    hBox->addLabel("s");
    hBox->addStretch();

    hBox = new HBox(vBox);
    lb = hBox->addLabel(tr("Offset")+" X:");
    lb->setMinimumWidth(70);
    lb->setAlignment(Qt::AlignVCenter|Qt::AlignRight);

    auto fdX = new QSpinBox;
    fdX->setRange(0, 99999);
    fdX->setValue(_x);
    connect(fdX, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, [=](int value) {
        _x = value;
    });
    hBox->addWidget(fdX);
    hBox->addSpacing(-3);
    hBox->addLabel()->setMinimumWidth(30);

    hBox->addLabel("Y:");
    auto fdY = new QSpinBox;
    fdY->setRange(0, 99999);
    fdY->setValue(_y);
    connect(fdY, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, [=](int value) {
        _y = value;
    });
    hBox->addWidget(fdY);
    hBox->addStretch();


    hBox = new HBox(vBox);
    lb = hBox->addLabel(tr("Scale")+" X:");
    lb->setMinimumWidth(70);
    lb->setAlignment(Qt::AlignVCenter|Qt::AlignRight);

    auto fdScaleX = new QSpinBox;
    fdScaleX->setRange(-9999, 99999);
    fdScaleX->setValue(scaleX);
    connect(fdScaleX, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, [=](int value) {
        scaleX = value;
    });
    hBox->addWidget(fdScaleX);
    hBox->addSpacing(-3);
    hBox->addLabel("%")->setMinimumWidth(30);

    hBox->addLabel("Y:");
    auto fdScaleY = new QSpinBox;
    fdScaleY->setRange(-9999, 99999);
    fdScaleY->setValue(scaleY);
    connect(fdScaleY, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, [=](int value) {
        scaleY = value;
    });
    hBox->addWidget(fdScaleY);
    hBox->addSpacing(-3);
    hBox->addLabel("%");
    hBox->addStretch();

    vBox->addStretch();
    return wgtAttr;
}

JObj EWeb::attrJson() const {
    JObj obj;
    addBaseAttr(obj);
    obj["elementType"] = "Web";
    obj["url"] = url;
    obj["zoom"] = zoom;
    obj["refreshSec"] = refresh;
    obj["offX"] = _x;
    obj["offY"] = _y;
    obj["scaleX"] = scaleX;
    obj["scaleY"] = scaleY;
    return obj;
}
