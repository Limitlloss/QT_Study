#include "emultiwin.h"
#include "main.h"
#include "base/extendedgroupbox.h"
#include "ebase.h"
#include "etext.h"
#include "ephoto.h"
#include "evideo.h"
#include "egif.h"
#include "edclock.h"
#include "eaclock.h"
#include "etimer.h"
#include "eenviron.h"
#include "eweb.h"
#include <QJsonArray>
#include <QBoxLayout>
#include <QMenu>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QFileDialog>

EMultiWin::EMultiWin(PageListItem *pageItem) : mPageItem(pageItem) {
    mType = EBase::Window;
}

EMultiWin::EMultiWin(const JObj &json, PageListItem *pageItem) : mPageItem(pageItem) {
    mType = EBase::Window;
    setBaseAttr(json);
    auto elements = json["elements"].toArray();
    index = json["index"].toInt();
    for(auto &element : elements) {
        QString type = element["elementType"].toString();
        EBase *inner = nullptr;
        if(type=="Text") inner = new EText(element.toObj(), this);
        else if(type=="Image"||type=="Photo") inner = EPhoto::create(element.toObj(), pageItem, this);
        else if(type=="Gif") inner = EGif::create(element.toObj(), pageItem, this);
        else if(type=="Video"||type=="Movie") inner = EVideo::create(element.toObj(), pageItem, this);
        else if(type=="DClock") inner = new EDClock(element.toObj(), this);
        else if(type=="AClock") inner = new EAClock(element.toObj(), this);
        else if(type=="Temp") inner = new EEnviron(element.toObj(), this);
        else if(type=="Web") inner = new EWeb(element.toObj(), this);
        else if(type=="Timer") inner = new ETimer(element.toObj(), this);
        if(inner==0) continue;
        inner->setPos(0, 0);
        inner->setFlag(QGraphicsItem::ItemStacksBehindParent);
        if(index != inners.size()) inner->freeFiles();
        inners.emplace_back(inner);
    }
    if(inners.empty()) return;
    if(index < 0 || index >= (int)inners.size()) index = (int)inners.size()-1;
    setCur(inners[index]);
}
EMultiWin::~EMultiWin() {
    foreach(auto inner, inners) delete inner;
}

void EMultiWin::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    if(inners.empty()) {
        QTextOption opt(Qt::AlignCenter);
        painter->save();
        painter->fillRect(rect(),QColor(0, 0, 0));
        painter->setFont(qfont("Arial", 12));
        painter->setPen(QColor(100, 100, 100));
        painter->drawText(rect(), tr("Please add media on the right"), opt);
        painter->restore();
    }
    EBase::paint(painter, option, widget);
}

bool EMultiWin::save(const QString &pageDir) {
    foreach(auto inner, inners) inner->save(pageDir);
    return true;
}

JObj EMultiWin::attrJson() const{
    JArray eles;
    for(auto inner : inners) eles.append(inner->attrJson());
    JObj oRoot;
    addBaseAttr(oRoot);
    oRoot["elementType"] = "Window";
    oRoot["index"] = index;
    oRoot["elements"] = eles;
    return oRoot;
}

void EMultiWin::setCur(EBase *cur) {
    cur->setSize(mWidth, mHeight);
    connect(this, &EMultiWin::sizeChanged, cur, [cur] {
        if(cur->mMultiWin!=nullptr) cur->setSize(cur->mMultiWin->mWidth, cur->mMultiWin->mHeight);
    });
    cur->setParentItem(this);
    cur->loadFiles();
}

class MListWidget : public QListWidget {
public:
    QSize minimumSizeHint() const override {
        return sizeHint();
    };
    QSize sizeHint() const override {
        auto size = QListWidget::sizeHint();
        auto minH = minimumHeight();
        if(minH > 0) size.setHeight(minH+0xfff);
        return size;
    };
};

QWidget* EMultiWin::attrWgt() {
    auto wgtAttr = new QWidget();
    auto vBox = new QVBoxLayout(wgtAttr);
    vBox->setContentsMargins(3, 0, 3, 0);
    vBox->setSpacing(3);

    addBaseAttrWgt(vBox);

    auto groupBox = new ExtendedGroupBox(tr("Media List"));
    groupBox->setFlat(true);
    groupBox->setCheckable(true);
    auto vBoxGroup = new QVBoxLayout(groupBox);
    vBoxGroup->setContentsMargins(0,0,0,0);
    vBoxGroup->setSpacing(2);

    auto hBox = new QHBoxLayout();

    auto btnAdd = new QPushButton();
    btnAdd->setIcon(QIcon(":/res/program/Add.png"));
    btnAdd->setProperty("style","multiTool");

    auto menu = new QMenu();
    menu->addAction(QIcon(":/res/program/Text.png"), tr("Text"))->setData(EBase::Text);
    menu->addAction(QIcon(":/res/program/Photo.png"), tr("Photo"))->setData(EBase::Image);
    menu->addAction(QIcon(":/res/program/Movie.png"), tr("Video"))->setData(EBase::Video);
    menu->addAction(QIcon(":/res/program/Gif.png"), tr("Gif"))->setData(EBase::Gif);
    menu->addAction(QIcon(":/res/program/DClock.png"), tr("DClock"))->setData(EBase::DClock);
    menu->addAction(QIcon(":/res/program/AClock.png"), tr("AClock"))->setData(EBase::AClock);
    auto listWgt = new MListWidget();
    connect(menu, &QMenu::triggered, this, [=](QAction *act) {
        int type = act->data().toInt();
        int order = listWgt->count();
        EBase *ele = 0;
        QListWidgetItem *item = 0;
        if(type==EBase::Image) {
            auto files = QFileDialog::getOpenFileNames(wgtAttr, translate("","Select File"), gFileHome, EPhoto::filters());
            for(int i=0; i<files.count(); i++) {
                auto ePhoto = EPhoto::create(files[i], mPageItem, this);
                if(ePhoto==0) continue;
                ePhoto->setSize(mWidth, mHeight);
                ePhoto->setZValue(order++);
                ePhoto->setFlag(QGraphicsItem::ItemStacksBehindParent);
                inners.emplace_back(ePhoto);
                auto item = new QListWidgetItem(QIcon(":/res/program/Photo.png"), tr("Photo")+" "+ePhoto->mName);
                item->setData(Qt::UserRole, QVariant::fromValue((void*) ePhoto));
                listWgt->addItem(item);
                if(i==files.count()-1) {
                    listWgt->setCurrentItem(item);
                    gFileHome = ePhoto->mDir;
                }
            }
        } else if(type==EBase::Gif) {
            auto files = QFileDialog::getOpenFileNames(wgtAttr, translate("","Select File"), gFileHome, EGif::filters());
            for(int i=0; i<files.count(); i++) {
                auto eGif = EGif::create(files[i], mPageItem, this);
                if(eGif==0) continue;
                eGif->setSize(mWidth, mHeight);
                eGif->setZValue(order++);
                eGif->setFlag(QGraphicsItem::ItemStacksBehindParent);
                inners.emplace_back(eGif);
                auto item = new QListWidgetItem(QIcon(":/res/program/Gif.png"), tr("Gif")+" "+eGif->mName);
                item->setData(Qt::UserRole, QVariant::fromValue((void*) eGif));
                listWgt->addItem(item);
                if(i == files.count()-1) {
                    listWgt->setCurrentItem(item);
                    gFileHome = eGif->mDir;
                }
            }
        } else if(type==EBase::Video) {
            auto file = QFileDialog::getOpenFileName(wgtAttr, translate("","Select File"), gFileHome, EVideo::filters());
            if(! file.isEmpty()) {
                auto eVideo = EVideo::create(file, mPageItem, this);
                if(eVideo==0) return;
                item = new QListWidgetItem(QIcon(":/res/program/Movie.png"), tr("Video")+" "+eVideo->mRawName);
                gFileHome = eVideo->mRawDir;
                ele = eVideo;
            }
        } else if(type==EBase::Text) {
            ele = new EText(this);
            item = new QListWidgetItem(QIcon(":/res/program/Text.png"), tr("Text"));
        } else if(type==EBase::DClock) {
            ele = new EDClock(this);
            item = new QListWidgetItem(QIcon(":/res/program/DClock.png"), tr("DClock"));
        } else if(type==EBase::AClock) {
            ele = new EAClock(this);
            item = new QListWidgetItem(QIcon(":/res/program/AClock.png"), tr("AClock"));
        } else if(type==EBase::Environ) {
            ele = new EEnviron(this);
            item = new QListWidgetItem(QIcon(":/res/program/Temp.png"), tr("Environment"));
        } else if(type==EBase::Timer) {
            ele = new ETimer(this);
            item = new QListWidgetItem(QIcon(":/res/program/Timer.png"), tr("Timer"));
        }
        if(ele) {
            ele->setSize(mWidth, mHeight);
            ele->setZValue(order);
            ele->setFlag(QGraphicsItem::ItemStacksBehindParent);
            inners.emplace_back(ele);
            item->setData(Qt::UserRole, QVariant::fromValue((void*)ele));
            listWgt->addItem(item);
            listWgt->setCurrentItem(item);
        }
    });
    btnAdd->setMenu(menu);
    hBox->addWidget(btnAdd);

    auto btnDel = new QPushButton();
    btnDel->setIcon(QIcon(":/res/program/Delete.png"));
    btnDel->setProperty("style","multiTool");
    connect(btnDel, &QPushButton::clicked, this, [this, listWgt] {
        auto row = listWgt->currentRow();
        if(row < 0) return;
        auto item = listWgt->takeItem(row);
        if(listWgt->count() > 0) listWgt->setCurrentRow(0);
        auto ele = static_cast<EBase*>(item->data(Qt::UserRole).value<void*>());
        delete item;
        for(auto i=inners.begin(); i < inners.end(); ++i) if(*i==ele) inners.erase(i);
        delete ele;
        int n = listWgt->count();
        for(int i=0; i<n; i++) static_cast<EBase*>(listWgt->item(i)->data(Qt::UserRole).value<void*>())->setZValue(i);
    });
    hBox->addWidget(btnDel);

    auto btnClean = new QPushButton();
    btnClean->setIcon(QIcon(":/res/program/Clean.png"));
    btnClean->setProperty("style","multiTool");
    connect(btnClean, &QPushButton::clicked, this, [this, listWgt] {
        listWgt->clear();
        foreach(auto inner, inners) delete inner;
        inners.clear();
    });
    hBox->addWidget(btnClean);

    auto btnGoUp = new QPushButton();
    btnGoUp->setIcon(QIcon(":/res/program/GoUp.png"));
    btnGoUp->setProperty("style","multiTool");
    connect(btnGoUp, &QPushButton::clicked, this, [this, listWgt] {
        int row = listWgt->currentRow();
        if(row < 1) return;
        listWgt->insertItem(row-1, listWgt->takeItem(row));
        listWgt->setCurrentRow(row-1);
        int n = listWgt->count();
        for(int i=0; i<n; i++) static_cast<EBase*>(listWgt->item(i)->data(Qt::UserRole).value<void*>())->setZValue(i);
        auto aaa = inners[row];
        inners[row] = inners[row-1];
        inners[row-1] = aaa;
    });
    hBox->addWidget(btnGoUp);

    auto btnGoDown = new QPushButton();
    btnGoDown->setIcon(QIcon(":/res/program/GoDown.png"));
    btnGoDown->setProperty("style","multiTool");
    connect(btnGoDown, &QPushButton::clicked, this, [this, listWgt] {
        int row = listWgt->currentRow();
        if(row < 0 || row > listWgt->count() - 2) return;
        listWgt->insertItem(row+1, listWgt->takeItem(row));
        listWgt->setCurrentRow(row+1);
        int n = listWgt->count();
        for(int i=0; i<n; i++) static_cast<EBase*>(listWgt->item(i)->data(Qt::UserRole).value<void*>())->setZValue(i);
        auto aaa = inners[row];
        inners[row] = inners[row+1];
        inners[row+1] = aaa;
    });
    hBox->addWidget(btnGoDown);

    vBoxGroup->addLayout(hBox);

    listWgt->setMinimumHeight(120);
    listWgt->setIconSize(QSize(20, 20));
    foreach(auto inner, inners) {
        QListWidgetItem *item = 0;
        int type = inner->type();
        if(type==EBase::Text) item = new QListWidgetItem(QIcon(":/res/program/Text.png"), tr("Text"));
        else if(type==EBase::Image) item = new QListWidgetItem(QIcon(":/res/program/Photo.png"), tr("Photo")+" "+static_cast<EPhoto*>(inner)->mName);
        else if(type==EBase::Video) item = new QListWidgetItem(QIcon(":/res/program/Movie.png"), tr("Video")+" "+static_cast<EVideo*>(inner)->mRawName);
        else if(type==EBase::Gif) item = new QListWidgetItem(QIcon(":/res/program/Gif.png"), tr("Gif")+" "+static_cast<EGif*>(inner)->mName);
        else if(type==EBase::DClock) item = new QListWidgetItem(QIcon(":/res/program/DClock.png"), tr("DClock"));
        else if(type==EBase::AClock) item = new QListWidgetItem(QIcon(":/res/program/AClock.png"), tr("AClock"));
        else if(type==EBase::Environ) item = new QListWidgetItem(QIcon(":/res/program/Temp.png"), tr("Environment"));
        else if(type==EBase::Timer) item = new QListWidgetItem(QIcon(":/res/program/Timer.png"), tr("Timer"));
        if(item != nullptr) {
            item->setData(Qt::UserRole, QVariant::fromValue((void*) inner));
            listWgt->addItem(item);
        }
    }
    connect(listWgt, &QListWidget::currentItemChanged, this, [=](QListWidgetItem *current, QListWidgetItem *previous) {
        if(previous) {
            auto eLast = static_cast<EBase*>(previous->data(Qt::UserRole).value<void*>());
            if(eLast) {
                if(scene()) scene()->removeItem(eLast);
                disconnect(this, &EMultiWin::sizeChanged, eLast, 0);
                eLast->freeFiles();
            }
            auto itemLast = vBox->itemAt(vBox->count()-1);
            if(itemLast && itemLast->widget() != groupBox) {
                vBox->takeAt(vBox->count()-1);
                if(itemLast->widget()) delete itemLast->widget();
                delete itemLast;
            }
        }
        EBase *eCur = 0;
        if(current) {
            eCur = (EBase*) current->data(Qt::UserRole).value<void*>();
            auto attr = eCur->attrWgt();
            if(attr) vBox->addWidget(attr);
        }
        index = listWgt->currentRow();
        if(eCur) setCur(eCur);
    });
    vBoxGroup->addWidget(listWgt);

    vBox->addWidget(groupBox);

    if(index > -1) listWgt->setCurrentRow(index);
    return wgtAttr;
}
