#include "eenviron.h"
#include "base/locolorselector.h"
#include "main.h"
#include "tools.h"
#include <QButtonGroup>
#include <QCheckBox>
#include <QFontComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QRadioButton>
#include <QSpinBox>
#include <QTextOption>
#include <QJsonArray>
#include <QToolButton>

JObj EEnviron::genProg(const JValue &json, const QString &dstDir, const QString &srcPageDir) {
    JObj res;
    res["_type"] = "EnvironmentalMonitoring";
    res["name"] = "EnvironmentalMonitoring";
    const auto items = json["items"].toObj();
    if(! items.empty()) {
        res["backColor"] = json["backColor"];
        res["bSingleScroll"] = json["isSingleLine"];
        auto scrollSpeed = json["scrollSpeed"].toInt();
        if(scrollSpeed && json["isSingleLine"].toBool()) {
            res["scrollSpeed"] = scrollSpeed;
            res["iScrollSpeed"] = 1000 / scrollSpeed;
        }
        res["timeSpan"] = json["duration"];
        res["alignType"] = json["align"];
        res["temperatureCompensation"] = json["tempCompen"];
        res["temperatureStyle"] = json["useFahrenheit"].toInt();
        QColor color = json["textColor"].toString();
        auto font = qfont(json["fontFamily"].toString(), json["fontSize"].toInt(), json["fontBold"].toBool(), json["fontItalic"].toBool());
        font.setUnderline(json["fontUnderline"].toBool());
        font.setStyleStrategy(gTextAntialiasing ? QFont::PreferAntialias : QFont::NoAntialias);
        QFontMetrics metric(font);
        res["spaceWidth"] = metric.horizontalAdvance(" ");
        auto title = json["title"].toString();
        if(! title.isEmpty()) Tools::saveImg(dstDir, metric, font, color, res, title, "title");
        JObj values;
        for(int i=0; i<=9; i++) Tools::saveImg(dstDir, metric, font, color, values, QString::number(i), QString::number(i));
        Tools::saveImg(dstDir, metric, font, color, values, ".", ".");
        Tools::saveImg(dstDir, metric, font, color, values, "-", "-");
        Tools::saveImg(dstDir, metric, font, color, values, tr("N"), "N");
        Tools::saveImg(dstDir, metric, font, color, values, tr("NE"), "NE");
        Tools::saveImg(dstDir, metric, font, color, values, tr("E"), "E");
        Tools::saveImg(dstDir, metric, font, color, values, tr("SE"), "SE");
        Tools::saveImg(dstDir, metric, font, color, values, tr("S"), "S");
        Tools::saveImg(dstDir, metric, font, color, values, tr("SW"), "SW");
        Tools::saveImg(dstDir, metric, font, color, values, tr("W"), "W");
        Tools::saveImg(dstDir, metric, font, color, values, tr("NW"), "NW");
        res["values"] = values;

        JArray oitems;
        std::unordered_map<QString, QString> unitMap;
        auto itemMap = genItemMap();
        for(auto &pair : items) {
            if(! pair.second["has"].toBool()) continue;
            oitems->push_back(JObj{{"name", pair.first}});
            auto oitem = oitems->back().toObj();
            Tools::saveImg(dstDir, metric, font, color, oitem, pair.second["label"].toString(), "label");
            auto unit = itemMap[pair.first].unit;
            if(unit.isEmpty())  continue;
            auto &url = unitMap[unit];
            if(url.isEmpty()) url = Tools::saveImg(dstDir, metric, font, color, unit);
            oitem["unit"] = url;
        }
        res["items"] = oitems;
        auto srcFile = srcPageDir+"/previewTmp.png";
        QFile srcQFile(srcFile);
        if(srcQFile.exists()) {
            auto id = Tools::fileMd5(srcFile);
            srcQFile.copy(dstDir+"/"+id);
            res["previewImg"] = id;
        }
    }
    return res;
}

EEnviron::EEnviron(EBase *multiWin): EBase(multiWin) {
    mType = EBase::Environ;
    for(auto &item : itemMap) item.second.label = item.second.text+": ";
    calAttr();
}

EEnviron::EEnviron(const JObj &json, EBase *multiWin): EBase(multiWin){
    mType = EBase::Environ;
    setBaseAttr(json);
    auto items = json["items"].toObj();
    if(! items.empty()) {
        for(auto &jitem : items) {
            auto iter = itemMap.find(jitem.first);
            if(iter!=itemMap.end()) {
                iter->second.label = jitem.second["label"].toString();
                iter->second.has = jitem.second["has"].toBool();
            }
        }
        backColor = json["backColor"].toStr("#00000000");
        textColor = json["textColor"].toStr("#ff000000");
        tempCompen = json["tempCompen"].toInt();
        useFahrenheit = json["useFahrenheit"].toInt();
        title = json["title"].toStr();
        isSingleLine = json["isSingleLine"].toBool();
        scrollSpeed = json["scrollSpeed"].toInt();
        align = json["align"].toInt();
        font = qfont(json["fontFamily"].toStr(), json["fontSize"].toInt(), json["fontBold"].toBool(), json["fontItalic"].toBool());
        font.setUnderline(json["fontUnderline"].toBool());
    } else {
        auto widget = json["widget"];
        auto &temp = itemMap["temp"];
        temp.label = widget["labelTemperature"].toStr();
        temp.has = widget["bTemperature"].toBool();
        auto &humidity = itemMap["humidity"];
        humidity.label = widget["labelHumidity"].toStr();
        humidity.has = widget["bHumidity"].toBool();
        auto &noise = itemMap["noise"];
        noise.label = widget["labelNoise"].toStr();
        noise.has = widget["bNoise"].toBool();
        auto &windSpeed = itemMap["windSpeed"];
        windSpeed.label = widget["labelWindSpeed"].toStr();
        windSpeed.has = widget["bWindSpeed"].toBool();
        auto &windDirection = itemMap["windDirection"];
        windDirection.label = widget["labelWindDirection"].toStr();
        windDirection.has = widget["bWindDirection"].toBool();
        auto &PM25 = itemMap["PM2.5"];
        PM25.label = widget["labelPM25"].toStr();
        PM25.has = widget["bPM25"].toBool();
        auto &PM10 = itemMap["PM10"];
        PM10.label = widget["labelPM10"].toStr();
        PM10.has = widget["bPM10"].toBool();
        for(auto item = itemMap.find("SO₂"); item!=itemMap.end(); ++item) {
            item->second.label = item->second.text+": ";
            item->second.has = false;
        }
        backColor = Tools::int2Color(widget["cBackground"].toInt());
        textColor = Tools::int2Color(widget["textColor"].toInt());
        tempCompen = widget["temperatureCompensation"].toInt();
        useFahrenheit = widget["temperatureStyle"].toBool();
        title = widget["labelTitle"].toStr();
        isSingleLine = widget["bPaomadeng"].toBool();
        scrollSpeed = widget["scrollSpeed"].toInt();
        align = widget["alignType"].toInt();
        font = qfont(widget["fontFamily"].toStr(), widget["fontSize"].toInt(), widget["fontBold"].toBool(), widget["fontItalics"].toBool());
        font.setUnderline(widget["fontUnderline"].toBool());
    }
    calAttr();
}

void EEnviron::calAttr() {
    item_cnt = 0;
    if(! title.isEmpty()) item_cnt++;
    for(auto &item : itemMap) if(item.second.has) item_cnt++;
    font.setStyleStrategy(gTextAntialiasing ? QFont::PreferAntialias : QFont::NoAntialias);
    if(isSingleLine) {
        scroll_txt = "";
        if(! title.isEmpty()) scroll_txt += title + " ";
        for(auto &item : itemMap) if(item.second.has) {
            scroll_txt += item.second.label;
            if(item.first=="temp") scroll_txt += QString::number(tempCompen);
            else if(item.first=="windDirection") scroll_txt += "--";
            else scroll_txt += "0";
            scroll_txt += item.second.unit + " ";
        }
        scroll_width = QFontMetrics(font).horizontalAdvance(scroll_txt);
    }
}
void EEnviron::drawText(QPainter *painter, QRectF& rect) {
    QTextOption opt(Qt::AlignLeft | Qt:: AlignVCenter);
    painter->setFont(font);
    painter->setPen(textColor);
    if(isSingleLine) {
        opt.setWrapMode(QTextOption::NoWrap);
        painter->drawText(rect, scroll_txt, opt);
    } else {
        if(align==0) opt.setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
        else if(align==1) opt.setAlignment(Qt::AlignCenter);
        else if(align==2) opt.setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        if(item_cnt > 0) rect.setHeight(rect.height() / item_cnt);
        if(! title.isEmpty()) {
            painter->drawText(rect, title, opt);
            rect.translate(0, rect.height());
        }
        for(auto &itemPair : itemMap) if(itemPair.second.has) {
            auto text = itemPair.second.label;
            if(itemPair.first=="temp") text += QString::number(tempCompen);
            else if(itemPair.first=="windDirection") text += "--";
            else text += "0";
            text += itemPair.second.unit;
            painter->drawText(rect, text, opt);
            rect.translate(0, rect.height());
        }
    }
}
void EEnviron::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    if(isSingleLine) {
        if(timer_id==0) {
            scroll_off = innerRect().width();
            timer_id = startTimer(1000/scrollSpeed, Qt::PreciseTimer);
        }
    } else {
        if(timer_id!=0) {
            killTimer(timer_id);
            timer_id = 0;
        }
    }
    painter->setClipRect(0, 0, mWidth, mHeight);
    painter->fillRect(0, 0, mWidth, mHeight, backColor);
    auto inner = innerRect();
    if(isSingleLine) {
        inner.moveLeft(scroll_off);
        inner.setWidth(scroll_width);
    }
    drawText(painter, inner);
    painter->setClipping(false);
    EBase::paint(painter, option, widget);
}
void EEnviron::timerEvent(QTimerEvent *e) {
    if(e->timerId() == timer_id) {
        if(isVisible()) {
            if(scroll_off < -scroll_width) scroll_off = mWidth;
            else scroll_off--;
            update();
        } else if(timer_id!=0) {
            killTimer(timer_id);
            timer_id = 0;
        }
    } else EBase::timerEvent(e);
}

QWidget* EEnviron::attrWgt() {
    auto wgtAttr = new QWidget;
    auto vBox = new VBox(wgtAttr);
    vBox->setContentsMargins(6, 0, 6, 0);
    vBox->setSpacing(3);

    addBaseAttrWgt(vBox);

    auto hBox = new HBox(vBox);
    hBox->addLabel(translate("","Basic Properties"));

    auto line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    hBox->addWidget(line, 1);

    hBox = new HBox(vBox);
    hBox->addSpacing(6);

    auto fdTitle = new QLineEdit(title);
    fdTitle->setPlaceholderText(tr("Title"));
    connect(fdTitle, &QLineEdit::textChanged, this, [this](const QString &text) {
        title = text;
        calAttr();
        update();
    });
    hBox->addWidget(fdTitle);

    for(auto &itemPair : itemMap) {
        hBox = new HBox(vBox);
        hBox->addSpacing(6);
        auto *item = &itemPair.second;
        bool isTemp = itemPair.first=="temp";
        auto text = item->text;
        if(! item->unit.isEmpty()) text += " ("+item->unit+")";
        auto fd = new QCheckBox(text);
        fd->setChecked(item->has);
        connect(fd, &QCheckBox::toggled, wgtAttr, [=](bool checked) {
            item->has = checked;
            calAttr();
            update();
        });
        hBox->addWidget(fd);
        hBox->addStretch();

        if(isTemp) {
            auto fdIsCelsius = new QRadioButton("℃");
            fdIsCelsius->setChecked(true);
            hBox->addWidget(fdIsCelsius);

            auto fdIsFahrenheit = new QRadioButton("℉");
            connect(fdIsFahrenheit, &QRadioButton::toggled, this, [this](bool checked) {
                useFahrenheit = checked;
                calAttr();
                update();
            });
            hBox->addWidget(fdIsFahrenheit);
            hBox->addStretch();

            auto grp = new QButtonGroup(wgtAttr);
            grp->addButton(fdIsCelsius);
            grp->addButton(fdIsFahrenheit);

            if(useFahrenheit) fdIsFahrenheit->setChecked(true);
        }
        auto fdLabel = new QLineEdit(item->label);
        fdLabel->setMaximumWidth(100);
        connect(fdLabel, &QLineEdit::textChanged, this, [=](const QString &text) {
            item->label = text;
            calAttr();
            update();
        });
        hBox->addWidget(fdLabel);

        if(isTemp) {
            hBox = new HBox(vBox);
            hBox->addStretch();

            hBox->addWidget(new QLabel(tr("Compensation")));

            auto fdCompen = new QSpinBox;
            fdCompen->setRange(-99, 999);
            fdCompen->setValue(tempCompen);
            connect(fdCompen, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, [=](int value) {
                tempCompen = value;
                calAttr();
                update();
            });
            hBox->addWidget(fdCompen);
        }
    }

    auto wgtAlign = new QWidget;
    vBox->addWidget(wgtAlign);
    hBox = new HBox(wgtAlign);
    hBox->setContentsMargins(6,0,0,0);
    hBox->addStretch();

    auto fdLeft = new QRadioButton(tr("Left"));
    fdLeft->setChecked(true);
    connect(fdLeft, &QRadioButton::toggled, this, [this](bool checked) {
        if(! checked) return;
        align = 0;
        calAttr();
        update();
    });
    hBox->addWidget(fdLeft);

    auto fdCenter = new QRadioButton(tr("Center"));
    connect(fdCenter, &QRadioButton::toggled, this, [this](bool checked) {
        if(! checked) return;
        align = 1;
        calAttr();
        update();
    });
    hBox->addWidget(fdCenter);

    auto fdRight = new QRadioButton(tr("Right"));
    connect(fdRight, &QRadioButton::toggled, this, [this](bool checked) {
        if(! checked) return;
        align = 2;
        calAttr();
        update();
    });
    hBox->addWidget(fdRight);
    hBox->addStretch();


    if(isSingleLine) wgtAlign->setVisible(false);
    if(align == 0) fdLeft->setChecked(true);
    else if(align == 1) fdCenter->setChecked(true);
    else if(align == 2) fdRight->setChecked(true);

    hBox = new HBox(vBox);
    hBox->addSpacing(6);

    auto fdIsScroll = new QCheckBox(tr("Single scroll"));
    fdIsScroll->setChecked(isSingleLine);
    connect(fdIsScroll, &QCheckBox::toggled, this, [this, wgtAlign](bool checked) {
        isSingleLine = checked;
        wgtAlign->setVisible(! checked);
        if(timer_id!=0) {
            killTimer(timer_id);
            timer_id = 0;
        }
        calAttr();
        update();
    });
    hBox->addWidget(fdIsScroll);
    hBox->addStretch();

    hBox->addWidget(new QLabel(tr("Speed")));

    auto fdSpeed = new QSpinBox();
    fdSpeed->setRange(1, 999);
    fdSpeed->setValue(scrollSpeed);
    connect(fdSpeed, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, [this](int value) {
        scrollSpeed = value;
        if(timer_id!=0) {
            killTimer(timer_id);
            timer_id = 0;
        }
        calAttr();
        update();
    });
    hBox->addWidget(fdSpeed);

    hBox->addLabel("px/s");
    hBox->addStretch();

    line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    vBox->addWidget(line);

    hBox = new HBox(vBox);
    hBox->addSpacing(6);

    auto fdFontFamily = new QFontComboBox();
    fdFontFamily->setEditable(false);
    fdFontFamily->setCurrentText(font.family());
    connect(fdFontFamily, &QFontComboBox::currentFontChanged, this, [this](const QFont &f) {
        auto ft = qfont(f.family(), font.pixelSize(), font.bold(), font.italic());
        ft.setUnderline(font.underline());
        font = ft;
        calAttr();
        update();
    });
    hBox->addWidget(fdFontFamily);

    auto fdFontSize = new QSpinBox();
    fdFontSize->setRange(4, 9999);
    fdFontSize->setValue(font.pixelSize());
    connect(fdFontSize, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, [this](int value) {
        font.setPixelSize(value);
        calAttr();
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
    fdFontBold->setChecked(font.bold());
    connect(fdFontBold, &QPushButton::toggled, this, [this](bool checked) {
        font.setBold(checked);
        calAttr();
        update();
    });
    hBox->addWidget(fdFontBold);

    auto fdFontItalic = new QPushButton("I");
    fdFontItalic->setStyleSheet("QPushButton{background: #bbb; color: #888; font-size: 20px; font-style: italic;} QPushButton:checked{background: #29c; color: #fff;}");
    fdFontItalic->setFixedSize(30, 30);
    fdFontItalic->setCheckable(true);
    fdFontItalic->setChecked(font.italic());
    connect(fdFontItalic, &QPushButton::toggled, this, [this](bool checked) {
        font.setItalic(checked);
        calAttr();
        update();
    });
    hBox->addWidget(fdFontItalic);

    auto fdFontUnderline = new QPushButton("U");
    fdFontUnderline->setStyleSheet("QPushButton{background: #bbb; color: #888; font-size: 20px; text-decoration: underline;} QPushButton:checked{background: #29c; color: #fff;}");
    fdFontUnderline->setFixedSize(30, 30);
    fdFontUnderline->setCheckable(true);
    fdFontUnderline->setChecked(font.underline());
    connect(fdFontUnderline, &QPushButton::toggled, this, [this](bool checked) {
        font.setUnderline(checked);
        calAttr();
        update();
    });
    hBox->addWidget(fdFontUnderline);

    auto fdColor = new LoColorSelector("T", textColor);
    fdColor->setFixedWidth(30);
    connect(fdColor, &LoColorSelector::sColorChanged, this, [this](const QColor &color) {
        textColor = color;
        calAttr();
        update();
    });
    hBox->addWidget(fdColor);

    auto fdBack = new LoColorSelector(tr("Back Color"), backColor);
    connect(fdBack, &LoColorSelector::sColorChanged, this, [this](const QColor &color) {
        backColor = color;
        calAttr();
        update();
    });
    hBox->addWidget(fdBack);
    hBox->addStretch();
    vBox->addStretch();
    return wgtAttr;
}

JObj EEnviron::attrJson() const{
    JObj json;
    addBaseAttr(json);
    json["elementType"] = "Temp";
    json["title"] = title;
    JObj items;
    for(auto &item : itemMap) items[item.first] = JObj{
        {"has", item.second.has},
        {"label", item.second.label}
    };
    json["items"] = items;
    json["tempCompen"] = tempCompen;
    json["useFahrenheit"] = useFahrenheit;
    json["isSingleLine"] = isSingleLine;
    json["scrollSpeed"] = scrollSpeed;
    json["align"] = align;
    json["fontFamily"] = font.family();
    json["fontSize"] = font.pixelSize();
    json["fontBold"] = font.bold();
    json["fontItalic"] = font.italic();
    json["fontUnderline"] = font.underline();
    json["textColor"] = textColor.name(QColor::HexArgb);
    json["backColor"] = backColor.name(QColor::HexArgb);
    json["idDir"] = QString("env-%1-%2-%3-%4-%5").arg((int) zValue()).arg((int) x()).arg((int) y()).arg((int) mWidth).arg((int) mHeight);
    return json;
}

bool EEnviron::save(const QString &pageDir) {
    auto inner = innerRect();
    QPixmap img(isSingleLine ? scroll_width : inner.width(), inner.height());
    img.fill(backColor);
    QPainter painter(&img);
    QRectF arect(0, 0, img.width(), img.height());
    drawText(&painter, arect);
    img.save(pageDir+"/previewTmp.png", "PNG");
    return true;
}
