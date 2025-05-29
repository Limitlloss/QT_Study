#include "etimer2.h"
#include "main.h"
#include "gutil/qgui.h"
#include "base/locolorselector.h"
#include "base/calendarbutton.h"
#include <QButtonGroup>
#include <QFontComboBox>
#include <QMessageBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QStackedLayout>
#include <QTextBlock>
#if(QT_VERSION_MAJOR > 5)
#include <QStringConverter>
#else
#include <QTextCodec>
#endif
#include <QTextEdit>
#include <QTimeEdit>
#include <QToolButton>
#include <QPainter>


ETimer2::ETimer2(EBase *multiWin) : EBase(multiWin) {
    isUp = false;
    targetTime = QDateTime::currentDateTime();
    html = "<body>"+tr("There are")+" %d "+tr("Days")+" %h "+tr("Hours")+" %m "+tr("Mins")+" %s "+tr("Secs")+"</body>";
    init();
}
ETimer2::ETimer2(const JObj &json, EBase *multiWin) : EBase(multiWin) {
    setBaseAttr(json);
    isUp = json["isUp"].toBool();
    targetTime = QDateTime::fromString(json["targetTime"].toString(), "yyyy-MM-dd HH:mm:ss");
    html = json["html"].toString();
    backColor = json["backColor"].toString("#00000000");
    init();
}

void ETimer2::init() {
    mType = EBase::Timer2;
    doc.setDocumentMargin(0);
    QFont font;
    font.setFamily("Arial");
    font.setFamilies({"Arial","黑体"});
    font.setPixelSize(16);
    doc.setDefaultFont(font);
    doc.setDefaultStyleSheet("body {color: #fff; line-height:1}");

    connect(this, &ETimer2::sizeChanged, this, &ETimer2::updImg);
    connect(gTick, &Tick::secChanged, this, [this](const QDateTime &cur) {
        auto sss = isUp ? targetTime.secsTo(cur) : cur.secsTo(targetTime);
        if(sss < 0) sss = 0;
        if(secs==sss) return;
        secs = sss;
        updImg();
    }, Qt::UniqueConnection);
    updImg();
}

class TTextEdit : public QTextEdit {
public:
    TTextEdit() {}
    explicit TTextEdit(const QString &text) : QTextEdit(text){}
    QSize minimumSizeHint() const override {
        return sizeHint();
    };
    QSize sizeHint() const override {
        auto size = QTextEdit::sizeHint();
        auto minH = minimumHeight();
        if(minH > 0) size.setHeight(minH+0xfff);
        return size;
    };
};
QWidget* ETimer2::attrWgt() {
    auto wgtAttr = new QWidget;
    auto vBox = new VBox(wgtAttr);
    vBox->setContentsMargins(4, 0, 4, 0);
    vBox->setSpacing(3);

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
    (isUp ? fdCntUp : fdCntDown)->setChecked(true);

    auto cntGroup = new QButtonGroup(wgtAttr);
    cntGroup->addButton(fdCntDown);
    cntGroup->addButton(fdCntUp);
    connect(fdCntUp, &QRadioButton::toggled, this, [this](bool checked) {
        isUp = checked;
        update();
    });
    hBox->addWidget(fdCntDown);
    hBox->addStretch();
    hBox->addWidget(fdCntUp);
    hBox->addStretch();

    hBox = new HBox(vBox);
    hBox->addSpacing(6);
    hBox->addLabel(tr("Target Time"));

    auto edTime = new QDateTimeEdit(targetTime);
    edTime->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    connect(edTime, &QTimeEdit::dateTimeChanged, this, [this](const QDateTime &dateTime) {
        targetTime = dateTime;
        update();
    });
    hBox->addWidget(edTime);

    auto btnCalendar = new CalendarButton;
    connect(btnCalendar->calendar, &QCalendarWidget::clicked, edTime, &QDateTimeEdit::setDate);
    hBox->addWidget(btnCalendar);
    hBox->addStretch();


    auto fdText = new TTextEdit("");

    hBox = new HBox(vBox);
    hBox->setSpacing(3);

    hBox->addLabel(tr("Font Size")+":");
    auto fdFontSize = new QSpinBox;
    fdFontSize->setRange(4, 9999);
    fdFontSize->setValue(16);
    connect(fdFontSize, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, fdText, [fdText](int value) {
        if(value <= 0) return;
        QTextCharFormat fmt;
        fmt.setProperty(QTextFormat::FontPixelSize, value);
        auto cursor = fdText->textCursor();
        if(! cursor.hasSelection()) cursor.select(QTextCursor::WordUnderCursor);
        cursor.mergeCharFormat(fmt);
    });
    hBox->addWidget(fdFontSize);



    hBox->addLabel(tr("Line Height"));

    auto edLineHeight = new QDoubleSpinBox;
    edLineHeight->setDecimals(1);
    edLineHeight->setRange(-999, 999);
    edLineHeight->setValue(1);
    connect(edLineHeight, (void(QDoubleSpinBox::*)(double))&QDoubleSpinBox::valueChanged, this, [this, fdText](double value) {
        QTextBlockFormat fmt;
        fmt.setLineHeight(value*100, QTextBlockFormat::ProportionalHeight);
        auto cursor = fdText->textCursor();
        if(! cursor.hasSelection()) cursor.select(QTextCursor::WordUnderCursor);
        cursor.mergeBlockFormat(fmt);
        updImg();
    });
    hBox->addWidget(edLineHeight);
    hBox->addStretch();


    hBox = new HBox(vBox);
    hBox->setSpacing(3);

    auto wTextAlignHL = new QPushButton(QIcon(":/res/program/TextAlignHL.png"), "");
    wTextAlignHL->setStyleSheet("QPushButton{border: none; background: #bbb;} QPushButton:checked{background: #29c;}");
    wTextAlignHL->setFixedSize(30, 30);
    wTextAlignHL->setIconSize(QSize(30, 30));
    wTextAlignHL->setCheckable(true);
    wTextAlignHL->setChecked(true);
    hBox->addWidget(wTextAlignHL);

    auto wTextAlignHC = new QPushButton(QIcon(":/res/program/TextAlignHC.png"), "");
    wTextAlignHC->setStyleSheet("QPushButton{border: none; background: #bbb;} QPushButton:checked{background: #29c;}");
    wTextAlignHC->setFixedSize(30, 30);
    wTextAlignHC->setIconSize(QSize(30, 30));
    wTextAlignHC->setCheckable(true);
    hBox->addWidget(wTextAlignHC);

    auto wTextAlignHR = new QPushButton(QIcon(":/res/program/TextAlignHR.png"), "");
    wTextAlignHR->setStyleSheet("QPushButton{border: none; background: #bbb;} QPushButton:checked{background: #29c;}");
    wTextAlignHR->setFixedSize(30, 30);
    wTextAlignHR->setIconSize(QSize(30, 30));
    wTextAlignHR->setCheckable(true);
    hBox->addWidget(wTextAlignHR);

    hBox->addStretch();

    auto fdFontBold = new QPushButton("B");
    fdFontBold->setStyleSheet("QPushButton{background: #bbb; color: #888; font-size: 20px; font-weight: bold;} QPushButton:checked{background: #29c; color: #fff;}");
    fdFontBold->setFixedSize(30, 30);
    fdFontBold->setCheckable(true);
    connect(fdFontBold, &QToolButton::toggled, fdText, [fdText](bool checked) {
        QTextCharFormat fmt;
        fmt.setFontWeight(checked ? QFont::Bold : QFont::Normal);
        MergeFmt(fdText, fmt);
    });
    hBox->addWidget(fdFontBold);

    auto fdFontItalic = new QPushButton("I");
    fdFontItalic->setStyleSheet("QPushButton{background: #bbb; color: #888; font-size: 20px; font-style: italic;} QPushButton:checked{background: #29c; color: #fff;}");
    fdFontItalic->setFixedSize(30, 30);
    fdFontItalic->setCheckable(true);
    connect(fdFontItalic, &QToolButton::toggled, fdText, [fdText](bool checked) {
        QTextCharFormat fmt;
        fmt.setFontItalic(checked);
        MergeFmt(fdText, fmt);
    });
    hBox->addWidget(fdFontItalic);

    auto fdFontUnderline = new QPushButton("U");
    fdFontUnderline->setStyleSheet("QPushButton{background: #bbb; color: #888; font-size: 20px; text-decoration: underline;} QPushButton:checked{background: #29c; color: #fff;}");
    fdFontUnderline->setFixedSize(30, 30);
    fdFontUnderline->setCheckable(true);
    connect(fdFontUnderline, &QToolButton::toggled, fdText, [fdText](bool checked) {
        QTextCharFormat fmt;
        fmt.setFontUnderline(checked);
        MergeFmt(fdText, fmt);
    });
    hBox->addWidget(fdFontUnderline);

    hBox->addStretch();

    auto edTextColor = new LoColorSelector("F", Qt::white);
    edTextColor->setToolTip(tr("Foreground Color"));
    edTextColor->setFixedSize(30, 30);
    connect(edTextColor, &LoColorSelector::sColorChanged, fdText, [fdText](const QColor &color) {
        if(! color.isValid()) return;
        QTextCharFormat fmt;
        fmt.setForeground(color);
        MergeFmt(fdText, fmt);
    });
    hBox->addWidget(edTextColor);

    auto edBack = new LoColorSelector("LB", Qt::white);
    edBack->setToolTip(tr("Line Background"));
    edBack->setFixedSize(34, 30);
    connect(edBack, &LoColorSelector::sColorChanged, fdText, [fdText](const QColor &color) {
        if(! color.isValid()) return;
        QTextCharFormat fmt;
        fmt.setBackground(color);
        MergeFmt(fdText, fmt);
    });
    hBox->addWidget(edBack);

    auto edBackColor = new LoColorSelector("B", backColor);
    edBackColor->setToolTip(tr("Background Color"));
    edBackColor->setFixedSize(30, 30);
    connect(edBackColor, &LoColorSelector::sColorChanged, this, [this](const QColor &color) {
        if(! color.isValid()) return;
        backColor = color;
        updImg();
    });
    hBox->addWidget(edBackColor);


    auto fdAlignH = new QButtonGroup(wgtAttr);
    fdAlignH->addButton(wTextAlignHL, Qt::AlignLeft);
    fdAlignH->addButton(wTextAlignHC, Qt::AlignHCenter);
    fdAlignH->addButton(wTextAlignHR, Qt::AlignRight);
    connect(fdAlignH, &QButtonGroup::idClicked, this, [this, fdText](int value) {
        QTextBlockFormat fmt;
        fmt.setAlignment((Qt::Alignment) value);
        QTextCursor cursor = fdText->textCursor();
        cursor.mergeBlockFormat(fmt);
        updImg();
    });

    auto ll = vBox->addLabel("%d: "+tr("Days")+", %h: "+tr("Hours")+", %m: "+tr("Mins")+", %s: "+tr("Secs"));
    gFont(ll, 16);

    fdText->setMinimumHeight(160);
    auto doc = fdText->document();
    if(doc) doc->setDocumentMargin(2);
    auto font = fdText->font();
    font.setFamily("Arial");
    font.setFamilies({"Arial","黑体"});
    font.setPixelSize(16);
    fdText->setFont(font);
    auto pal = fdText->palette();
    pal.setColor(QPalette::Base, Qt::black);
    pal.setColor(QPalette::Text, Qt::white);
    fdText->setPalette(pal);
    fdText->document()->setDefaultStyleSheet("body {color: #fff; line-height:1}");
    fdText->setFrameShape(QFrame::NoFrame);
    fdText->setAcceptRichText(false);
    fdText->setHtml(html);
    connect(fdText, &QTextEdit::textChanged, this, [this, fdText] {
        html = fdText->toHtml();
        updImg();
    });
    connect(fdText, &QTextEdit::currentCharFormatChanged, this, [=](const QTextCharFormat &format) {
        fdFontSize->blockSignals(true);
        fdFontSize->setValue(format.font().pixelSize());
        fdFontSize->blockSignals(false);
        auto foreground = format.foreground();
        edTextColor->blockSignals(true);
        edTextColor->setColor(foreground.style()==Qt::NoBrush ? Qt::white : foreground.color());
        edTextColor->blockSignals(false);

        auto background = format.background();
        edBack->blockSignals(true);
        edBack->setColor(background.style()==Qt::NoBrush ? Qt::transparent : background.color());
        edBack->blockSignals(false);

        auto cursor = fdText->textCursor();
        auto blockFormat = cursor.blockFormat();
        auto btn = fdAlignH->button(blockFormat.alignment() & Qt::AlignHorizontal_Mask);
        if(btn) btn->setChecked(true);

        edLineHeight->blockSignals(true);
        edLineHeight->setValue(blockFormat.lineHeightType()==QTextBlockFormat::ProportionalHeight ? blockFormat.lineHeight()*0.01 : 1.0);
        edLineHeight->blockSignals(false);
    });
    vBox->addWidget(fdText);

    return wgtAttr;
}
void repLineHeight(QString &html) {
    QString pre;
    int last = 0, idx;
    while((idx = html.indexOf("line-height:", last)) > -1) {
        idx += 12;
        auto end = idx;
        QChar ch;
        while((ch = html[end])<='9' && ch>='-' && ch!='/') if(++end >= html.size()) goto end;
        if(ch!='%') last = end;
        else {
            pre += html.mid(last, idx-last) + QString::number(html.mid(idx, end-idx).toDouble() / 100);
            last = end + 1;
        }
    }
    end:
    if(! pre.isEmpty()) html = pre + html.mid(last);
}
JObj ETimer2::attrJson() const {
    JObj ele{{"elementType", "Timer2"}};
    addBaseAttr(ele);
    ele["isUp"] = isUp;
    auto hhhh = html;
    auto idx = hhhh.indexOf("<html");
    if(idx > -1) hhhh = hhhh.mid(idx);
    idx = hhhh.indexOf("<body style=\"");
    if(idx > -1 && ! hhhh.mid(idx+13, 8).contains("color:")) hhhh = hhhh.insert(idx+13, "color:#fff;");
    hhhh.replace("<meta name=\"qrichtext\" content=\"1\" />", "");
    hhhh.replace(" -qt-block-indent:0; text-indent:0px;", "");
    repLineHeight(hhhh);
    ele["html"] = hhhh.replace("margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;", "margin:0;");
    ele["targetTime"] = targetTime.toString("yyyy-MM-dd HH:mm:ss");
    ele["backColor"] = backColor.name(QColor::HexArgb);
    return ele;
}

void ETimer2::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    painter->save();
    auto rect = innerRect();
    painter->drawImage(rect, img);
    painter->restore();
    EBase::paint(painter, option, widget);
}

void ETimer2::updImg() {
    auto innerRect = this->innerRect();
    int width = innerRect.width();
    int height = innerRect.height();
    if(width<1 || height<1) return;

    auto hasDay = html.contains("%d");
    auto hasHour = html.contains("%h");
    auto hasMin = html.contains("%m");
    auto hasSec = html.contains("%s");

    auto htm = html;
    if(hasDay) {
        htm = htm.replace("%d", QString::number(secs/86400));
        secs %= 86400;
    }
    if(hasHour) {
        htm = htm.replace("%h", QString::asprintf("%02d", secs/3600));
        secs %= 3600;
    }
    if(hasMin) {
        htm = htm.replace("%m", QString::asprintf("%02d", secs/60));
        secs %= 60;
    }
    if(hasSec) htm = htm.replace("%s", QString::asprintf("%02d", secs));

    doc.setHtml(htm);
    doc.setTextWidth(width);
    width<<=1;
    height<<=1;
    if(img.width()!=width || img.height()!=height) img = QImage(width, height, QImage::Format_ARGB32);
    img.fill(backColor);
    {
        QPainter painter(&img);
        painter.scale(2,2);
        doc.drawContents(&painter);
    }
    update();
}
