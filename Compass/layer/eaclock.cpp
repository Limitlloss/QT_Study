#include "eaclock.h"
#include "main.h"
#include "gutil/qgui.h"
#include "tools.h"
#include "base/locolorselector.h"
#include <QtMath>
#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QSpinBox>
#include <QFontComboBox>
#include <QToolButton>

EAClock::EAClock(EBase *multiWin) : EBase(multiWin) {
    mType = EBase::AClock;
    m_attr.timeZone = QTimeZone::systemTimeZone();
    m_attr.hourHandColor = Qt::yellow;
    m_attr.minHandColor = Qt::green;
    m_attr.secHandColor = Qt::red;
    m_attr.textColor = Qt::red;
}

EAClock::EAClock(const JObj &json, EBase *multiWin) : EBase(multiWin) {
    mType = EBase::AClock;
    setBaseAttr(json);
    auto widget = json["widget"];
    if(widget.isNull()) widget = json;
    m_attr.timeZone = QTimeZone(widget["timeZone"].toString().toUtf8());
    m_attr.hourMark = widget["hourMark"].toInt();
    m_attr.hourMarkSize = widget["hourMarkSize"].toInt();
    auto color = widget["hourMarkColor"];
    m_attr.hourMarkColor = color.isStr() ? QColor(color.toString()) : Tools::int2Color(color.toInt());
    m_attr.minMark = widget["minMark"].toInt();
    m_attr.minMarkSize = widget["minMarkSize"].toInt();
    color = widget["minMarkColor"];
    m_attr.minMarkColor = color.isStr() ? QColor(color.toString()) : Tools::int2Color(color.toInt());
    color = widget["hourHandColor"];
    m_attr.hourHandColor = color.isStr() ? QColor(color.toString()) : Tools::int2Color(color.toInt());
    color = widget["minHandColor"];
    m_attr.minHandColor = color.isStr() ? QColor(color.toString()) : Tools::int2Color(color.toInt());
    color = widget["secHandColor"];
    m_attr.secHandColor = color.isStr() ? QColor(color.toString()) : Tools::int2Color(color.toInt());
    color = widget["textColor"];
    m_attr.textColor = color.isStr() ? QColor(color.toString()) : Tools::int2Color(color.toInt());
    m_attr.text = widget["text"].toString();
    m_attr.textFont = QFont(widget["textFontFamily"].toString());
    m_attr.textFont.setPixelSize(widget["textFontSize"].toInt());
    m_attr.textFont.setBold(widget["textFontBold"].toBool());
    m_attr.textFont.setItalic(widget["textFontItalics"].toBool());
    m_attr.textFont.setUnderline(widget["textFontUnderline"].toBool());
    m_attr.hhLen = widget["hhLen"].toInt(m_attr.hhLen);
    m_attr.mhLen = widget["mhLen"].toInt(m_attr.mhLen);
    m_attr.shLen = widget["shLen"].toInt(m_attr.shLen);
    m_attr.hhWidth = widget["hhWidth"].toInt(m_attr.hhWidth);
    m_attr.mhWidth = widget["mhWidth"].toInt(m_attr.mhWidth);
    m_attr.shWidth = widget["shWidth"].toInt(m_attr.shWidth);
    m_attr.showSecHand = widget["showSecHand"].toBool(true);
    m_attr.path = widget["path"].toString();
    m_attr.name = widget["name"].toString();
    m_attr.hasDialImg  = widget["bCustomDial"].toBool();
    if(m_attr.hasDialImg) dial_img.load(m_attr.path+"/"+m_attr.name);
}

void EAClock::timerEvent(QTimerEvent *) {
    if(isVisible()) {
        cal();
        update();
    } else if(timerId) {
        killTimer(timerId);
        timerId = 0;
    }
}
void EAClock::cal() {
    auto time = QDateTime::currentDateTime().toTimeZone(m_attr.timeZone).time();
    sAngle = time.second() * 6;
    mAngle = time.minute() * 6 + sAngle/60;
    hAngle = time.hour() * 30 + mAngle/12;
}
void EAClock::paintDial(QPainter *painter) {
    if(! m_attr.hasDialImg || dial_img.isNull()) {
        auto inner = innerRect();
        auto r = (qMin(inner.width(), inner.height()) - qMax(m_attr.minMarkSize, m_attr.hourMarkSize)) / 2;
        auto cx = inner.width() / 2;
        auto cy = inner.height() / 2;
        painter->translate(cx, cy);
        for(int i=0; i<60; i++) {
            auto k = i * M_PI / 30;
            auto x = sin(k) * r;
            auto y = -cos(k) * r;
            if(i % 5) {
                if(m_attr.minMark==0) drawMarkCircular(painter, {x, y}, m_attr.minMarkColor, m_attr.minMarkSize);
                else if(m_attr.minMark==1) drawMarkRectangle(painter, {x, y}, m_attr.minMarkColor, m_attr.minMarkSize, i*6);
            } else {
                if(m_attr.hourMark==0) drawMarkCircular(painter, {x, y}, m_attr.hourMarkColor, m_attr.hourMarkSize);
                else if(m_attr.hourMark==1) drawMarkRectangle(painter, {x, y}, m_attr.hourMarkColor, m_attr.hourMarkSize, i*6);
                else if(m_attr.hourMark==2) drawMarkNumber(painter, {x, y}, m_attr.hourMarkColor, m_attr.hourMarkSize, i/5);
            }
        }
        painter->translate(-cx, -cy);
    } else {
        auto inner = innerRect();
        int wid, hei;
        double rate = qMin(inner.width() / dial_img.width(), inner.height() / dial_img.height());
        wid = dial_img.width() * rate;
        hei = dial_img.height() * rate;
        painter->drawImage(QRectF((inner.width() - wid)/2, (inner.height() - hei)/2, wid, hei), dial_img, QRectF());
    }
}

void EAClock::drawMarkCircular(QPainter *painter, const QPointF &pos, const QColor &color, qreal diameter) {
    auto r = diameter / 2;
    painter->setBrush(color);
    painter->setPen(color);
    painter->drawEllipse(pos, r, r);
}

void EAClock::drawMarkRectangle(QPainter *painter, const QPointF &pos, const QColor &color, qreal len, qreal angle) {
    QRectF rect(-len/2, -len/2, len, len);
    painter->save();
    painter->setBrush(color);
    painter->setPen(color);
    painter->translate(pos);
    painter->rotate(angle);
    painter->drawRect(rect);
    painter->restore();
}

void EAClock::drawMarkNumber(QPainter *painter, const QPointF &pos, const QColor &color, qreal len, int num) {
    QRectF rect(pos.x()-len/2, pos.y()-len/2, len, len);
    QFont font("Arial");
    font.setPixelSize(round(len));
    painter->setFont(font);
    painter->setPen(color);
    painter->drawText(rect, QString::number(num==0 ? 12 : num), QTextOption(Qt::AlignCenter));
}

void EAClock::paintText(QPainter *painter){
    if(m_attr.text.isNull() || m_attr.text.isEmpty()) return;
    auto inner = innerRect();
    painter->setPen(m_attr.textColor);
    m_attr.textFont.setStyleStrategy(gTextAntialiasing ? QFont::PreferAntialias : QFont::NoAntialias);
    painter->setFont(m_attr.textFont);
    painter->drawText(QRectF(0, inner.height()/6, inner.width(), inner.height()/4), m_attr.text, QTextOption(Qt::AlignCenter));
}

void EAClock::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    if(timerId==0) {
        timerId = startTimer(500, Qt::PreciseTimer);
        cal();
    }
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing|QPainter::SmoothPixmapTransform);
    auto inner = innerRect();
    auto sideLen = qMin(inner.width(), inner.height());
    auto lineWidth = sideLen/128;
    if(lineWidth < 1) lineWidth = 1;
    painter->translate(inner.x(), inner.y());
    paintDial(painter);
    paintText(painter);
    painter->translate(inner.width() / 2, inner.height() / 2);

    QPen pen(m_attr.hourHandColor, lineWidth);
    pen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(pen);
    painter->setBrush(pen.brush());
    painter->rotate(hAngle);
    QPointF poses[3]{{m_attr.hhWidth*sideLen/400, 0}, {0, m_attr.hhLen*sideLen/-200}};
    poses[2].rx() = -poses[0].rx();
    painter->drawPolygon(poses, 3);

    pen.setColor(m_attr.minHandColor);
    painter->setPen(pen);
    painter->setBrush(pen.brush());
    painter->rotate(mAngle-hAngle);
    poses[0].rx() = m_attr.mhWidth*sideLen/400;
    poses[1].ry() = m_attr.mhLen*sideLen/-200;
    poses[2].rx() = -poses[0].rx();
    painter->drawPolygon(poses, 3);
    if(m_attr.showSecHand) {
        pen.setColor(m_attr.secHandColor);
        painter->setPen(pen);
        painter->setBrush(pen.brush());
        painter->rotate(sAngle-mAngle);
        poses[0].rx() = m_attr.shWidth*sideLen/400;
        poses[1].ry() = m_attr.shLen*sideLen/-200;
        poses[2].rx() = -poses[0].rx();
        painter->drawPolygon(poses, 3);
    }
    painter->restore();
    EBase::paint(painter, option, widget);
}

QWidget* EAClock::attrWgt() {
    auto wgtAttr = new QWidget();

    auto vBox = new VBox(wgtAttr);
    vBox->setContentsMargins(6, 0, 6, 0);
    if(mMultiWin) vBox->setSpacing(3);

    addBaseAttrWgt(vBox);

    auto hBox = new HBox(vBox);
    hBox->addWidget(new QLabel(translate("","Basic Properties")));

    auto line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    hBox->addWidget(line, 1);


    hBox = new HBox(vBox);
    hBox->addSpacing(6);
    hBox->addWidget(new QLabel(tr("Time Zone")));

    auto edTimeZone = new QComboBox;
    auto zoneIds = QTimeZone::availableTimeZoneIds();
    for(auto &zoneId : zoneIds) edTimeZone->addItem(zoneId);
    edTimeZone->setCurrentText(m_attr.timeZone.id());
    connect(edTimeZone, &QComboBox::currentTextChanged, this, [this](const QString &text) {
        m_attr.timeZone = QTimeZone(text.toUtf8());
        update();
    });
    hBox->addWidget(edTimeZone);
    hBox->addStretch();


    hBox = new HBox(vBox);
    auto fdHasDialImg = new QCheckBox(tr("Custom Dial"));
    fdHasDialImg->setChecked(m_attr.hasDialImg);
    hBox->addWidget(fdHasDialImg);

    line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    hBox->addWidget(line, 1);


    auto wgtDial = new QWidget();
    if(! m_attr.hasDialImg) wgtDial->setVisible(false);
    hBox = new HBox(wgtDial);
    hBox->setContentsMargins(0, 0, 0, 0);
    hBox->addSpacing(6);
    auto fdDialImg = new QLineEdit();
    fdDialImg->setReadOnly(true);
    if(! m_attr.path.isEmpty() && ! m_attr.name.isEmpty()) fdDialImg->setText(m_attr.path+"/"+m_attr.name);
    hBox->addWidget(fdDialImg);

    auto btnSelImg = new QPushButton(tr("Select"));
    btnSelImg->setProperty("ssType", "progManageTool");
    connect(btnSelImg, &QPushButton::clicked, wgtAttr, [=] {
        QString fileName = QFileDialog::getOpenFileName(wgtAttr, tr("Select Dail file"), QApplication::applicationDirPath()+"/AClock/", "Dail files(*.png)");
        if(fileName.isEmpty()) return;
        fdDialImg->setText(fileName);
        QFileInfo info(fileName);
        if(info.exists()) {
            m_attr.name = info.fileName();
            m_attr.path = info.absolutePath();
            dial_img.load(info.absoluteFilePath());
            update();
        }
    });
    hBox->addWidget(btnSelImg);

    vBox->addWidget(wgtDial);

    auto wgtMarks = new QWidget();
    if(m_attr.hasDialImg) wgtMarks->setVisible(false);

    connect(fdHasDialImg, &QCheckBox::toggled, this, [this, wgtDial, wgtMarks, fdDialImg](bool checked) {
        m_attr.hasDialImg = checked;
        wgtDial->setVisible(checked);
        wgtMarks->setVisible(!checked);
        if(checked) {
            auto text = fdDialImg->text();
            if(! text.isEmpty()) {
                QFileInfo info(text);
                if(info.exists()) {
                    m_attr.name = info.fileName();
                    m_attr.path = info.absolutePath();
                    dial_img.load(info.absoluteFilePath());
                }
            }
        }
        update();
    });

    auto vbMarks = new VBox(wgtMarks);
    vbMarks->setContentsMargins(0, 0, 0, 0);

    hBox = new HBox(vbMarks);
    hBox->addSpacing(6);
    hBox->addWidget(new QLabel(tr("Hour Mark")));

    auto fdHourMark = new QComboBox();
    fdHourMark->addItem(tr("Circular"));
    fdHourMark->addItem(tr("Rectangle"));
    fdHourMark->addItem(tr("Number"));
    fdHourMark->setCurrentIndex(m_attr.hourMark);
    connect(fdHourMark, (void(QComboBox::*)(int))&QComboBox::currentIndexChanged, this, [this](int index) {
        m_attr.hourMark = index;
        update();
    });
    hBox->addWidget(fdHourMark);

    auto fdHourMarkSize = new QSpinBox();
    fdHourMarkSize->setMaximum(9999);
    fdHourMarkSize->setValue(m_attr.hourMarkSize);
    connect(fdHourMarkSize, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, [this](int value) {
        m_attr.hourMarkSize = value;
        update();
    });
    hBox->addWidget(fdHourMarkSize);

    auto fdHourMarkColor = new LoColorSelector("T", m_attr.hourMarkColor);
    fdHourMarkColor->setFixedWidth(30);
    connect(fdHourMarkColor, &LoColorSelector::sColorChanged, this, [this](const QColor &color) {
        m_attr.hourMarkColor = color;
        update();
    });
    hBox->addWidget(fdHourMarkColor);
    hBox->addStretch();


    hBox = new HBox(vbMarks);
    hBox->addSpacing(6);
    hBox->addWidget(new QLabel(tr("Min Mark")));

    auto fdMinMark = new QComboBox();
    fdMinMark->addItem(tr("Circular"));
    fdMinMark->addItem(tr("Rectangle"));
    fdMinMark->setCurrentIndex(m_attr.minMark);
    connect(fdMinMark, (void(QComboBox::*)(int))&QComboBox::currentIndexChanged, this, [this](int index) {
        m_attr.minMark = index;
        update();
    });
    hBox->addWidget(fdMinMark);

    auto fdMinMarkSize = new QSpinBox();
    fdMinMarkSize->setMaximum(9999);
    fdMinMarkSize->setValue(m_attr.minMarkSize);
    connect(fdMinMarkSize, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, [this](int value) {
        m_attr.minMarkSize = value;
        update();
    });
    hBox->addWidget(fdMinMarkSize);

    auto fdMinMarkColor = new LoColorSelector("T", m_attr.minMarkColor);
    fdMinMarkColor->setFixedWidth(30);
    connect(fdMinMarkColor, &LoColorSelector::sColorChanged, this, [this](const QColor &color) {
        m_attr.minMarkColor = color;
        update();
    });
    hBox->addWidget(fdMinMarkColor);
    hBox->addStretch();

    vBox->addWidget(wgtMarks);

    auto grid = new QGridLayout;
    grid->setColumnStretch(4, 1);
    vBox->addLayout(grid);

    grid->addWidget(new QLabel(tr("Color")), 0, 1);
    grid->addWidget(new QLabel(tr("Length")+" (%)"), 0, 2);
    grid->addWidget(new QLabel(tr("Width")+" (%)"), 0, 3);

    grid->addWidget(new QLabel(tr("Hour Hand")), 1, 0, Qt::AlignRight);

    auto fdHourHandColor = new LoColorSelector("", m_attr.hourHandColor);
    fdHourHandColor->setFixedWidth(30);
    connect(fdHourHandColor, &LoColorSelector::sColorChanged, this, [this](const QColor &color) {
        m_attr.hourHandColor = color;
        update();
    });
    grid->addWidget(fdHourHandColor, 1, 1);

    auto hhLen = new QSpinBox;
    hhLen->setRange(0, 999);
    hhLen->setValue(m_attr.hhLen);
    connect(hhLen, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, [this](int value) {
        m_attr.hhLen = value;
        update();
    });
    grid->addWidget(hhLen, 1, 2);

    auto hhWidth = new QSpinBox;
    hhWidth->setRange(0, 999);
    hhWidth->setValue(m_attr.hhWidth);
    connect(hhWidth, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, [this](int value) {
        m_attr.hhWidth = value;
        update();
    });
    grid->addWidget(hhWidth, 1, 3);

    grid->addWidget(new QLabel(tr("Min Hand")), 2, 0, Qt::AlignRight);

    auto fdMinHandColor = new LoColorSelector("", m_attr.minHandColor);
    fdMinHandColor->setFixedWidth(30);
    connect(fdMinHandColor, &LoColorSelector::sColorChanged, this, [this](const QColor &color) {
        m_attr.minHandColor = color;
        update();
    });
    grid->addWidget(fdMinHandColor, 2, 1);

    auto mhLen = new QSpinBox;
    mhLen->setRange(0, 999);
    mhLen->setValue(m_attr.mhLen);
    connect(mhLen, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, [this](int value) {
        m_attr.mhLen = value;
        update();
    });
    grid->addWidget(mhLen, 2, 2);

    auto mhWidth = new QSpinBox;
    mhWidth->setRange(0, 999);
    mhWidth->setValue(m_attr.mhWidth);
    connect(mhWidth, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, [this](int value) {
        m_attr.mhWidth = value;
        update();
    });
    grid->addWidget(mhWidth, 2, 3);

    grid->addWidget(new QLabel(tr("Sec Hand")), 3, 0, Qt::AlignRight);

    auto fdSecHandColor = new LoColorSelector("", m_attr.secHandColor);
    fdSecHandColor->setFixedWidth(30);
    connect(fdSecHandColor, &LoColorSelector::sColorChanged, this, [this](const QColor &color) {
        m_attr.secHandColor = color;
        update();
    });
    grid->addWidget(fdSecHandColor, 3, 1);

    auto shLen = new QSpinBox;
    shLen->setRange(0, 999);
    shLen->setValue(m_attr.shLen);
    connect(shLen, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, [this](int value) {
        m_attr.shLen = value;
        update();
    });
    grid->addWidget(shLen, 3, 2);

    auto shWidth = new QSpinBox;
    shWidth->setRange(0, 999);
    shWidth->setValue(m_attr.shWidth);
    connect(shWidth, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, [this](int value) {
        m_attr.shWidth = value;
        update();
    });
    grid->addWidget(shWidth, 3, 3);

    auto fdShowSecHand = new QCheckBox(tr("Show"));
    fdShowSecHand->setChecked(m_attr.showSecHand);
    connect(fdShowSecHand, &QCheckBox::toggled, this, [this](bool checked) {
        m_attr.showSecHand = checked;
        update();
    });
    grid->addWidget(fdShowSecHand, 3, 4);


    line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    vBox->addWidget(line);

    auto fdText = new QTextEdit(m_attr.text);
    fdText->setMaximumHeight(60);
    fdText->setPlaceholderText(tr("Text"));
    connect(fdText, &QTextEdit::textChanged, this, [this, fdText] {
        m_attr.text = fdText->toPlainText();
    });
    vBox->addWidget(fdText);

    hBox = new HBox(vBox);
    hBox->addSpacing(6);

    auto fdFontFamily = new QFontComboBox();
    fdFontFamily->setEditable(false);
    fdFontFamily->setCurrentText(m_attr.textFont.family());
    connect(fdFontFamily, &QFontComboBox::currentFontChanged, this, [this](const QFont &font) {
        QFont newFont(font.family());
        newFont.setPixelSize(m_attr.textFont.pixelSize());
        newFont.setBold(m_attr.textFont.bold());
        newFont.setItalic(m_attr.textFont.italic());
        newFont.setUnderline(m_attr.textFont.underline());
        m_attr.textFont = newFont;
        update();
    });
    hBox->addWidget(fdFontFamily);

    auto fdFontSize = new QSpinBox();
    fdFontSize->setRange(4, 9999);
    fdFontSize->setValue(m_attr.textFont.pixelSize());
    connect(fdFontSize, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, [this](int value) {
        m_attr.textFont.setPixelSize(value);
        update();
    });
    hBox->addWidget(fdFontSize);
    hBox->addStretch();


    hBox = new HBox(vBox);
    hBox->addSpacing(6);

    auto fdBold = new QPushButton("B");
    fdBold->setStyleSheet("QPushButton{background: #bbb; color: #888; font-size: 20px; font-weight: bold;} QPushButton:checked{background: #29c; color: #fff;}");
    fdBold->setFixedSize(30, 30);
    fdBold->setCheckable(true);
    fdBold->setChecked(m_attr.textFont.bold());
    connect(fdBold, &QPushButton::toggled, this, [this](bool checked) {
        m_attr.textFont.setBold(checked);
        update();
    });
    hBox->addWidget(fdBold);

    auto fdItalic = new QPushButton("I");
    fdItalic->setStyleSheet("QPushButton{background: #bbb; color: #888; font-size: 20px; font-style: italic;} QPushButton:checked{background: #29c; color: #fff;}");
    fdItalic->setFixedSize(30, 30);
    fdItalic->setCheckable(true);
    fdItalic->setChecked(m_attr.textFont.italic());
    connect(fdItalic, &QPushButton::toggled, this, [this](bool checked) {
        m_attr.textFont.setItalic(checked);
        update();
    });
    hBox->addWidget(fdItalic);

    auto fdUnderline = new QPushButton("U");
    fdUnderline->setStyleSheet("QPushButton{background: #bbb; color: #888; font-size: 20px; text-decoration: underline;} QPushButton:checked{background: #29c; color: #fff;}");
    fdUnderline->setFixedSize(30, 30);
    fdUnderline->setCheckable(true);
    fdUnderline->setChecked(m_attr.textFont.underline());
    connect(fdUnderline, &QPushButton::toggled, this, [this](bool checked) {
        m_attr.textFont.setUnderline(checked);
        update();
    });
    hBox->addWidget(fdUnderline);

    auto fdTextColor = new LoColorSelector("T", m_attr.textColor);
    fdTextColor->setFixedWidth(30);
    connect(fdTextColor, &LoColorSelector::sColorChanged, this, [this](const QColor &color) {
        m_attr.textColor = color;
        update();
    });
    hBox->addWidget(fdTextColor);
    hBox->addStretch();

    vBox->addStretch();
    return wgtAttr;
}

bool EAClock::save(const QString &pRoot){
    if(m_attr.hasDialImg) {
        auto old_file = m_attr.path + PAGEDEL_SUFFIX + "/" + m_attr.name;
        if(! QFileInfo::exists(old_file)) {
            old_file = m_attr.path + "/" + m_attr.name;
            if(! QFileInfo::exists(old_file)) return false; //自己画点，不用背景表盘
        }
        auto new_file = pRoot + "/" + m_attr.name;
        if(! QFileInfo::exists(new_file)) QFile(old_file).copy(new_file);
    } else {
        auto filename = QString("%1-%2-%3-%4-%5.png").arg((int)zValue()).arg((int)x()).arg((int)y()).arg((int)mWidth).arg((int)mHeight);
        m_attr.path = pRoot;
        QRectF inner = innerRect();
        QImage img(inner.width(), inner.height(), QImage::Format_ARGB32);
        img.fill(Qt::transparent);
        {
            QPainter painter(&img);
            paintDial(&painter);
            paintText(&painter);
        }
        img.save(pRoot+"/"+filename, "PNG");
    }
    return true;
}
JObj EAClock::attrJson() const {
    JObj json;
    json["elementType"] = "AClock";
    json["timeZone"] = QString::fromUtf8(m_attr.timeZone.id());
    json["hourMark"]          = m_attr.hourMark;
    json["hourMarkSize"]      = m_attr.hourMarkSize;
    json["hourMarkColor"]     = m_attr.hourMarkColor.name();
    json["minMark"]           = m_attr.minMark;
    json["minMarkSize"]       = m_attr.minMarkSize;
    json["minMarkColor"]      = m_attr.minMarkColor.name();
    json["hourHandColor"]     = m_attr.hourHandColor.name();
    json["minHandColor"]      = m_attr.minHandColor.name();
    json["secHandColor"]      = m_attr.secHandColor.name();
    json["text"]              = m_attr.text;
    json["textFontFamily"]    = m_attr.textFont.family();
    json["textFontSize"]      = m_attr.textFont.pixelSize();
    json["textFontBold"]      = m_attr.textFont.bold();
    json["textFontItalics"]   = m_attr.textFont.italic();
    json["textFontUnderline"] = m_attr.textFont.underline();
    json["textColor"] = m_attr.textColor.name();
    json["hhLen"] = m_attr.hhLen;
    json["mhLen"] = m_attr.mhLen;
    json["shLen"] = m_attr.shLen;
    json["hhWidth"] = m_attr.hhWidth;
    json["mhWidth"] = m_attr.mhWidth;
    json["shWidth"] = m_attr.shWidth;
    json["showSecHand"] = m_attr.showSecHand;
    json["path"] = m_attr.path;
    json["name"] = m_attr.name;
    json["selfCreateDialName"] = QString("%1-%2-%3-%4-%5.png").arg((int)zValue()).arg((int)x()).arg((int)y()).arg((int)mWidth).arg((int)mHeight);
    json["bCustomDial"] = m_attr.hasDialImg;
    addBaseAttr(json);
    return json;
}
