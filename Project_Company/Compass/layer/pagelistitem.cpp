#include "pagelistitem.h"
#include "main.h"
#include "gutil/qgui.h"
#include "base/ffutil.h"
#include "base/calendarbutton.h"
#include "program/eaclock.h"
#include "program/ebase.h"
#include "program/edclock.h"
#include "program/eenviron.h"
#include "program/egif.h"
#include "program/emultiwin.h"
#include "program/ephoto.h"
#include "program/etext.h"
#include "program/etimer.h"
#include "program/etimer2.h"
#include "program/evideo.h"
#include "program/eweb.h"
#include <QDateEdit>
#include <QDir>
#include <QGraphicsView>
#include <QLineEdit>
#include <QMessageBox>
#include <QSpinBox>
#include <QFileDialog>

class PageScene : public QGraphicsScene {
public:
    using QGraphicsScene::QGraphicsScene;
    void drawBackground(QPainter *painter, const QRectF &rect) {
        QGraphicsScene::drawBackground(painter, rect);
        painter->fillRect(sceneRect(), Qt::black);
    }
};

PageListItem::PageListItem(const JObj &attr, const QString &pageDir) : mAttr(attr), mPageDir(pageDir) {
    scale = qMin(128.0 / gProgItem->mWidth, 96.0 / gProgItem->mHeight);
    viewW = scale * gProgItem->mWidth;
    viewH = scale * gProgItem->mHeight;
    setSizeHint(QSize(0, viewH+20));
    mScene = new PageScene(0, 0, gProgItem->mWidth, gProgItem->mHeight, this);
    auto elements = mAttr["elements"].toArray();
    for(auto &ele : elements) {
        auto type = ele["elementType"].toStr();
        EBase *element = 0;
        if(type=="Text") element = new EText(ele.toObj());
        else if(type=="Image"||type=="Photo") element = EPhoto::create(ele.toObj(), this);
        else if(type=="Gif") element = EGif::create(ele.toObj(), this);
        else if(type=="Video"||type=="Movie") element = EVideo::create(ele.toObj(), this);
        else if(type=="DClock") element = new EDClock(ele.toObj());
        else if(type=="AClock") element = new EAClock(ele.toObj());
        else if(type=="Temp") element = new EEnviron(ele.toObj());
        else if(type=="Web") element = new EWeb(ele.toObj());
        else if(type=="Timer") element = new ETimer(ele.toObj());
        else if(type=="Timer2") element = new ETimer2(ele.toObj());
        else if(type=="Window") element = new EMultiWin(ele.toObj(), this);
        if(element) mScene->addItem(element);
    }
}

//更新页属性参数到page.json
void PageListItem::updateJson() {
    if(mAttrWgt==0) return; //没有点开该页面进行编辑
    JArray elements;
    auto items = mScene->items();
    for(auto &item : items) {
        auto element = (EBase*) item;
        if(element->mMultiWin == 0) elements << element->attrJson();
    }
    mAttr["elements"] = elements;

    auto cnt = mAudiosList->count();
    if(cnt==0) mAttr.erase("audios");
    else {
        JArray audios;
        for(int i=0; i<cnt; i++) {
            auto info = mAudiosList->item(i)->data(Qt::UserRole).value<AudioInfo>();
            audios << JObj{
                {"dir", info.dir},
                {"name", info.name},
                {"dur", info.dur},
                {"vol", info.vol}
            };
        }
        mAttr["audios"] = audios;
    }
}
bool PageListItem::saveFiles() {
    mPageDir = gProgItem->mProgDir + "/" + mAttr["name"].toString();
    QDir pageQDir(mPageDir);
    if(! pageQDir.exists() && ! pageQDir.mkpath(mPageDir)) return false;

    JArray elements;
    auto items = mScene->items();
    for(auto &item : items) {
        auto element = (EBase*) item;
        if(element->mMultiWin == 0 && element->save(mPageDir)) elements << element->attrJson();
    }
    mAttr["elements"] = elements;
    if(mAttrWgt) {
        auto cnt = mAudiosList->count();
        if(cnt==0) mAttr.erase("audios");
        else {
            JArray audios;
            for(int i=0; i<cnt; i++) {
                auto info = mAudiosList->item(i)->data(Qt::UserRole).value<AudioInfo>();
                QString oldFile = info.dir + PAGEDEL_SUFFIX + "/" + info.name;
                if(QFileInfo::exists(oldFile)) ;
                else if(QFileInfo::exists(oldFile = info.dir + "/" + info.name)) ;
                else continue;
                QFile::copy(oldFile, mPageDir + "/" + info.name);
                info.dir = mPageDir;
                audios << JObj{
                    {"dir", info.dir},
                    {"name", info.name},
                    {"dur", info.dur},
                    {"vol", info.vol}
                };
            }
            mAttr["audios"] = audios;
        }
    }

    QFile file(mPageDir + "/page.json");
    if(! file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(listWidget(), "Write Error", mPageDir + "/page.json "+file.errorString());
        return false;
    }
    file.write(JToBytes(mAttr, "\t"));
    file.close();
    return true;
}

QWidget *PageListItem::itemWgt() {
    auto wgtPage = new QWidget;
    auto grid = new Grid(wgtPage);
    grid->setContentsMargins(0,0,0,0);
    grid->setSpacing(0);

    grid->addWidget(fdIdx = new QLabel, 0, 0);
    fdIdx->setMinimumWidth(32);
    fdIdx->setAlignment(Qt::AlignCenter);
    fdIdx->setNum(listWidget()->count());

    mGraView = new QGraphicsView;
    mGraView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mGraView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mGraView->setEnabled(false);
    mGraView->setFrameShape(QFrame::NoFrame);
    mGraView->setFixedSize(viewW, viewH);
    mGraView->setStyleSheet("QGraphicsView{background-color:transparent;}");
    mGraView->setScene(mScene);
    mGraView->scale(scale, scale);
    grid->addWidget(mGraView, 0, 1);

    auto hBox = new HBox;
    hBox->setSpacing(4);
    grid->addLayout(hBox, 1, 1);
    grid->setColumnStretch(2, 1);
    grid->setRowStretch(2, 1);

    hBox->addWidget(fdPlayTimes = new QLabel(QString::number(mAttr["repeat"].toInt())));
    gFont(fdPlayTimes, 12);

    auto fdTimes = new QLabel(tr("times"));
    gFont(fdTimes, 12);
    hBox->addWidget(fdTimes);
    hBox->addStretch();

    return wgtPage;
}

QWidget *PageListItem::attrWgt() {
    if(mAttrWgt) return mAttrWgt;
    mAttrWgt = new QWidget();
    mAttrWgt->setStyleSheet(R"rrr(
QPushButton {
    padding-left: 4px;
    padding-right: 4px;
    padding-top: 2px;
    padding-bottom: 2px;
    border-radius: 4px;
}
QPushButton:hover {
    background-color: #dddddd;
}

QPushButton#bnAddPlan {
    image: url(:/res/program/AddPlan.png);
    width: 24;
    height: 24;
}
QPushButton#bnClearPlan {
    image: url(:/res/program/Clean.png);
    width: 24;
    height: 24;
}
QPushButton#bnAddPlan:pressed,
QPushButton#bnClearPlan:pressed,
QPushButton[ssName="weeklySelector"] {
    border: 1px solid #6A838F;
    width: 24;
    height: 24;
}
QPushButton[ssName="weeklySelector"]:checked {
    background-color: #0099cc;
}
)rrr");
    auto vBox = new QVBoxLayout(mAttrWgt);
    vBox->setContentsMargins(6, 0, 6, 0);
    vBox->setSpacing(4);

    auto hBox = new QHBoxLayout();
    auto lb1 = new QLabel(tr("Page name"));
    lb1->setMinimumWidth(80);
    hBox->addWidget(lb1);

    auto fdPageName = new QLineEdit(mAttr["name"].toString());
    connect(fdPageName, &QLineEdit::textEdited, this, [this, fdPageName](const QString &text) {
        bool isDupli = false;
        auto listWgt = listWidget();
        if(listWgt) {
            for(int i=0; i<listWgt->count(); i++) {
                auto item = static_cast<PageListItem*>(listWgt->item(i));
                if(text == item->mAttr["name"].toString()) {
                    isDupli = true;
                    break;
                }
            }
        }
        if(isDupli) {
            QString newName = tr("New") + QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
            mAttr["name"] = newName;
            fdPageName->setText(newName);
        } else mAttr["name"] = text;
    });
    hBox->addWidget(fdPageName);

    vBox->addLayout(hBox);

    hBox = new QHBoxLayout();
    lb1 = new QLabel(tr("Play times"));
    lb1->setMinimumWidth(80);
    hBox->addWidget(lb1);

    auto fdPlayTimes = new QSpinBox();
    fdPlayTimes->setRange(1, 9999);
    fdPlayTimes->setValue(mAttr["repeat"].toInt());
    connect(fdPlayTimes, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, [this](int value) {
        mAttr["repeat"] = value;
        this->fdPlayTimes->setText(QString::number(value));
    });
    hBox->addWidget(fdPlayTimes);
    hBox->addSpacing(20);

    auto fdLoop = new QCheckBox(tr("Sources Repeat"));
    fdLoop->setChecked(mAttr["loop"].toBool());
    connect(fdLoop, &QCheckBox::toggled, this, [this](bool checked) {
        mAttr["loop"] = checked;
    });
    hBox->addWidget(fdLoop);

    auto edWaitAudio = new QCheckBox(tr("Wait Audio"));
    edWaitAudio->setChecked(mAttr["waitAudio"].toBool());
    connect(edWaitAudio, &QCheckBox::toggled, this, [this](bool checked) {
        mAttr["waitAudio"] = checked;
    });
    hBox->addWidget(edWaitAudio);

    hBox->addStretch();

    vBox->addLayout(hBox);

    auto line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    vBox->addWidget(line);

    hBox = new QHBoxLayout();
    auto lb = new QLabel();
    lb->setPixmap(QPixmap(":/res/program/Audio.png").scaledToHeight(24, Qt::SmoothTransformation));
    hBox->addWidget(lb);
    hBox->addWidget(new QLabel(tr("Audios")));
    hBox->addStretch();

    hBox->addWidget(new QLabel(tr("Total Dur")));
    auto fdTtlDur = new QLabel("0");
    hBox->addWidget(fdTtlDur);
    hBox->addWidget(new QLabel(tr("s")));
    hBox->addStretch();

    auto btnAdd = new QPushButton();
    btnAdd->setIcon(QIcon(":/res/program/Add.png"));
    btnAdd->setProperty("style","multiTool");

    mAudiosList = new QListWidget;
    connect(btnAdd, &QPushButton::clicked, mAttrWgt, [this, fdTtlDur] {
        auto files = QFileDialog::getOpenFileNames(mAttrWgt, translate("","Select File"), gFileHome);
        int durs = fdTtlDur->text().toInt();
        for(int i=0; i<files.count(); i++) {
            int64_t dur;
            QString err = audioInfo(files[i].toUtf8(), &dur);
            if(! err.isEmpty()) {
                QMessageBox::critical(mAttrWgt, "Audio Error", err+"\n"+files[i]);
                continue;
            }
            QFileInfo fInfo(files[i]);
            AudioInfo info{fInfo.absolutePath(), fInfo.fileName(), (int)round(dur*0.000001)};
            auto item = new QListWidgetItem(QIcon(":/res/program/Audio.png"), info.name);
            item->setData(Qt::UserRole, QVariant::fromValue(info));
            mAudiosList->addItem(item);
            if(i == files.count()-1) {
                mAudiosList->setCurrentItem(item);
                gFileHome = fInfo.absolutePath();
            }
            durs += info.dur;
            mAudiosList->addItem(item);
        }
        fdTtlDur->setText(QString::number(durs));
    });
    hBox->addWidget(btnAdd);

    auto btnDel = new QPushButton();
    btnDel->setIcon(QIcon(":/res/program/Delete.png"));
    btnDel->setProperty("style","multiTool");
    connect(btnDel, &QPushButton::clicked, this, [this, fdTtlDur] {
        auto row = mAudiosList->currentRow();
        if(row < 0) return;
        int durs = fdTtlDur->text().toInt();
        auto item = mAudiosList->takeItem(row);
        auto info = static_cast<AudioInfo>(item->data(Qt::UserRole).value<AudioInfo>());
        durs -= info.dur;
        delete item;
        fdTtlDur->setText(QString::number(durs));
    });
    hBox->addWidget(btnDel);

    auto btnGoUp = new QPushButton();
    btnGoUp->setIcon(QIcon(":/res/program/GoUp.png"));
    btnGoUp->setProperty("style","multiTool");
    connect(btnGoUp, &QPushButton::clicked, this, [this] {
        int row = mAudiosList->currentRow();
        if(row < 1) return;
        mAudiosList->insertItem(row-1, mAudiosList->takeItem(row));
        mAudiosList->setCurrentRow(row-1);
    });
    hBox->addWidget(btnGoUp);

    auto btnGoDown = new QPushButton();
    btnGoDown->setIcon(QIcon(":/res/program/GoDown.png"));
    btnGoDown->setProperty("style","multiTool");
    connect(btnGoDown, &QPushButton::clicked, this, [this] {
        int row = mAudiosList->currentRow();
        if(row < 0 || row > mAudiosList->count() - 2) return;
        mAudiosList->insertItem(row+1, mAudiosList->takeItem(row));
        mAudiosList->setCurrentRow(row+1);
    });
    hBox->addWidget(btnGoDown);

    vBox->addLayout(hBox);

    mAudiosList->setMinimumHeight(120);
    mAudiosList->setIconSize(QSize(20, 20));
    vBox->addWidget(mAudiosList, 1);

    auto audios = mAttr("audios");
    int durs = 0;
    for(auto &audio : audios) {
        AudioInfo info{audio["dir"].toString(), audio["name"].toString(), audio["dur"].toInt(), audio["vol"].toInt()};
        if(info.dir.isEmpty() || info.name.isEmpty() || info.dur==0) continue;
        auto item = new QListWidgetItem(QIcon(":/res/program/Audio.png"), info.name);
        item->setData(Qt::UserRole, QVariant::fromValue(info));
        durs += info.dur;
        mAudiosList->addItem(item);
    }
    fdTtlDur->setText(QString::number(durs));

    auto wgtAudioInfo = new QWidget();
    wgtAudioInfo->hide();
    hBox = new QHBoxLayout(wgtAudioInfo);
    hBox->setContentsMargins(0,0,0,0);
    hBox->setSpacing(2);

    hBox->addWidget(new QLabel(tr("Duration")));
    hBox->addSpacing(4);

    auto fdAudioDur = new QLabel("0");
    hBox->addWidget(fdAudioDur);
    hBox->addWidget(new QLabel(tr("s")));

    hBox->addStretch();
    hBox->addWidget(new QLabel(tr("Vol")));

    auto fdVol = new QSlider(Qt::Horizontal);
    fdVol->setMaximum(100);
    fdVol->setMinimumWidth(120);
    hBox->addWidget(fdVol);

    auto fdAudioVol = new QLabel("0");
    fdAudioVol->setAlignment(Qt::AlignRight);
    fdAudioVol->setFixedWidth(38);
    hBox->addWidget(fdAudioVol);

    vBox->addWidget(wgtAudioInfo);

    connect(fdVol, &QSlider::valueChanged, this, [this, fdAudioVol](int value) {
        fdAudioVol->setText(QString::number(value)+"%");
        auto item = mAudiosList->currentItem();
        if(item==nullptr) return;
        auto info = static_cast<AudioInfo>(item->data(Qt::UserRole).value<AudioInfo>());
        info.vol = value;
        item->setData(Qt::UserRole, QVariant::fromValue(info));
    });
    connect(mAudiosList, &QListWidget::currentItemChanged, this, [fdAudioDur, wgtAudioInfo, fdVol, fdAudioVol](QListWidgetItem *current, QListWidgetItem *) {
        if(current==nullptr) {
            wgtAudioInfo->hide();
            return;
        }
        wgtAudioInfo->show();
        auto info = static_cast<AudioInfo>(current->data(Qt::UserRole).value<AudioInfo>());
        fdAudioDur->setText(QString::number(info.dur));
        fdVol->setValue(info.vol);
        fdAudioVol->setText(QString::number(info.vol)+"%");
    });

    line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    vBox->addWidget(line);

    auto validDate = mAttr["validDate"];

    auto wValidDate = new QCheckBox(tr("Valid Date"));
    bool isDateValid = validDate["isValid"].toBool();
    wValidDate->setChecked(isDateValid);
    vBox->addWidget(wValidDate);

    hBox = new QHBoxLayout();

    auto fdStart = new QDateEdit();
    fdStart->setDisplayFormat("yyyy-MM-dd");
    fdStart->setEnabled(isDateValid);
    fdStart->setDate(QDate::fromString(validDate["start"].toString(), "yyyy-MM-dd"));
    hBox->addWidget(fdStart);

    auto bnDateStart = new CalendarButton;
    bnDateStart->setEnabled(isDateValid);
    connect(bnDateStart->calendar, &QCalendarWidget::clicked, fdStart, &QDateEdit::setDate);
    hBox->addWidget(bnDateStart);

    hBox->addWidget(new QLabel("～"));

    auto fdEnd = new QDateEdit();
    fdEnd->setDisplayFormat("yyyy-MM-dd");
    fdEnd->setEnabled(isDateValid);
    fdEnd->setDate(QDate::fromString(validDate["end"].toString(), "yyyy-MM-dd"));
    connect(fdStart, &QDateEdit::dateChanged, this, [this, wValidDate, fdStart, fdEnd](const QDate &date) {
        auto end = fdEnd->date();
        if(! date.isValid() || date > end) {
            QMessageBox::warning(mAttrWgt, tr("Warning"), tr("Start Time can't be later than End Time"));
            fdStart->setDate(end);
        }
        mAttr["validDate"] = JObj{
            {"isValid", wValidDate->isChecked()},
            {"start", fdStart->date().toString("yyyy-MM-dd")},
            {"end", end.toString("yyyy-MM-dd")}
        };
    });
    connect(fdEnd, &QDateEdit::dateChanged, mAttrWgt, [this, wValidDate, fdStart, fdEnd](const QDate &date) {
        QDate start = fdStart->date();
        if(! date.isValid() || start > date) {
            QMessageBox::warning(mAttrWgt, tr("Warning"), tr("End Time can't be earlier than Start Time"));
            fdEnd->setDate(start);
        }
        mAttr["validDate"] = JObj{
            {"isValid", wValidDate->isChecked()},
            {"start", start.toString("yyyy-MM-dd")},
            {"end", fdEnd->date().toString("yyyy-MM-dd")}
        };
    });
    hBox->addWidget(fdEnd);

    auto bnDateEnd = new CalendarButton;
    bnDateEnd->setEnabled(isDateValid);
    connect(bnDateEnd->calendar, &QCalendarWidget::clicked, fdEnd, &QDateEdit::setDate);
    hBox->addWidget(bnDateEnd);

    hBox->addStretch();

    connect(wValidDate, &QCheckBox::toggled, this, [this, fdStart, fdEnd, bnDateStart, bnDateEnd](bool checked) {
        fdStart->setEnabled(checked);
        fdEnd->setEnabled(checked);
        bnDateStart->setEnabled(checked);
        bnDateEnd->setEnabled(checked);
        mAttr["validDate"] = JObj{
            {"isValid", checked},
            {"start", fdStart->date().toString("yyyy-MM-dd")},
            {"end", fdEnd->date().toString("yyyy-MM-dd")}
        };
    });

    vBox->addLayout(hBox);

    line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    vBox->addWidget(line);

    hBox = new QHBoxLayout();
    hBox->setSpacing(12);
    hBox->addWidget(new QLabel(tr("Plan")));

    hBox->addStretch();

    auto bnAddPlan = new QPushButton();
    bnAddPlan->setObjectName("bnAddPlan");
    hBox->addWidget(bnAddPlan);

    auto bnClearPlan = new QPushButton;
    bnClearPlan->setObjectName("bnClearPlan");
    hBox->addWidget(bnClearPlan);

    vBox->addLayout(hBox);

    auto listPlan = new QListWidget;
    connect(bnClearPlan, &QPushButton::clicked, listPlan, [=] {
        listPlan->clear();
        mAttr.erase("plans");
    });
    auto planChanged = [this, listPlan] {
        if(listPlan->count()==0) mAttr.erase("plans");
        else {
            JArray plans;
            for(int i=0; i<listPlan->count(); i++) {
                auto item = (PlanItemWgt*) listPlan->itemWidget(listPlan->item(i));
                JArray weekly;
                for(int i=0; i<7; ++i) if(item->btnDays[i]->isChecked()) weekly.append(i+1);
                plans.append(JObj{
                    {"start", item->tStart->time().toString("hh:mm")},
                    {"end", item->tEnd->time().toString("hh:mm")},
                    {"weekly", weekly}
                });
            }
            mAttr["plans"] = plans;
        }
    };
    auto plans = mAttr("plans");
    for(int i=0; i<(int)plans.size(); i++) {
        QListWidgetItem *item = new QListWidgetItem(listPlan);
        item->setSizeHint({1, 80});
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        auto json = plans[i].toObj();
        auto widget = new PlanItemWgt(item, &json);
        connect(widget, &PlanItemWgt::sigItemChanged, listPlan, planChanged);
    }
    connect(bnAddPlan, &QPushButton::clicked, listPlan, [=] {
        QListWidgetItem *item = new QListWidgetItem(listPlan);
        item->setSizeHint({1, 80});
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        auto widget = new PlanItemWgt(item);
        connect(widget, &PlanItemWgt::sigItemChanged, listPlan, planChanged);
        planChanged();
    });
    vBox->addWidget(listPlan, 2);
    return mAttrWgt;
}


PlanItemWgt::PlanItemWgt(QListWidgetItem *item, const JObj *json) : m_item(item) {
    auto vBox = new VBox(this);
    vBox->setContentsMargins(6, 6, 6, 0);
    auto hBox = new HBox(vBox);

    fdIdx = new QLabel;
    fdIdx->setNum(item->listWidget()->count());
    hBox->addWidget(fdIdx);
    hBox->addStretch();

    tStart = new QTimeEdit;
    tStart->setDisplayFormat("HH:mm");
    tStart->setMinimumWidth(80);
    auto ft = tStart->font();
    ft.setPixelSize(14);
    tStart->setFont(ft);
    connect(tStart, &QTimeEdit::timeChanged, this, &PlanItemWgt::sigItemChanged);
    hBox->addWidget(tStart);
    hBox->addStretch();

    tEnd = new QTimeEdit(QTime(23, 59));
    tEnd->setDisplayFormat("HH:mm");
    tEnd->setMinimumWidth(80);
    tEnd->setFont(ft);
    connect(tEnd, &QTimeEdit::timeChanged, this, &PlanItemWgt::sigItemChanged);
    hBox->addWidget(tEnd);
    hBox->addStretch();

    bnDel = new QPushButton;
    bnDel->setIcon(QIcon(":/res/program/Delete.png"));
    bnDel->setFixedSize(24, 24);
    bnDel->setObjectName("bnDel");
    connect(bnDel,  &QPushButton::clicked, this, [=] {
        auto listPlan = m_item->listWidget();
        listPlan->removeItemWidget(m_item);
        delete m_item;
        for(int i=0; i<listPlan->count(); i++) ((PlanItemWgt*) listPlan->itemWidget(listPlan->item(i)))->fdIdx->setNum(i+1);
        emit sigItemChanged();
    });
    hBox->addWidget(bnDel);

    hBox = new HBox(vBox);

    QString strs[]{tr("M"), tr("Tu"), tr("W"), tr("Th"), tr("F"), tr("Sa"), tr("Su")};
    for(int i=0; i<7; ++i) {
        btnDays[i] = new QPushButton(strs[i]);
        btnDays[i]->setCheckable(true);
        btnDays[i]->setProperty("ssName", "weeklySelector");
        hBox->addWidget(btnDays[i]);
        hBox->addStretch();
    }

    if(json==0) for(int i=0; i<7; ++i) btnDays[i]->setChecked(true);
    else {
        tStart->setTime(QTime::fromString((*json)["start"].toString(), "hh:mm"));
        tEnd->setTime(QTime::fromString((*json)["end"].toString(), "hh:mm"));
        auto weekly = (*json)["weekly"].toArray();
        for(auto &day : weekly) {
            auto i = day.toInt()-1;
            if(i>=0 && i<7) btnDays[i]->setChecked(true);
        }
    }
    for(int i=0; i<7; ++i) connect(btnDays[i], &QPushButton::toggled, this, &PlanItemWgt::sigItemChanged);

    vBox->addSpacing(6);

    auto line = new QFrame;
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    vBox->addWidget(line);

    item->listWidget()->setItemWidget(item, this);
}
