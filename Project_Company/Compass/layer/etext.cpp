#include "etext.h"
#include "base/locolorselector.h"
#include "main.h"
#include "gutil/qgui.h"
#include <QButtonGroup>
#include <QFontComboBox>
#include <QMessageBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QFileDialog>
#include <QStackedLayout>
#include <QTextBlock>
#include <QCheckBox>
#if(QT_VERSION_MAJOR > 5)
#include <QStringConverter>
#else
#include <QTextCodec>
#endif
#include <QTimeEdit>
#include <QToolButton>
#include <QPainter>

static QColor charColors[]{"#fff","#f00","#f00","#f0f","#c0c","#ff0","#f80","#0f0","#0f0","#0a0","#0a0","#7b0","#00f","#00f","#0af","#0ef"};
QString playModes[]{"Flip", "Scroll", "Static"};

EText::EText(EBase *multiWin) : EBase(multiWin) {
    mType = EBase::Text;
    text = "<body>"+tr("Enter your text")+"</body>";
    connect(this, &EText::sizeChanged, this, &EText::updImg);
    updImg();
}
EText::EText(const JObj &json, EBase *multiWin) : EBase(multiWin) {
    mType = EBase::Text;
    setBaseAttr(json);
    auto widget = json["widget"];
    if(widget.isNull()) widget = json;
    text = widget["text"].toString();
    align = (Qt::Alignment) widget["align"].toInt();
    backColor = widget["backColor"].toString("#00000000");
    lastFont = widget["lastFont"].toString("黑体");
    auto play = json["play"];
    if(play.isNull()) {
        playMode = json["playMode"].toString();
        if(playMode=="Scroll") {
            direction = json["direction"].toString();
            speed = json["speed"].toInt();
            tailSpacing = json["headTailSpacing"].toInt();
            useNewFmt = json["useNewFmt"].toBool();
        }
    } else {
        playMode = playModes[play["style"].toInt()];
        auto rolling = play["rolling"];
        QString ds[]{"left", "top", "right", "bottom"};
        direction = ds[rolling["rollingStyle"].toInt()];
        speed = 1000/rolling["rollingSpeed"].toInt(33);
        tailSpacing = rolling["headTailSpacing"].toInt();
    }
    connect(this, &EText::sizeChanged, this, &EText::updImg);
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
QWidget* EText::attrWgt() {
    auto wgtAttr = new QWidget;
    auto vBox = new QVBoxLayout(wgtAttr);
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

    auto fdText = new TTextEdit("");

    auto fdFontFamily = new QFontComboBox;
    fdFontFamily->setEditable(false);
    fdFontFamily->setCurrentFont(QFont(lastFont));
    connect(fdFontFamily, &QFontComboBox::currentFontChanged, fdText, [=](const QFont &f) {
        QTextCharFormat fmt;
        fmt.setFontFamilies({f.family()});
        QTextCursor cursor = fdText->textCursor();
        if(! cursor.hasSelection()) cursor.select(QTextCursor::WordUnderCursor);
        cursor.mergeCharFormat(fmt);
        lastFont = f.family();
    });
    hBox->addWidget(fdFontFamily);

    hBox->addStretch();

    auto fdFontSize = new QSpinBox;
    fdFontSize->setRange(4, 9999);
    fdFontSize->setValue(16);
    connect(fdFontSize, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, fdText, [=](int value) {
        if(value <= 0) return;
        QTextCharFormat fmt;
        fmt.setProperty(QTextFormat::FontPixelSize, value);
        QTextCursor cursor = fdText->textCursor();
        if(! cursor.hasSelection()) cursor.select(QTextCursor::WordUnderCursor);
        cursor.mergeCharFormat(fmt);
    });
    hBox->addWidget(fdFontSize);

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

    auto fdTextColor = new LoColorSelector("T", Qt::white);
    fdTextColor->setToolTip(tr("Text Color"));
    fdTextColor->setFixedSize(30, 30);
    connect(fdTextColor, &LoColorSelector::sColorChanged, fdText, [fdText](const QColor &color) {
        if(! color.isValid()) return;
        QTextCharFormat fmt;
        fmt.setForeground(color);
        MergeFmt(fdText, fmt);
    });
    hBox->addWidget(fdTextColor);

    auto fdBackColor = new LoColorSelector("B", backColor);
    fdBackColor->setToolTip(tr("Back Color"));
    fdBackColor->setFixedSize(30, 30);
    connect(fdBackColor, &LoColorSelector::sColorChanged, this, [this](const QColor &color) {
        if(! color.isValid()) return;
        backColor = color;
        updImg();
    });
    hBox->addWidget(fdBackColor);

    auto fdRandomColor = new QPushButton(QIcon(":/res/random.png"), "");
    fdRandomColor->setToolTip(tr("Colorful Text"));
    fdRandomColor->setFixedSize(30, 30);
    fdRandomColor->setIconSize(QSize(30, 30));
    connect(fdRandomColor, &QPushButton::clicked, fdText, [fdText] {
        auto cursor = fdText->textCursor();
        auto len = fdText->document()->characterCount();
        QTextCharFormat fmt;
        int last = -1, idx;
        for(int i=0; i<len; i++) {
            cursor.setPosition(i, QTextCursor::MoveAnchor);
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            while((idx = rand()%(sizeof(charColors)/sizeof(charColors[0])))==last) ;
            last = idx;
            fmt.setForeground(charColors[idx]);
            cursor.mergeCharFormat(fmt);
        }
    });
    hBox->addWidget(fdRandomColor);

    hBox = new HBox(vBox);
    hBox->setSpacing(3);

    auto wTextAlignVT = new QPushButton(QIcon(":/res/program/TextAlignVT.png"), "");
    wTextAlignVT->setStyleSheet("QPushButton{border: none; background: #bbb;} QPushButton:checked{background: #29c;}");
    wTextAlignVT->setFixedSize(30, 30);
    wTextAlignVT->setIconSize(QSize(30, 30));
    wTextAlignVT->setCheckable(true);
    hBox->addWidget(wTextAlignVT);

    auto wTextAlignVC = new QPushButton(QIcon(":/res/program/TextAlignVC.png"), "");
    wTextAlignVC->setStyleSheet("QPushButton{border: none; background: #bbb;} QPushButton:checked{background: #29c;}");
    wTextAlignVC->setFixedSize(30, 30);
    wTextAlignVC->setIconSize(QSize(30, 30));
    wTextAlignVC->setCheckable(true);
    hBox->addWidget(wTextAlignVC);

    auto wTextAlignVB = new QPushButton(QIcon(":/res/program/TextAlignVB.png"), "");
    wTextAlignVB->setStyleSheet("QPushButton{border: none; background: #bbb;} QPushButton:checked{background: #29c;}");
    wTextAlignVB->setFixedSize(30, 30);
    wTextAlignVB->setIconSize(QSize(30, 30));
    wTextAlignVB->setCheckable(true);
    hBox->addWidget(wTextAlignVB);

    hBox->addStretch();

    auto lb = new QLabel(tr("Kerning"));
    lb->setToolTip(lb->text());
    hBox->addWidget(lb);

    auto edLetterSpacing = new QSpinBox;
    edLetterSpacing->setMaximum(9999);
    edLetterSpacing->setValue(100);
    connect(edLetterSpacing, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, [this, fdText](int value) {
        QTextCharFormat fmt;
        fmt.setFontLetterSpacing(value);
        auto cursor = fdText->textCursor();
        if(! cursor.hasSelection()) cursor.select(QTextCursor::WordUnderCursor);
        cursor.mergeCharFormat(fmt);
        updImg();
    });
    hBox->addWidget(edLetterSpacing);

    lb = new QLabel(tr("Line Height"));
    lb->setToolTip(lb->text());
    hBox->addWidget(lb);

    auto edLineHeight = new QSpinBox;
    edLineHeight->setRange(-999, 9999);
    edLineHeight->setValue(100);
    connect(edLineHeight, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, [this, fdText](int value) {
        QTextBlockFormat fmt;
        fmt.setLineHeight(value, QTextBlockFormat::ProportionalHeight);
        auto cursor = fdText->textCursor();
        if(! cursor.hasSelection()) cursor.select(QTextCursor::WordUnderCursor);
        cursor.mergeBlockFormat(fmt);
        updImg();
    });
    hBox->addWidget(edLineHeight);
    hBox->addSpacing(-2);
    hBox->addLabel("%");


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

    auto fdAlignV = new QButtonGroup(wgtAttr);
    fdAlignV->addButton(wTextAlignVT, Qt::AlignTop);
    fdAlignV->addButton(wTextAlignVC, Qt::AlignVCenter);
    fdAlignV->addButton(wTextAlignVB, Qt::AlignBottom);
    connect(fdAlignV, &QButtonGroup::idClicked, this, [this](int value) {
        align = (Qt::Alignment) value;
        updImg();
    });

    auto v_align = align & Qt::AlignVertical_Mask;
    if(v_align==Qt::AlignTop) wTextAlignVT->setChecked(true);
    if(v_align==Qt::AlignVCenter) wTextAlignVC->setChecked(true);
    if(v_align==Qt::AlignBottom) wTextAlignVB->setChecked(true);

    fdText->setMinimumHeight(160);
    auto doc = fdText->document();
    if(doc) doc->setDocumentMargin(2);
    auto font = fdText->font();
    font.setFamilies({"Arial","黑体"});
    font.setPixelSize(16);
    if(! gTextAntialiasing) font.setStyleStrategy(QFont::NoAntialias);
    fdText->setFont(font);
    auto pal = fdText->palette();
    pal.setColor(QPalette::Base, Qt::black);
    pal.setColor(QPalette::Text, Qt::white);
    fdText->setPalette(pal);
    fdText->setFrameShape(QFrame::NoFrame);
    fdText->setAcceptRichText(false);
    fdText->setHtml(text);
    connect(fdText, &QTextEdit::textChanged, this, [this, fdText] {
        text = fdText->toHtml();
        updImg();
    });
    connect(fdText, &QTextEdit::currentCharFormatChanged, this, [=](const QTextCharFormat &format) {
        auto ft = format.font();
        auto families = ft.families();
        if(! families.isEmpty()) {
            fdFontFamily->blockSignals(true);
            fdFontFamily->setCurrentFont(families[0]);
            fdFontFamily->blockSignals(false);
            lastFont = families[0];
        }
        fdFontSize->blockSignals(true);
        fdFontSize->setValue(ft.pixelSize());
        fdFontSize->blockSignals(false);
        auto foreground = format.foreground();
        fdTextColor->blockSignals(true);
        fdTextColor->setColor(foreground.style()==Qt::NoBrush ? Qt::white : foreground.color());
        fdTextColor->blockSignals(false);

        auto spa = format.fontLetterSpacing();
        edLetterSpacing->blockSignals(true);
        edLetterSpacing->setValue(spa==0 ? 100 : spa);
        edLetterSpacing->blockSignals(false);

        auto cursor = fdText->textCursor();
        auto blockFormat = cursor.blockFormat();
        auto btn = fdAlignH->button(blockFormat.alignment() & Qt::AlignHorizontal_Mask);
        if(btn) btn->setChecked(true);

        edLineHeight->blockSignals(true);
        edLineHeight->setValue(blockFormat.lineHeightType()==QTextBlockFormat::ProportionalHeight ? blockFormat.lineHeight() : 100);
        edLineHeight->blockSignals(false);
    });
    vBox->addWidget(fdText);

    hBox = new HBox(vBox);
    hBox->addStretch();

    auto pageInfoWgt = new QWidget;
    auto hhh = new QHBoxLayout(pageInfoWgt);
    hhh->setContentsMargins(0,0,0,0);

    hhh->addWidget(new QLabel(tr("PageCount:")));

    auto fdPageCnt = new QLabel(QString::number(mImgs.size()));
    hhh->addWidget(fdPageCnt);

    hhh->addSpacing(20);
    hhh->addWidget(new QLabel(tr("page")));

    auto fdPageIdx = new QSpinBox();
    fdPageIdx->setRange(1, mImgs.size());
    connect(fdPageIdx, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, [this](int idx) {
        curIdx = idx - 1;
        update();
    });
    hhh->addWidget(fdPageIdx);
    hBox->addWidget(pageInfoWgt);

    hBox->addStretch();

    auto btnImport = new QPushButton(tr("Import txt File"));
    btnImport->setProperty("ssType", "progManageTool");
    connect(btnImport, &QPushButton::clicked, fdText, [=] {
        auto filePath = QFileDialog::getOpenFileName(wgtAttr, translate("","Select File"), gFileHome, "Txt(*.txt)");
        if(filePath.isEmpty()) return;
        QFile qFile(filePath);
        if(! qFile.open(QFile::ReadOnly)) {
            QMessageBox::critical(wgtAttr, tr("Fail"), tr("Cannot Open File")+": "+qFile.errorString()+"\n"+filePath);
            return;
        }
        auto data = qFile.readAll();
        qFile.close();
#if(QT_VERSION_MAJOR > 5)
#include <QStringConverter>
        QStringDecoder decoder(QStringDecoder::Utf8);
        QString text = decoder(data);
        if(decoder.hasError()) text = QStringDecoder(QStringDecoder::System)(data);
#else
        QTextCodec::ConverterState state;
        auto text = QTextCodec::codecForName("UTF-8")->toUnicode(data.constData(), data.size(), &state);
        if(state.invalidChars > 0) text = QString::fromLocal8Bit(data);
#endif
        fdText->setText(text);
    });
    hBox->addWidget(btnImport);

    hBox = new HBox(vBox);
    hBox->addWidget(new QLabel(tr("Play Properties")));

    line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    hBox->addWidget(line, 1);


    hBox = new HBox(vBox);
    hBox->addStretch();

    auto fdFlip = new QRadioButton(tr("Flip"));
    hBox->addWidget(fdFlip);
    hBox->addStretch();

    auto fdScroll = new QRadioButton(tr("Scroll"));
    hBox->addWidget(fdScroll);
    hBox->addStretch();

    auto fdStatic = new QRadioButton(tr("Static"));
    hBox->addWidget(fdStatic);
    hBox->addStretch();


    auto fdPlayStyle = new QButtonGroup(wgtAttr);
    fdPlayStyle->addButton(fdFlip, 0);
    fdPlayStyle->addButton(fdScroll, 1);
    fdPlayStyle->addButton(fdStatic, 2);
    if(playMode=="Flip") fdFlip->setChecked(true);
    else if(playMode=="Scroll") fdScroll->setChecked(true);
    else if(playMode=="Static") fdStatic->setChecked(true);

    auto wgtAttrFlip = new QWidget;
    auto wgtAttrScroll = new QWidget;
    {
        auto vBox = new VBox(wgtAttrScroll);
        vBox->setContentsMargins(2, 0, 2, 0);
        vBox->setSpacing(3);

        auto hBox = new HBox(vBox);
        auto label = hBox->addLabel(tr("Direction"));
        label->setMinimumWidth(80);
        label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

        auto fdDirection = new QComboBox;
        fdDirection->addItem(tr("Right -> Left"), "left");
        fdDirection->addItem(tr("Bottom -> Top"), "top");
        fdDirection->addItem(tr("Left -> Right"), "right");
        fdDirection->addItem(tr("Top -> Bottom"), "bottom");
        SetCurData(fdDirection, direction);
        connect(fdDirection, (void(QComboBox::*)(int))&QComboBox::currentIndexChanged, this, [=] {
            direction = fdDirection->currentData().toString();
            updImg();
        });
        hBox->addWidget(fdDirection);

        label = hBox->addLabel(tr("Speed"));
        label->setMinimumWidth(80);
        label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

        auto fd = new QComboBox;
        fd->setEditable(true);
        fd->setMinimumWidth(50);
        fd->addItem("600");
        fd->addItem("540");
        fd->addItem("480");
        fd->addItem("420");
        fd->addItem("360");
        fd->addItem("300");
        fd->addItem("240");
        fd->addItem("180");
        fd->addItem("120");
        fd->addItem("60");
        fd->addItem("30");
        fd->setMaxVisibleItems(fd->count());
        auto text = QString::number(speed);
        if(SetCurText(fd, text)==-1) {
            SetCurText(fd, "60");
            fd->setCurrentText(text);
        }
        connect(fd, &QComboBox::editTextChanged, this, [=](const QString &text) {
            bool ok;
            auto speed = text.toInt(&ok);
            if(ok) this->speed = speed;
        });
        hBox->addWidget(fd);
        hBox->addLabel("px/s");
        hBox->addStretch();

        hBox = new HBox(vBox);
        label = hBox->addLabel(tr("Tail Spacing"));
        label->setMinimumWidth(80);
        label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

        auto edTailSpacing = new QSpinBox;
        edTailSpacing->setRange(0, 9999);
        edTailSpacing->setValue(tailSpacing);
        connect(edTailSpacing, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, [this](int value) {
            tailSpacing = value;
            updImg();
        });
        hBox->addWidget(edTailSpacing);
        hBox->addStretch();

        auto edNewFmt = new QCheckBox(tr("New Format")+" (Player 2.1.18)");
        edNewFmt->setChecked(useNewFmt);
        connect(edNewFmt, &QCheckBox::stateChanged, this, [this](int value) {
            useNewFmt = value==Qt::Checked;
            updImg();
        });
        hBox->addWidget(edNewFmt);

        vBox->addStretch();
    }
    auto stackBox = new QStackedLayout;
    vBox->addLayout(stackBox);
    stackBox->addWidget(wgtAttrFlip);
    stackBox->addWidget(wgtAttrScroll);
    stackBox->addWidget(new QWidget);
    auto idx = std::find(playModes, playModes+3, playMode)-playModes;
    if(idx>2) idx = 0;
    stackBox->setCurrentIndex(idx);
    connect(fdPlayStyle, &QButtonGroup::idToggled, this, [=](int value, bool checked) {
        if(! checked) return;
        playMode = playModes[value];
        updImg();
        stackBox->setCurrentIndex(value);
        pageInfoWgt->setVisible(playMode=="Flip");
    });
    connect(this, &EText::updPageCnt, wgtAttr, [=] {
        fdPageCnt->setText(QString::number(mImgs.size()));
        fdPageIdx->setRange(1, mImgs.size());
        fdPageIdx->setValue(1);
    });
    return wgtAttr;
}

bool EText::save(const QString &pageDir) {
    QString idDir = pageDir + QString("/%1-%2-%3-%4-%5").arg(zValue()).arg((int)x()).arg((int)y()).arg((int)mWidth).arg((int)mHeight);
    QDir().mkpath(idDir);
    for(int i=0; i<mImgs.count(); i++) mImgs[i].save(idDir + QString("/text%1.png").arg(i));
    return true;
}
JObj EText::attrJson() const {
    JArray files;
    for(int i=0; i<mImgs.count(); i++) files.append(QString("text%1.png").arg(i)); //上下滚动，生成一张纵向长图
    JObj obj{
        {"elementType", "Text"},
        {"playMode", playMode},
        {"text", text},
        {"align", (int) align},
        {"backColor", backColor.name(QColor::HexArgb)},
        {"lastFont", lastFont},
        {"files", files},
        {"idDir", QString("%1-%2-%3-%4-%5").arg(zValue()).arg((int)x()).arg((int)y()).arg((int)mWidth).arg((int)mHeight)}
    };
    addBaseAttr(obj);
    if(playMode=="Scroll") {
        obj["direction"] = direction;
        obj["speed"] = speed;
        obj["headTailSpacing"] = tailSpacing;
        obj["useNewFmt"] = useNewFmt;
    }
    return obj;
}

void EText::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    painter->save();
    auto rect = innerRect();
    if(playMode!="Flip") curIdx = 0;
    else if(curIdx>=mImgs.size()) curIdx = mImgs.size() - 1;
    else if(curIdx < 0) curIdx = 0;
    if(playMode=="Scroll") painter->drawImage(rect.x(), rect.y(), mImgs[0], 0, 0, rect.width(), rect.height());
    else painter->drawImage(rect.x(), rect.y(), mImgs[curIdx]);
    painter->restore();
    EBase::paint(painter, option, widget);
}

void EText::updImg() {
    auto innerRect = this->innerRect();
    int width = innerRect.width();
    int height = innerRect.height();
    if(width<1 || height<1) return;
    QTextDocument doc;
    doc.setDocumentMargin(0);
    QFont font;
    font.setFamilies({"Arial","黑体"});
    font.setPixelSize(16);
    if(! gTextAntialiasing) font.setStyleStrategy(QFont::NoAntialias);
    doc.setDefaultFont(font);
    doc.setDefaultStyleSheet("body {color: #fff;}");
    if(playMode=="Flip") {
        doc.setHtml(text);
        doc.setPageSize(innerRect.size());
        auto pageHeight = height;
        auto pageCnt = doc.pageCount();
        QImage img(width, pageHeight*pageCnt, QImage::Format_ARGB32);
        img.fill(backColor);
        {
            QPainter painter(&img);
            doc.drawContents(&painter);
        }
        if(pageCnt > 1) {
            check:
            for(int y=pageHeight-1; y<img.height(); y+=pageHeight) for(int x=0; x<width; x++) if(img.pixelColor(x, y)!=backColor) {
                pageHeight *= 2;
                doc.setPageSize(QSizeF(width, pageHeight));
                pageCnt = doc.pageCount();
                img = QImage(width, pageHeight*pageCnt, QImage::Format_ARGB32);
                img.fill(backColor);
                {
                    QPainter painter(&img);
                    doc.drawContents(&painter);
                }
                if(pageHeight/height <= 16) goto check;
                else goto forend;
            }
            if(pageCnt > 1) {
                for(int y=img.height()-pageHeight; y<img.height(); y++) for(int x=0; x<width; x++) if(img.pixelColor(x, y)!=backColor) goto forend;
                pageCnt--;
            }
        }
        forend:
        QRect rect(0, 0, width, pageHeight);
        mImgs.clear();
        for(int i=0; i<pageCnt; i++) {
            auto pageImg = img.copy(rect);
            alignV(pageImg, height);
            mImgs.append(pageImg);
            rect.translate(0, pageHeight);
        }
        emit updPageCnt();
    } else if(playMode=="Scroll") {//生成一张大图
        auto isHor = direction=="left" || direction=="right";
        if(isHor) {
            doc.setHtml(text.replace("<p ", "<span ").replace("</p>", "</span>").replace("<br />", " "));
            width = ceil(doc.idealWidth()) + tailSpacing;
        } else {
            doc.setTextWidth(width);
            doc.setHtml(text);
            height = doc.size().height() + tailSpacing;
        }
        QImage img(width, height, QImage::Format_ARGB32);
        img.fill(backColor);
        {
            QPainter painter(&img);
            doc.drawContents(&painter);
        }
        if(isHor) {
            alignV(img);
            if(width < innerRect.width()) {
                auto newWidth = width*2;
                while(newWidth<innerRect.width()) newWidth += width;
                QImage newImg(newWidth, height, QImage::Format_ARGB32);
                newImg.fill(Qt::transparent);
                {
                    QPainter painter(&newImg);
                    for(int x=0; x<newWidth; x+=width) painter.drawImage(x, 0, img);
                }
                img = newImg;
            }
        }
        mImgs.clear();
        if(! useNewFmt) mImgs.append(img);
        else if(isHor) {
            if(img.width()<=4096) mImgs.append(img);
            else {
                auto rem = img.width();
                do {
                    mImgs.append(img.copy(img.width()-rem, 0, qMin(4096, rem), img.height()));
                    rem -= 4096;
                } while (rem>0);
            }
        } else {
            if(img.height()<=4096) mImgs.append(img);
            else {
                auto rem = img.height();
                do {
                    mImgs.append(img.copy(0, img.height()-rem, img.width(), qMin(4096, rem)));
                    rem -= 4096;
                } while (rem>0);
            }
        }
    } else if(playMode=="Static") {//生成一张图
        doc.setHtml(text);
        doc.setTextWidth(width);
        QImage img(width, height, QImage::Format_ARGB32);
        img.fill(backColor);
        {
            QPainter painter(&img);
            doc.drawContents(&painter);
        }
        alignV(img);
        mImgs.clear();
        mImgs.append(img);
    }
    update();
}
void EText::alignV(QImage &img, int cutHeight) {
    int width = img.width(), height = img.height();
    if(cutHeight==0) cutHeight = height;
    if(align & Qt::AlignTop) {
        int ss = 0;
        for(; ss<height; ss++) for(int i=0; i<width; i++) if(img.pixelColor(i, ss)!=backColor) goto l12;
        if(cutHeight==height) return;
        ss = 0;
        l12:
        img = copy(img, 0, ss, width, cutHeight);
    } else if(align & Qt::AlignVCenter) {
        int ss = 0, ee = height - 1;
        for(; ss<height; ss++) for(int i=0; i<width; i++) if(img.pixelColor(i, ss)!=backColor) goto l2;
        if(cutHeight==height) return;
        ss = 0;
        goto l3;
        l2:
        for(; ee>ss; ee--) for(int i=0; i<width; i++) if(img.pixelColor(i, ee)!=backColor) goto l3;
        l3:
        img = copy(img, 0, (ss+ee-cutHeight+2)/2, width, cutHeight);
    } else if(align & Qt::AlignBottom) {
        int ee = height - 1;
        for(; ee>=0; ee--) for(int i=0; i<width; i++) if(img.pixelColor(i, ee)!=backColor) goto l32;
        if(cutHeight==height) return;
        ee = height - 1;
        l32:
        img = copy(img, 0, ee+1-cutHeight, width, cutHeight);
    } else if(cutHeight != height) img = copy(img, 0, 0, width, cutHeight);
}

QImage EText::copy(QImage &img, int x, int y, int w, int h) {
    QImage imgpart(w, h, QImage::Format_ARGB32);
    imgpart.fill(backColor);
    {
        QPainter painter(&imgpart);
        painter.drawImage(-x, -y, img);
    }
    return imgpart;
}
