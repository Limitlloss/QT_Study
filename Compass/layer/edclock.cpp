#include "edclock.h"
#include "base/locolorselector.h"
#include "main.h"
#include "gutil/qgui.h"
#include "tools.h"
#include <QCheckBox>
#include <QFontComboBox>
#include <QSpinBox>
#include <QToolButton>
#include <QLabel>

EDClock::EDClock(EBase *multiWin) : EBase(multiWin) {
    mType = EBase::DClock;
    timeZone = QTimeZone::systemTimeZone();
    init();
}

EDClock::EDClock(const JObj &json, EBase *multiWin) : EBase(multiWin) {
    mType = EBase::DClock;
    setBaseAttr(json);
    auto widget = json["widget"];
    if(widget.isNull()) {
        widget = json;
        font = qfont(json["font"].toString(), json["fontSize"].toInt(), json["fontBold"].toBool(), json["fontItalic"].toBool());
        font.setUnderline(json["fontUnderline"].toBool());
        color = json["color"].toStr("#ffff0000");
        hasYear  = widget["hasYear"].toBool();
        hasMonth = widget["hasMonth"].toBool();
        hasDay   = widget["hasDay"].toBool();
        hasHour  = widget["hasHour"].toBool();
        hasMin   = widget["hasMin"].toBool();
        hasSec   = widget["hasSec"].toBool();
        hasWeek = widget["hasWeek"].toBool();
        hasAmPm = widget["hasAmPm"].toBool();
        is12Hour = widget["is12Hour"].toBool();
        isFullYear = widget["isFullYear"].toBool();
        isMultiline = json["isMultiline"].toBool();
    } else {
        auto font = widget["font"];
        this->font = qfont(font["family"].toString(), font["size"].toInt(), font["bold"].toBool(), font["italics"].toBool());
        this->font.setUnderline(font["underline"].toBool());
        color = Tools::int2Color(font["color"].toInt());
        hasYear     = widget["year"].toBool();
        hasMonth    = widget["month"].toBool();
        hasDay      = widget["day"].toBool();
        hasHour     = widget["hour"].toBool();
        hasMin      = widget["min"].toBool();
        hasSec      = widget["sec"].toBool();
        hasWeek   = widget["weekly"].toBool();
        hasAmPm  = widget["AmPm"].toBool();
        is12Hour   = widget["12Hour"].toBool();
        isFullYear = widget["fullYear"].toBool();
        isMultiline = widget["multiline"].toBool();
    }
    timeZone = QTimeZone(widget["timeZone"].toString().toUtf8());
    dateStyle = widget["dateStyle"].toInt();
    timeStyle = widget["timeStyle"].toInt();
    isSingleMD = dateStyle==1||dateStyle==2||dateStyle==4||dateStyle==6||dateStyle==8||dateStyle==10||dateStyle==12;
    init();
}

void EDClock::init() {
    connect(gTick, &Tick::secChanged, this, [this](const QDateTime &cur) {
        datetime = cur.toTimeZone(timeZone);
        update();
    });
}

void EDClock::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    QString text;
    QDate date = datetime.date();
    QTime time = datetime.time();
    QString spacer = isMultiline ? "\n" : "  ";
    QString yearFmt = isFullYear ? "yyyy" : "yy";
    QString monthFmt = isSingleMD ? "M" : "MM";
    QString dateFmt = isSingleMD ? "d" : "dd";
    QString sep;
    if(dateStyle > 7) sep = "-";
    else if(dateStyle > 1) sep = "/";
    switch(dateStyle) {
    case 0: case 1:
        if(hasYear) text += date.toString(yearFmt) + "年";
        if(hasMonth) text += date.toString(monthFmt) + "月";
        if(hasDay) text += date.toString(dateFmt) + "日";
        break;
    case 2: case 3: case 8: case 9:
        if(hasMonth) text += date.toString(monthFmt) + sep;
        if(hasDay) text += date.toString(dateFmt) + sep;
        if(hasYear) text += date.toString(yearFmt);
        if(text.endsWith(sep)) text.chop(1);
        break;
    case 4: case 5: case 10: case 11:
        if(hasDay) text += date.toString(dateFmt) + sep;
        if(hasMonth) text += date.toString(monthFmt) + sep;
        if(hasYear) text += date.toString(yearFmt);
        if(text.endsWith(sep)) text.chop(1);
        break;
    case 6: case 7: case 12: case 13:
        if(hasYear) text += date.toString(yearFmt) + sep;
        if(hasMonth) text += date.toString(monthFmt) + sep;
        if(hasDay) text += date.toString(dateFmt);
        if(text.endsWith(sep)) text.chop(1);
        break;
    default:
        break;
    }
    if(! text.isEmpty()) text += spacer;

    if(hasWeek) {
        auto dayOfWeek = date.dayOfWeek();
        if(dayOfWeek==1) text += tr("MON");
        else if(dayOfWeek==2) text += tr("TUE");
        else if(dayOfWeek==3) text += tr("WED");
        else if(dayOfWeek==4) text += tr("THU");
        else if(dayOfWeek==5) text += tr("FRI");
        else if(dayOfWeek==6) text += tr("SAT");
        else if(dayOfWeek==7) text += tr("SUN");
        text += spacer;
    }
    QString timeFmt;
    if(is12Hour && hasAmPm) timeFmt += (time.hour()<12 ? tr("AM") : tr("PM")) + " ";
    if(hasHour) timeFmt += (timeStyle!=1 ? "hh:" : "h:");
    if(hasMin) timeFmt += "mm:";
    if(hasSec) timeFmt += "ss";
    if(timeFmt.endsWith(":")) timeFmt.chop(1);
    if(! timeFmt.isEmpty()) text += time.toString(timeFmt);
    text = text.trimmed();
    auto lines = text.split("\n");
    auto rect = innerRect();
    if(lines.size() > 1) rect.setHeight(lineHeight = rect.height() / lines.size());
    painter->save();
    painter->setPen(color);
    font.setStyleStrategy(gTextAntialiasing ? QFont::PreferAntialias : QFont::NoAntialias);
    painter->setFont(font);
    for(auto &line : lines) {
        painter->drawText(rect, line, QTextOption(Qt::AlignCenter));
        rect.translate(0, rect.height());
    }
    painter->restore();
    EBase::paint(painter, option, widget);
}

QWidget* EDClock::attrWgt() {
    auto wgtAttr = new QWidget();
    auto vBox = new QVBoxLayout(wgtAttr);
    vBox->setContentsMargins(6, 0, 6, 0);
    if(mMultiWin) vBox->setSpacing(3);

    addBaseAttrWgt(vBox);

    auto hBox = new QHBoxLayout();
    hBox->addWidget(new QLabel(translate("","Basic Properties")));

    auto line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    hBox->addWidget(line, 1);

    vBox->addLayout(hBox);

    hBox = new QHBoxLayout();
    hBox->addSpacing(6);
    hBox->addWidget(new QLabel(tr("Time Zone")));

    auto edTimeZone = new QComboBox;
    auto zoneIds = QTimeZone::availableTimeZoneIds();
    for(auto &zoneId : zoneIds) edTimeZone->addItem(zoneId);
    edTimeZone->setCurrentText(timeZone.id());
    connect(edTimeZone, &QComboBox::currentTextChanged, this, [this](const QString &text) {
        timeZone = QTimeZone(text.toUtf8());
        update();
    });
    hBox->addWidget(edTimeZone);
    hBox->addStretch();

    vBox->addLayout(hBox);

    line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    vBox->addWidget(line);

    hBox = new QHBoxLayout();
    hBox->addSpacing(6);

    auto fdYear = new QCheckBox(tr("Year"));
    fdYear->setChecked(hasYear);
    connect(fdYear, &QCheckBox::toggled, this, [this](bool checked) {
        hasYear = checked;
        update();
    });
    hBox->addWidget(fdYear);

    auto fdMonth = new QCheckBox(tr("Month"));
    fdMonth->setChecked(hasMonth);
    connect(fdMonth, &QCheckBox::toggled, this, [this](bool checked) {
        hasMonth = checked;
        update();
    });
    hBox->addWidget(fdMonth);

    auto fdDay = new QCheckBox(tr("Day"));
    fdDay->setChecked(hasDay);
    connect(fdDay, &QCheckBox::toggled, this, [this](bool checked) {
        hasDay = checked;
        update();
    });
    hBox->addWidget(fdDay);

    vBox->addLayout(hBox);

    hBox = new QHBoxLayout();
    hBox->addSpacing(6);

    auto fdHour = new QCheckBox(tr("Hour"));
    fdHour->setChecked(hasHour);
    connect(fdHour, &QCheckBox::toggled, this, [this](bool checked) {
        hasHour = checked;
        update();
    });
    hBox->addWidget(fdHour);

    auto fdMin = new QCheckBox(tr("Min."));
    fdMin->setChecked(hasMin);
    connect(fdMin, &QCheckBox::toggled, this, [this](bool checked) {
        hasMin = checked;
        update();
    });
    hBox->addWidget(fdMin);

    auto fdSec = new QCheckBox(tr("Sec."));
    fdSec->setChecked(hasSec);
    connect(fdSec, &QCheckBox::toggled, this, [this](bool checked) {
        hasSec = checked;
        update();
    });
    hBox->addWidget(fdSec);

    vBox->addLayout(hBox);

    hBox = new QHBoxLayout();
    hBox->addSpacing(6);
    auto fdHasWeek = new QCheckBox(tr("Weekly"));
    fdHasWeek->setChecked(hasWeek);
    connect(fdHasWeek, &QCheckBox::toggled, this, [this](bool checked) {
        hasWeek = checked;
        update();
    });
    hBox->addWidget(fdHasWeek);

    auto fdFullYear = new QCheckBox(tr("Full Year"));
    fdFullYear->setChecked(isFullYear);
    connect(fdFullYear, &QCheckBox::toggled, this, [this](bool checked) {
        isFullYear = checked;
        update();
    });
    hBox->addWidget(fdFullYear);

    vBox->addLayout(hBox);

    hBox = new QHBoxLayout();
    hBox->addSpacing(6);

    auto fdIs12Hour = new QCheckBox(tr("12-Hour"));
    fdIs12Hour->setChecked(is12Hour);
    hBox->addWidget(fdIs12Hour);

    auto fdAmPm = new QCheckBox(tr("AM")+"/"+tr("PM"));
    fdAmPm->setChecked(hasAmPm);
    connect(fdIs12Hour, &QCheckBox::toggled, this, [this, fdAmPm](bool checked) {
        is12Hour = checked;
        fdAmPm->setVisible(checked);
        if(! checked) fdAmPm->setChecked(false);
        update();
    });
    connect(fdAmPm, &QCheckBox::toggled, this, [this](bool checked) {
        hasAmPm = checked;
        update();
    });
    hBox->addWidget(fdAmPm);

    vBox->addLayout(hBox);

    line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    vBox->addWidget(line);

    hBox = new QHBoxLayout();
    hBox->addSpacing(6);
    hBox->addWidget(new QLabel(tr("Date Style")));

    auto fdDateFmt = new QComboBox;
    fdDateFmt->addItem("1970-02-24", 13);
    fdDateFmt->addItem("1970-2-24", 12);
    fdDateFmt->addItem("1970年02月04日", 0);
    fdDateFmt->addItem("1970年2月4日", 1);
    fdDateFmt->addItem("1970/02/24", 7);
    fdDateFmt->addItem("1970/2/24", 6);
    fdDateFmt->addItem("02-24-1970", 9);
    fdDateFmt->addItem("2-24-1970", 8);
    fdDateFmt->addItem("02/24/1970", 3);
    fdDateFmt->addItem("2/24/1970", 2);
    fdDateFmt->addItem("24-02-1970", 11);
    fdDateFmt->addItem("24-2-1970", 10);
    fdDateFmt->addItem("24/02/1970", 5);
    fdDateFmt->addItem("24/2/1970", 4);
    SetCurData(fdDateFmt, dateStyle);
    connect(fdDateFmt, (void(QComboBox::*)(int))&QComboBox::currentIndexChanged, this, [=](int index) {
        dateStyle = fdDateFmt->itemData(index).toInt();
        isSingleMD = index==1||index==2||index==4||index==6||index==8||index==10||index==12;
        update();
    });
    hBox->addWidget(fdDateFmt);
    hBox->addStretch();

    vBox->addLayout(hBox);

    hBox = new QHBoxLayout();
    hBox->addSpacing(6);
    hBox->addWidget(new QLabel(tr("Time Style")));

    auto fdTimeFmt = new QComboBox;
    fdTimeFmt->addItem("HH:MM:SS");
    fdTimeFmt->addItem("H:MM:SS");
    fdTimeFmt->setCurrentIndex(timeStyle);
    connect(fdTimeFmt, (void(QComboBox::*)(int))&QComboBox::currentIndexChanged, this, [this](int index) {
        timeStyle = index;
        update();
    });
    hBox->addWidget(fdTimeFmt);
    hBox->addStretch();

    vBox->addLayout(hBox);

    hBox = new QHBoxLayout();
    hBox->addSpacing(6);
    hBox->addWidget(new QLabel(tr("Display Style")));

    auto fdIsMultiline = new QCheckBox(tr("Multiline"));
    fdIsMultiline->setChecked(isMultiline);
    connect(fdIsMultiline, &QCheckBox::toggled, this, [this](bool checked) {
        isMultiline = checked;
        update();
    });
    hBox->addWidget(fdIsMultiline);
    hBox->addStretch();

    vBox->addLayout(hBox);

    line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    vBox->addWidget(line);

    hBox = new QHBoxLayout();
    hBox->addSpacing(6);

    auto fdFontFamily = new QFontComboBox();
    fdFontFamily->setEditable(false);
    fdFontFamily->setCurrentText(font.family());
    connect(fdFontFamily, &QFontComboBox::currentFontChanged, this, [this](const QFont &f) {
        QFont ft(f.family());
        ft.setPixelSize(font.pixelSize());
        ft.setBold(font.bold());
        ft.setItalic(font.italic());
        ft.setUnderline(font.underline());
        font = ft;
        update();
    });
    hBox->addWidget(fdFontFamily);

    auto fdFontSize = new QSpinBox();
    fdFontSize->setRange(4, 9999);
    fdFontSize->setValue(font.pixelSize());
    connect(fdFontSize, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, [this](int value) {
        font.setPixelSize(value);
        update();
    });
    hBox->addWidget(fdFontSize);
    hBox->addStretch();

    vBox->addLayout(hBox);

    hBox = new QHBoxLayout();
    hBox->addSpacing(6);

    auto fdBold = new QPushButton("B");
    fdBold->setStyleSheet("QPushButton{background: #bbb; color: #888; font-size: 20px; font-weight: bold;} QPushButton:checked{background: #29c; color: #fff;}");
    fdBold->setFixedSize(30, 30);
    fdBold->setCheckable(true);
    fdBold->setChecked(font.bold());
    connect(fdBold, &QCheckBox::toggled, this, [this](bool checked) {
        font.setBold(checked);
        update();
    });
    hBox->addWidget(fdBold);

    auto fdItalic = new QPushButton("I");
    fdItalic->setStyleSheet("QPushButton{background: #bbb; color: #888; font-size: 20px; font-style: italic;} QPushButton:checked{background: #29c; color: #fff;}");
    fdItalic->setFixedSize(30, 30);
    fdItalic->setCheckable(true);
    fdItalic->setChecked(font.italic());
    connect(fdItalic, &QCheckBox::toggled, this, [this](bool checked) {
        font.setItalic(checked);
        update();
    });
    hBox->addWidget(fdItalic);

    auto fdFontUnderline = new QPushButton("U");
    fdFontUnderline->setStyleSheet("QPushButton{background: #bbb; color: #888; font-size: 20px; text-decoration: underline;} QPushButton:checked{background: #29c; color: #fff;}");
    fdFontUnderline->setFixedSize(30, 30);
    fdFontUnderline->setCheckable(true);
    fdFontUnderline->setChecked(font.underline());
    connect(fdFontUnderline, &QCheckBox::toggled, this, [this](bool checked) {
        font.setUnderline(checked);
        update();
    });
    hBox->addWidget(fdFontUnderline);

    auto fdColor = new LoColorSelector("T", color);
    fdColor->setFixedWidth(30);
    connect(fdColor, &LoColorSelector::sColorChanged, this, [this](const QColor &color) {
        this->color = color;
        update();
    });
    hBox->addWidget(fdColor);
    hBox->addStretch();
    vBox->addLayout(hBox);

    vBox->addStretch();
    return wgtAttr;
}

JObj EDClock::attrJson() const {
    JObj json;
    addBaseAttr(json);
    json["elementType"] = "DClock";
    json["timeZone"] = QString::fromUtf8(timeZone.id());
    json["hasYear"] = hasYear;
    json["hasMonth"] = hasMonth;
    json["hasDay"]  = hasDay;
    json["hasHour"] = hasHour;
    json["hasMin"]  = hasMin;
    json["hasSec"]  = hasSec;
    json["hasWeek"] = hasWeek;
    json["isFullYear"] = isFullYear;
    json["is12Hour"] = is12Hour;
    json["hasAmPm"] = hasAmPm;
    json["dateStyle"] = dateStyle;
    json["timeStyle"] = timeStyle;
    json["isMultiline"] = isMultiline;
    json["color"] = color.name(QColor::HexArgb);
    json["font"] = font.family();
    json["fontSize"] = font.pixelSize();
    json["fontBold"] = font.bold();
    json["fontItalic"] = font.italic();
    json["fontUnderline"] = font.underline();
    json["lineHeight"] = lineHeight;
    return json;
}
