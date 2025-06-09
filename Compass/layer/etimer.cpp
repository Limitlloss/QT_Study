#include "etimer.h"
#include "main.h"
#include "base/calendarbutton.h"
#include "base/locolorselector.h"
#include "gutil/qgui.h"
#include <QRadioButton>
#include <QCheckBox>
#include <QButtonGroup>
#include <QTextEdit>
#include <QDateTimeEdit>
#include <QFontComboBox>
#include <QSpinBox>
#include <QTextOption>
#include <QPainter>
#include <QToolButton>

ETimer::ETimer(EBase *multiWin) : EBase(multiWin) {
    mType = EBase::Timer;
    isDown = true;
    targetTime = QDateTime::currentDateTime();
    hasDay  = true;
    hasHour = true;
    hasMin  = true;
    hasSec  = true;
    isMultiline = true;
    font = qfont("Arial", 12);
    textColor = Qt::red;
    backColor = Qt::transparent;
    init();
}

ETimer::ETimer(const JObj &json, EBase *multiWin) : EBase(multiWin){
    mType = EBase::Timer;
    setBaseAttr(json);
    isDown = json["isDown"].toBool();
    targetTime = QDateTime::fromString(json["targetTime"].toString(), "yyyy-MM-dd HH:mm:ss");
    hasDay = json["hasDay"].toBool();
    hasHour = json["hasHour"].toBool();
    hasMin = json["hasMin"].toBool();
    hasSec = json["hasSec"].toBool();
    text = json["text"].toString();
    isMultiline = json["isMultiline"].toBool();
    font = qfont(json["font"].toString(), json["fontSize"].toInt(), json["fontBold"].toBool(), json["fontItalic"].toBool());
    font.setUnderline(json["fontUnderline"].toBool());
    textColor = json["textColor"].toString();
    backColor = json["backColor"].toString("#00000000");
    init();
}

void ETimer::init() {
    connect(gTick, &Tick::secChanged, this, [this](const QDateTime &cur) {
        auto sss = isDown ? cur.secsTo(targetTime) : targetTime.secsTo(cur);
        if(sss < 0) sss = 0;
        if(secs==sss) return;
        secs = sss;
        update();
    });
}

void ETimer::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    QString text;
    if(!this->text.isEmpty()) {
        text += this->text;
        if(this->isMultiline) text += '\n';
        else text += " ";
    }
    int secs = this->secs;
    if(this->hasDay) {
        text.append(QString::number(secs/86400)).append(" ").append(tr("day")).append(" ");
        secs %= 86400;
    }
    if(this->hasHour) {
        text.append(QString::asprintf("%02d ", secs/3600)).append(tr("hour")).append(" ");
        secs %= 3600;
    }
    if(this->hasMin) {
        text.append(QString::asprintf("%02d ", secs/60)).append(tr("min")).append(" ");
        secs %= 60;
    }
    if(this->hasSec) text.append(QString::asprintf("%02d ", secs)).append(tr("sec")).append(" ");
    text = text.trimmed();
    this->font.setStyleStrategy(gTextAntialiasing ? QFont::PreferAntialias : QFont::NoAntialias);
    if(this->backColor.alpha() != 0) painter->fillRect(0, 0, mWidth, mHeight, this->backColor);
    painter->setFont(this->font);
    painter->setPen(this->textColor);
    painter->drawText(innerRect(), text, QTextOption(Qt::AlignCenter));
    EBase::paint(painter, option, widget);
}

QWidget* ETimer::attrWgt() {
    auto wgtAttr = new QWidget();
    auto vBox = new QVBoxLayout(wgtAttr);
    vBox->setContentsMargins(6, 0, 6, 0);
    if(mMultiWin!=nullptr) vBox->setSpacing(3);

    addBaseAttrWgt(vBox);

    auto hBox = new HBox(vBox);
    hBox->addLabel(translate("","Basic Properties"));

    auto line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    hBox->addWidget(line, 1);

    hBox = new HBox(vBox);
    hBox->addStretch();
    auto fdCntDown = new QRadioButton(tr("Count Down"));
    auto fdCntUp = new QRadioButton(tr("Count Up"));
    if(this->isDown) fdCntDown->setChecked(true);
    else fdCntUp->setChecked(true);
    auto cntGroup = new QButtonGroup(wgtAttr);
    cntGroup->addButton(fdCntDown);
    cntGroup->addButton(fdCntUp);
    connect(fdCntDown, &QRadioButton::toggled, this, [this](bool checked) {
        this->isDown = checked;
        update();
    });
    hBox->addWidget(fdCntDown);
    hBox->addStretch();
    hBox->addWidget(fdCntUp);
    hBox->addStretch();

    hBox = new HBox(vBox);
    hBox->addSpacing(6);
    hBox->addLabel(tr("Target Time"));

    auto edTime = new QDateTimeEdit(this->targetTime);
    edTime->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    connect(edTime, &QTimeEdit::dateTimeChanged, this, [this](const QDateTime &dateTime) {
        this->targetTime = dateTime;
        update();
    });
    hBox->addWidget(edTime);

    auto btnTime = new CalendarButton;
    connect(btnTime->calendar, &QCalendarWidget::clicked, edTime, &QDateTimeEdit::setDate);
    hBox->addWidget(btnTime);
    hBox->addStretch();

    hBox = new HBox(vBox);
    hBox->addSpacing(6);

    auto fdHasDay = new QCheckBox(tr("Day"));
    fdHasDay->setChecked(this->hasDay);
    connect(fdHasDay, &QCheckBox::toggled, this, [this](bool checked) {
        this->hasDay = checked;
        update();
    });
    hBox->addWidget(fdHasDay);

    auto fdHasHour = new QCheckBox(tr("Hour"));
    fdHasHour->setChecked(this->hasHour);
    connect(fdHasHour, &QCheckBox::toggled, this, [this](bool checked) {
        this->hasHour = checked;
        update();
    });
    hBox->addWidget(fdHasHour);

    auto fdHasMin = new QCheckBox(tr("Min"));
    fdHasMin->setChecked(this->hasMin);
    connect(fdHasMin, &QCheckBox::toggled, this, [this](bool checked) {
        this->hasMin = checked;
        update();
    });
    hBox->addWidget(fdHasMin);

    auto fdHasSec = new QCheckBox(tr("Sec"));
    fdHasSec->setChecked(this->hasSec);
    connect(fdHasSec, &QCheckBox::toggled, this, [this](bool checked) {
        this->hasSec = checked;
        update();
    });
    hBox->addWidget(fdHasSec);
    hBox->addStretch();

    auto fdIsMultiline = new QCheckBox(tr("Multiline"));
    fdIsMultiline->setChecked(this->isMultiline);
    connect(fdIsMultiline, &QCheckBox::toggled, this, [this](bool checked) {
        this->isMultiline = checked;
        update();
    });
    hBox->addWidget(fdIsMultiline);

    hBox = new HBox(vBox);
    hBox->addSpacing(6);
    hBox->addWidget(new QLabel(tr("Text")));

    auto fdText = new QTextEdit(this->text);
    fdText->setMaximumHeight(50);
    connect(fdText, &QTextEdit::textChanged, this, [this, fdText]() {
        this->text = fdText->toPlainText();
        update();
    });
    hBox->addWidget(fdText);

    hBox = new HBox(vBox);
    hBox->addSpacing(6);

    auto fdFont = new QFontComboBox();
    fdFont->setCurrentText(this->font.family());
    fdFont->setEditable(false);
    connect(fdFont, &QFontComboBox::currentFontChanged, this, [this](const QFont &f) {
        QFont font(f.family());
        font.setPixelSize(this->font.pixelSize());
        font.setBold(this->font.bold());
        font.setItalic(this->font.italic());
        font.setUnderline(this->font.underline());
        this->font = font;
        update();
    });
    hBox->addWidget(fdFont);

    auto fdFontSize = new QSpinBox();
    fdFontSize->setRange(4, 9999);
    fdFontSize->setValue(this->font.pixelSize());
    connect(fdFontSize, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, [this](int value) {
        this->font.setPixelSize(value);
        update();
    });
    hBox->addWidget(fdFontSize);
    hBox->addStretch();

    hBox = new HBox(vBox);
    hBox->addSpacing(6);

    auto fdFontBold = new QPushButton("B");
    fdFontBold->setStyleSheet("QPushButton{background: #bbb; color: #888; font-size: 20px; font-weight: bold;} QPushButton:checked{background: #29c; color: #fff;}");
    fdFontBold->setFixedSize(30, 30);
    fdFontBold->setCheckable(true);
    fdFontBold->setChecked(this->font.bold());
    connect(fdFontBold, &QCheckBox::toggled, this, [this](bool checked) {
        this->font.setBold(checked);
        update();
    });
    hBox->addWidget(fdFontBold);

    auto fdFontItalic = new QPushButton("I");
    fdFontItalic->setStyleSheet("QPushButton{background: #bbb; color: #888; font-size: 20px; font-style: italic;} QPushButton:checked{background: #29c; color: #fff;}");
    fdFontItalic->setFixedSize(30, 30);
    fdFontItalic->setCheckable(true);
    fdFontItalic->setChecked(this->font.italic());
    connect(fdFontItalic, &QCheckBox::toggled, this, [this](bool checked) {
        this->font.setItalic(checked);
        update();
    });
    hBox->addWidget(fdFontItalic);

    auto fdFontUnderline = new QPushButton("U");
    fdFontUnderline->setStyleSheet("QPushButton{background: #bbb; color: #888; font-size: 20px; text-decoration: underline;} QPushButton:checked{background: #29c; color: #fff;}");
    fdFontUnderline->setFixedSize(30, 30);
    fdFontUnderline->setCheckable(true);
    fdFontUnderline->setChecked(this->font.underline());
    connect(fdFontUnderline, &QCheckBox::toggled, this, [this](bool checked) {
        this->font.setUnderline(checked);
        update();
    });
    hBox->addWidget(fdFontUnderline);
    hBox->addStretch();

    auto fdTextColor = new LoColorSelector("T", this->textColor);
    fdTextColor->setFixedSize(30, 30);
    fdTextColor->setFlat(true);
    connect(fdTextColor, &LoColorSelector::sColorChanged, this, [this](const QColor &color) {
        this->textColor = color;
        update();
    });
    hBox->addWidget(fdTextColor);

    auto fdBackColor = new LoColorSelector("B", this->backColor);
    fdBackColor->setFixedSize(30, 30);
    fdBackColor->setFlat(true);
    connect(fdBackColor, &LoColorSelector::sColorChanged, this, [this](const QColor &color) {
        this->backColor = color;
        update();
    });
    hBox->addWidget(fdBackColor);

    vBox->addStretch();
    return wgtAttr;
}

JObj ETimer::attrJson() const {
    JObj obj;
    addBaseAttr(obj);
    obj["elementType"] = "Timer";
    obj["isDown"] = this->isDown;
    obj["targetTime"] = this->targetTime.toString("yyyy-MM-dd HH:mm:ss");
    obj["hasDay"] = this->hasDay;
    obj["hasHour"] = this->hasHour;
    obj["hasMin"] = this->hasMin;
    obj["hasSec"] = this->hasSec;
    obj["text"] = this->text;
    obj["isMultiline"] = this->isMultiline;
    obj["font"] = this->font.family();
    obj["fontSize"] = this->font.pixelSize();
    obj["fontBold"] = this->font.bold();
    obj["fontItalic"] = this->font.italic();
    obj["fontUnderline"] = this->font.underline();
    obj["textColor"] = this->textColor.name();
    obj["backColor"] = this->backColor.alpha()==0 ? "" : this->backColor.name();
    return obj;
}
