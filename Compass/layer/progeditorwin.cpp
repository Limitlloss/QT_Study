#include "progeditorwin.h"
#include "ebase.h"
#include "pagelistitem.h"
#include "player/playwin.h"
#include "tools.h"
#include "main.h"
#include "gutil/qwaitingdlg.h"
#include "program/ebase.h"
#include "program/etext.h"
#include "program/ephoto.h"
#include "program/evideo.h"
#include "program/egif.h"
#include "program/edclock.h"
#include "program/eaclock.h"
#include "program/eenviron.h"
#include "program/eweb.h"
#include "program/etimer.h"
#include "program/etimer2.h"
#include "program/emultiwin.h"
#include "program/gentmpthread.h"
#include "program/sendprogramdialog.h"
#include <QCloseEvent>
#include <QGraphicsDropShadowEffect>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QToolBar>
#include <QUdpSocket>
#include <QMovie>

ProgItem *gProgItem = 0;

ProgEditorWin::ProgEditorWin(ProgItem *progItem, QWidget *parent) : QWidget(parent), mProgItem(progItem) {
    gProgItem = progItem;
    setWindowFlag(Qt::Window);
    setWindowTitle(progItem->mName);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_AlwaysShowToolTips);
    auto parentWin = parentWidget()->window();
    if(! parentWin->isMaximized()) resize(parentWin->size());
    else resize(1280, 720);
#ifdef Q_OS_WIN
    setWindowModality(Qt::WindowModal);
#else
    parentWin->hide();
#endif

    auto vBox = new QVBoxLayout(this);
    vBox->setContentsMargins(0, 0, 0, 0);
    vBox->setSpacing(0);

    auto toolBar = new QToolBar();
    toolBar->setFloatable(false);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->setIconSize(QSize(46, 40));

    auto action = new QAction(QIcon(":/res/program/Save.png"), tr("Save"));
    connect(action, &QAction::triggered, this, &ProgEditorWin::onSave);
    toolBar->addAction(action);
//    action = new QAction(QIcon(":/res/program/SaveAs.png"), tr("Save as"));
//    connect(action, &QAction::triggered, this, [this] {
//        auto saveThread = QThread::create([this](){
//            save();
//        });
//        connect(saveThread, SIGNAL(finished()), saveThread, SLOT(deleteLater()));
//        connect(saveThread, &QThread::finished, this, [this] {
//            mProgItem->onSetProgram();
//        });
//        saveThread->start();
//        bool isOK;
//        QString progName = QInputDialog::getText(this, tr("Save as"), tr("Save as"), QLineEdit::Normal, QString(), &isOK, Qt::Dialog | Qt::WindowCloseButtonHint);
//        if(! isOK || progName.isEmpty()) return;
//        mProgItem->mProgDir = mProgItem->mProgsDir + "/" + progName;
//        LoEmptyDialog *dlgTip = new LoEmptyDialog(this);
//        saveThread = QThread::create([this](){
//            save();
//        });
//        dlgTip->lock(tr("Saving..."),translate("","Success"),tr("Save failed"));
//        connect(saveThread, SIGNAL(finished()), dlgTip, SLOT(unlock()));//显示正在保存提示对话框
//        connect(saveThread, SIGNAL(finished()), saveThread, SLOT(deleteLater()));
//        connect(saveThread, &QThread::finished, this, [this] {
//            mProgItem->onSetProgram();
//        });
//        saveThread->start();
//        dlgTip->exec();
//        close();
//        setWindowTitle(progName);
//        mProgItem->mProgPanel->onCreateNewProgramOnOpenEditProgramWidget(progName, QSize(mProgItem->mWidth, mProgItem->mHeight), mProgItem->mRemark, mProgItem->mSplitWidths, mProgItem->mMaxWidth);
//    void ProgPanel::onCreateNewProgramOnOpenEditProgramWidget(QString name, QSize res, QString remarks, QList<int> &partLengths, int max)
//    {
//        if(checkIfNameRepeated(name)) return;
//        auto item = new ProgItem(mProgsDir, name, res.width(), res.height(), remarks, partLengths, max, mProgTree);
//        item->save();//保存pro.json
//        if(mProgTree->fdCheckAll->checkState()==Qt::Checked) {
//            mProgTree->fdCheckAll->blockSignals(true);
//            mProgTree->fdCheckAll->setCheckState(Qt::PartiallyChecked);
//            mProgTree->fdCheckAll->blockSignals(false);
//        }
//        auto editor = new ProgEditorWin(item, this);
//        editor->show();
//    }
//    });
//    toolBar->addAction(action);
    action = new QAction(QIcon(":/res/program/Setting.png"), tr("Setting"));
    connect(action, &QAction::triggered, this, [this] {
        QString widthsStr;
        for(auto &width : mProgItem->partLens) {
            if(! widthsStr.isEmpty()) widthsStr.append(" ");
            widthsStr.append(QString::number(width));
        }
        ProgCreateDlg dlg(mProgItem->mName, mProgItem->mWidth, mProgItem->mHeight, mProgItem->mRemark, widthsStr, mProgItem->isVer, this);
        dlg.edIsInsert->setChecked(mProgItem->isInsert);
        if(dlg.exec() != QDialog::Accepted) return;
        mProgItem->mWidth = gProgWidth = dlg.fdWidth->value();
        mProgItem->mHeight = gProgHeight = dlg.fdHeight->value();
        mProgItem->mRemark = dlg.fdRemark->toPlainText();
        mProgItem->isInsert = dlg.edIsInsert->isChecked();
        mProgItem->partLens.clear();
        mProgItem->maxLen = 0;
        mProgItem->isVer = dlg.fdVer->isChecked();
        auto len = mProgItem->isVer ? mProgItem->mHeight : mProgItem->mWidth;
        if(dlg.fdIsUltraLong->isChecked()) {
            auto partLengths = dlg.fdSplitWidths->text().split(" ", Qt::SkipEmptyParts);
            int ttl = 0;
            for(auto &partLength : partLengths) {
                int val = partLength.toInt();
                if(val==0) continue;
                if(mProgItem->maxLen < val) mProgItem->maxLen = val;
                ttl += val;
                mProgItem->partLens.emplace_back(val);
                if(ttl>=len) break;
            }
            if(mProgItem->maxLen) {
                while(ttl < len) {
                    mProgItem->partLens.push_back(mProgItem->maxLen);
                    ttl += mProgItem->maxLen;
                }
                if(ttl > len) mProgItem->partLens.back() -= ttl - len;
            }
        }
        mProgItem->onSetProgram();
        for(int i=0; i<listPage->count(); i++) {
            auto page = (PageListItem*) listPage->item(i);
            page->mScene->setSceneRect(0, 0, mProgItem->mWidth, mProgItem->mHeight);
            page->mGraView->resetTransform();
            qreal scale = qMin(page->mGraView->width() / page->mScene->width(), page->mGraView->height() / page->mScene->height());
            page->mGraView->scale(scale, scale);

            auto items = page->mScene->items();
            for(auto item : items) {
                auto element = (EBase*) item;
                if(element->mMultiWin == 0) element->fitProgSize();
            }
            page->mScene->update();
        }
        onSave();
    });
    toolBar->addAction(action);
    toolBar->addSeparator();

    if(progItem->partLens.empty()) {
        action = new QAction(QIcon(":/res/program/Window.png"), tr("MuliContentWindow"));
        action->setToolTip(tr("In this window, a plurality of different program materials can be added and played according to the order of joining the list;"));
        action->setData(EBase::Window);
        toolBar->addAction(action);
        toolBar->addSeparator();
    }
    action = new QAction(QIcon(":/res/program/Text.png"), tr("Text"));
    action->setData(EBase::Text);
    toolBar->addAction(action);
    action = new QAction(QIcon(":/res/program/Photo.png"), tr("Photo"));
    action->setData(EBase::Image);
    toolBar->addAction(action);
    action = new QAction(QIcon(":/res/program/Movie.png"), tr("Video"));
    action->setData(EBase::Video);
    toolBar->addAction(action);
    action = new QAction(QIcon(":/res/program/Gif.png"), tr("Gif"));
    action->setData(EBase::Gif);
    toolBar->addAction(action);
    action = new QAction(QIcon(":/res/program/DClock.png"), tr("Clock"));
    action->setData(EBase::DClock);
    toolBar->addAction(action);
    action = new QAction(QIcon(":/res/program/AClock.png"), tr("Analog Clock"));
    action->setData(EBase::AClock);
    toolBar->addAction(action);
    action = new QAction(QIcon(":/res/program/Temp.png"), tr("Environment"));
    action->setData(EBase::Environ);
    toolBar->addAction(action);
    action = new QAction(QIcon(":/res/program/Web.png"), tr("Web"));
    action->setData(EBase::Web);
    toolBar->addAction(action);
    action = new QAction(QIcon(":/res/program/Timer.png"), tr("Timer"));
    action->setData(EBase::Timer);
    toolBar->addAction(action);
    action = new QAction(QIcon(":/res/program/Timer.png"), tr("Timer")+"2");
    action->setData(EBase::Timer2);
    toolBar->addAction(action);
#ifndef leyide
    action = new QAction(QIcon(":/res/program/demo-video.png"), tr("Demos"));
    connect(action, &QAction::triggered, this, [this] {
        auto file = QFileDialog::getOpenFileName(this, tr("Open Demo"), QApplication::applicationDirPath()+"/Demos");
        if(file.isEmpty()) return;
        auto scene = mPageEditor->graphicsView->scene();
        if(scene==0) return;
        if(file.endsWith("png", Qt::CaseInsensitive)) {
            auto ePhoto = EPhoto::create(file, mPageItem);
            if(ePhoto==0) return;
            auto rect = Tools::centerRect(ePhoto->img.width(), ePhoto->img.height(), mProgItem->mWidth, mProgItem->mHeight);
            ePhoto->setPos(rect.topLeft());
            ePhoto->setSize(rect.width(), rect.height());
            ePhoto->setZValue(mPageEditor->sortedEles().count());
            scene->addItem(ePhoto);
            auto sels = scene->selectedItems();
            if(sels.count() == 1) sels.at(0)->setSelected(false);
            ePhoto->setSelected(true);
        } else {
            auto eVideo = EVideo::create(file, mPageItem);
            if(eVideo==0) return;
            auto rect = Tools::centerRect(eVideo->mCoverImg.width(), eVideo->mCoverImg.height(), mProgItem->mWidth, mProgItem->mHeight);
            eVideo->setPos(rect.topLeft());
            eVideo->setSize(rect.width(), rect.height());
            eVideo->setZValue(mPageEditor->sortedEles().count());
            scene->addItem(eVideo);
            auto sels = scene->selectedItems();
            if(sels.count() == 1) sels.at(0)->setSelected(false);
            eVideo->setSelected(true);
        }
    });
    toolBar->addAction(action);
#endif
    toolBar->addSeparator();

    action = new QAction(QIcon(":/res/program/preview.png"), tr("Play")+"/"+tr("Stop"));
    connect(action, &QAction::triggered, this, [this] {
        if(PlayWin::self) PlayWin::self->close();
        else {
             if(isProgChanged()) onSave();
             auto waitingDlg = new WaitingDlg(this, tr("Generate preview data")+" ...");
             auto gen = new GenTmpThread(mProgItem, mProgItem->mName, "", "");
             connect(gen, &GenTmpThread::onErr, this, [=](QString err) {
                 QMessageBox::warning(this, "GenTmpThread Error", err);
             });
             connect(gen, &QThread::finished, waitingDlg, &WaitingDlg::close);
             gen->start();
             waitingDlg->exec();
             QFile file(mProgItem->mProgDir+"_tmp/program");
             if(! file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
             auto value = file.readAll();
             file.close();
             QString jsErr;
             auto prog = JFrom(value, &jsErr);
             if(! jsErr.isEmpty()) return;
             int www = mProgItem->mWidth, hhh = mProgItem->mHeight;
             if(mProgItem->maxLen) {
                 if(mProgItem->isVer) {
                     hhh = mProgItem->maxLen;
                     www *= mProgItem->partLens.size();
                 } else {
                     www = mProgItem->maxLen;
                     hhh *= mProgItem->partLens.size();
                 }
             }
             PlayWin::self = PlayWin::newIns(www, hhh, mProgItem->mProgDir+"_tmp", prog);
         }
     });
    toolBar->addAction(action);
    action = new QAction(QIcon(":/res/program/Send.png"), tr("Publish"));
    connect(action, &QAction::triggered, this, [this]{
        onSave();
        auto waitingDlg = new WaitingDlg(this, tr("Convertering")+" ...");
        auto gen = new GenTmpThread(mProgItem, mProgItem->mName, "", "");
        connect(gen, &GenTmpThread::onErr, this, [=](QString err) {
            QMessageBox::warning(this, "GenTmpThread Error", err);
        });
        connect(gen, &QThread::finished, waitingDlg, &WaitingDlg::close);
        gen->start();
        waitingDlg->exec();
        SendProgramDialog dlg(mProgItem->mName, this);
        dlg.exec();
    });
    toolBar->addAction(action);

    connect(toolBar, &QToolBar::actionTriggered, this, [this](QAction *act) {
        auto data = act->data();
        if(data.type()!=QVariant::Int) return;
        auto scene = mPageEditor->graphicsView->scene();
        if(scene==0) return;
        int order = mPageEditor->sortedEles().count();
        EBase *element = 0;
        int iNewWidth = mProgItem->mWidth;
        int iNewHeight = mProgItem->mHeight;
        if(iNewWidth>128) iNewWidth = iNewWidth * 2 / 3;
        if(iNewHeight>128) iNewHeight = iNewHeight * 2 / 3;
        if(mNewEleX+iNewWidth>mProgItem->mWidth) mNewEleX=0;
        if(mNewEleY+iNewHeight>mProgItem->mHeight) mNewEleY=0;
        auto type = data.toInt();
        if(type==EBase::Image) {
            auto files = QFileDialog::getOpenFileNames(this, translate("","Select File"), gFileHome, EPhoto::filters());
            for(int i=0; i<files.count(); i++) {
                auto ePhoto = EPhoto::create(files[i], mPageItem);
                if(ePhoto==0) continue;
                auto rect = Tools::centerRect(ePhoto->img.width(), ePhoto->img.height(), mProgItem->mWidth, mProgItem->mHeight);
                ePhoto->setPos(rect.topLeft());
                ePhoto->setSize(rect.width(), rect.height());
                ePhoto->setZValue(order++);
                scene->addItem(ePhoto);
                if(i==files.count()-1) {
                    auto sels = scene->selectedItems();
                    if(sels.count() == 1) sels.at(0)->setSelected(false);
                    ePhoto->setSelected(true);
                    gFileHome = ePhoto->mDir;
                }
            }
        } else if(type==EBase::Gif) {
            auto files = QFileDialog::getOpenFileNames(this, translate("","Select File"), gFileHome, EGif::filters());
            for(int i=0; i<files.count(); i++) {
                auto eGif = EGif::create(files[i], mPageItem);
                if(eGif==0) continue;
                auto img = eGif->mMovie->currentPixmap();
                auto rect = Tools::centerRect(img.width(), img.height(), mProgItem->mWidth, mProgItem->mHeight);
                eGif->setPos(rect.topLeft());
                eGif->setSize(rect.width(), rect.height());
                eGif->setZValue(order++);
                scene->addItem(eGif);
                if(i == files.count()-1) {
                    auto sels = scene->selectedItems();
                    if(sels.count() == 1) sels.at(0)->setSelected(false);
                    eGif->setSelected(true);
                    gFileHome = eGif->mDir;
                }
            }
        } else if(type==EBase::Video) {
            auto file = QFileDialog::getOpenFileName(this, translate("","Select File"), gFileHome, EVideo::filters());
            if(file.isEmpty()) return;
            auto eVideo = EVideo::create(file, mPageItem);
            if(eVideo==nullptr) return;
            auto rect = Tools::centerRect(eVideo->mCoverImg.width(), eVideo->mCoverImg.height(), mProgItem->mWidth, mProgItem->mHeight);
            eVideo->setPos(rect.topLeft());
            eVideo->setSize(rect.width(), rect.height());
            gFileHome = eVideo->mRawDir;
            element = eVideo;
        } else if(type==EBase::Text) {
            if(iNewHeight > 80 && (mProgItem->mWidth >= mProgItem->mHeight)) iNewHeight = 80;
            element = new EText;
        } else if(type==EBase::DClock) {
            if(iNewHeight>80 && (mProgItem->mWidth>=mProgItem->mHeight)) iNewHeight=80;
            element = new EDClock;
        } else if(type==EBase::AClock) {
            if(iNewWidth > 120) iNewWidth = 120;
            if(iNewHeight > 120) iNewHeight = 120;
            element = new EAClock;
        } else if(type==EBase::Environ) element = new EEnviron;
        else if(type==EBase::Web) {
            element = new EWeb;
            element->setSize(mProgItem->mWidth, mProgItem->mHeight);
        } else if(type==EBase::Timer) element = new ETimer;
        else if(type==EBase::Timer2) element = new ETimer2;
        else if(type==EBase::Window) element = new EMultiWin(mPageItem);
        if(element) {
            if(element->mWidth==0) {
                element->setPos(mNewEleX, mNewEleY);
                element->setSize(iNewWidth, iNewHeight);
            }
            element->setZValue(order);
            scene->addItem(element);
            mNewEleX += 8;
            mNewEleY += 8;
            auto sels = scene->selectedItems();
            if(sels.count() == 1) sels.at(0)->setSelected(false);
            element->setSelected(true);
        }
    });
    vBox->addWidget(toolBar);

    auto line = new QFrame();
    line->setFrameStyle(QFrame::Sunken);
    line->setFrameShape(QFrame::HLine);
    vBox->addWidget(line);

    //主区域横向布局（幻灯片页列表，编辑窗，属性窗）
    auto hBox = new QHBoxLayout();
    hBox->setContentsMargins(0, 0, 0, 0);
    hBox->setSpacing(0);

    auto vBoxPage = new QVBoxLayout();
    vBoxPage->setContentsMargins(0, 0, 0, 0);
    vBoxPage->setSpacing(0);
    auto lbName = new QLabel(tr("program"));
    lbName->setStyleSheet("QLabel{background-color: #bbb;}");
    lbName->setAlignment(Qt::AlignCenter);
    vBoxPage->addWidget(lbName);

    toolBar = new QToolBar();
    toolBar->setIconSize(QSize(22,16));
    toolBar->setStyleSheet("QToolBar{spacing: 8px;}");

    auto actAdd = new QAction(QIcon(":/res/program/Add.png"), tr("Add page"));
    connect(actAdd, &QAction::triggered, this, &ProgEditorWin::onAddPage);
    toolBar->addAction(actAdd);

    auto actCopy = new QAction(QIcon(":/res/program/Copy.png"), tr("Copy page"));
    connect(actCopy, &QAction::triggered, this, [this] {
        if(listPage->count() > 0) {
            auto cur = (PageListItem*) listPage->currentItem();
            cur->updateJson();
            JObj attr;
            for(auto &pair : cur->mAttr) attr.insert(pair.first, pair.second);
            auto item = new PageListItem(attr, cur->mPageDir);
            item->mAttr["name"] = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
            listPage->addItem(item);
            listPage->setItemWidget(item, item->itemWgt());
            listPage->setCurrentItem(item);
            if(listPage->count()==5) for(int i=0; i<4; i++) ((PageListItem*)listPage->item(i))->fdIdx->setNum(i+1);
        }
    });
    toolBar->addAction(actCopy);

    auto actDel = new QAction(QIcon(":/res/program/Delete.png"), tr("Delete page"));
    connect(actDel, &QAction::triggered, this, [this] {
        if(listPage->count() == 1) mPageEditor->onClean();
        else if(listPage->count() > 1) {
            auto item = (PageListItem*) listPage->currentItem();
            auto res = QMessageBox::information(this, translate("","Tip"), tr("Are you sure you want to delete this program page?"), QMessageBox::Ok, QMessageBox::Cancel);
            if(res == QMessageBox::Ok) {
                delete item;
                if(listPage->count() > 0) listPage->setCurrentRow(0);
                for(int i=0; i<listPage->count(); i++) ((PageListItem*)listPage->item(i))->fdIdx->setNum(i+1);
            }
        }
    });
    toolBar->addAction(actDel);

    auto actMoveUp = new QAction(QIcon(":/res/program/GoUp.png"), tr("Move up"));
    connect(actMoveUp, &QAction::triggered, this, [this] {
        if(listPage->count() > 1) {
            int index = listPage->currentRow();
            if(index > 0) {
                auto item = static_cast<PageListItem*>(listPage->takeItem(index));
                listPage->insertItem(index-1, item);
                listPage->setItemWidget(item, item->itemWgt());
                listPage->setCurrentRow(index-1);
            }
            for(int i=0; i<listPage->count(); i++) ((PageListItem*)listPage->item(i))->fdIdx->setNum(i+1);
        }
    });
    toolBar->addAction(actMoveUp);

    auto actMoveDown = new QAction(QIcon(":/res/program/GoDown.png"), tr("Move down"));
    connect(actMoveDown, &QAction::triggered, this, [this] {
        if(listPage->count() > 1) {
            int index = listPage->currentRow();
            if(index < listPage->count()-1) {
                auto item = static_cast<PageListItem*>(listPage->takeItem(index));
                listPage->insertItem(index+1, item);
                listPage->setItemWidget(item, item->itemWgt());
                listPage->setCurrentRow(index+1);
            }
            for(int i=0; i<listPage->count(); i++) ((PageListItem*)listPage->item(i))->fdIdx->setNum(i+1);
        }
    });
    toolBar->addAction(actMoveDown);

    vBoxPage->addWidget(toolBar);

    listPage = new QListWidget;
    listPage->setMaximumWidth(190);
    connect(listPage, &QListWidget::currentItemChanged, this, [=](QListWidgetItem *current, QListWidgetItem *previous) {
        auto curItem = static_cast<PageListItem*>(current);
        auto preItem = static_cast<PageListItem*>(previous);
        if(preItem) preItem->mScene->clearSelection();
        mPageItem = curItem;
        auto scene = mPageEditor->graphicsView->scene();
        if(scene) disconnect(scene, 0, 0, 0);
        if(mPageEditor->curScale != 100) {
            mPageEditor->curScale = 100;
            mPageEditor->fdScale->setText("100");
            mPageEditor->graphicsView->resetTransform();
        }
        if(curItem == 0) mPageEditor->graphicsView->setScene(0);
        else {
            mPageEditor->graphicsView->setScene(scene = curItem->mScene);
            connect(scene, &QGraphicsScene::selectionChanged, this, [=] {
                auto sels = scene->selectedItems();
                auto scroll = static_cast<QScrollArea*>(mTabsAttr->widget(0));
                if(sels.size() != 1) {
                    mTabsAttr->setCurrentIndex(1);
                    auto wgt = scroll->takeWidget();
                    if(wgt) delete wgt;
                } else {
                    mTabsAttr->setCurrentIndex(0);
                    scroll->setWidget(static_cast<EBase*>(sels[0])->attrWgt());
                    scroll->updateGeometry();
                }
            });
        }
        auto scroll = static_cast<QScrollArea*>(mTabsAttr->widget(1));
        scroll->takeWidget();
        if(curItem) {
            scroll->setWidget(curItem->attrWgt());
            scroll->updateGeometry();
        }
        mTabsAttr->setCurrentIndex(1);
    });
    vBoxPage->addWidget(listPage);
    hBox->addLayout(vBoxPage);

    hBox->addWidget(mPageEditor = new PageEditor, 1);

    mTabsAttr = new QTabWidget();
    mTabsAttr->setMinimumWidth(360);
    mTabsAttr->setStyleSheet("QTabWidget::pane{border:none;}");

    QPalette backTrans;
    backTrans.setColor(QPalette::Window, Qt::transparent);

    auto scroll = new QScrollArea();
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setWidgetResizable(true);
    scroll->setPalette(backTrans);
    mTabsAttr->addTab(scroll, tr("Widget Properties"));

    scroll = new QScrollArea();
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setWidgetResizable(true);
    scroll->setPalette(backTrans);
    mTabsAttr->addTab(scroll, tr("Program Properties"));
    mTabsAttr->setCurrentIndex(0);

    hBox->addWidget(mTabsAttr);

    vBox->addLayout(hBox);

    QDir progQDir(mProgItem->mProgDir);
    if(progQDir.exists()) {
        auto pageNames = progQDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
        for(QString &pageName : pageNames) {
            if(pageName.endsWith(PAGEDEL_SUFFIX)) {
                QDir(mProgItem->mProgDir + "/" + pageName).removeRecursively();
                continue;
            }
            QFile pageFile(mProgItem->mProgDir + "/" + pageName + "/page.json");
            if(pageFile.exists()) {
                pageFile.open(QIODevice::ReadOnly);
                mPageJsons.emplace_back(JFrom(pageFile.readAll()).toObj());
                pageFile.close();
            } else QDir(mProgItem->mProgDir + "/" + pageName).removeRecursively();
        }
        std::sort(mPageJsons.begin(), mPageJsons.end(), [](const JObj &a, const JObj &b) {
            return a["order"].toInt() < b["order"].toInt();
        });
    }
    if(mPageJsons.empty()) onAddPage();
    else for(JObj &pageJson : mPageJsons) {
        auto pageDir = mProgItem->mProgDir+"/"+pageJson["name"].toString();
        QDir dir(pageDir);
        if(! dir.exists() && ! dir.mkdir(pageDir)) continue;
        JObj attr;
        for(auto &pair : pageJson) attr.insert(pair.first, pair.second);
        auto item = new PageListItem(attr, pageDir);
        listPage->addItem(item);
        listPage->setItemWidget(item, item->itemWgt());
        auto cnt = listPage->count();
        if(cnt==5) for(int i=0; i<4; i++) ((PageListItem*)listPage->item(i))->fdIdx->setNum(i+1);
    }
    listPage->setCurrentRow(0);
}
bool ProgEditorWin::isProgChanged() {
    if(mPageJsons.size() != listPage->count()) return true;
    for(int i=0; i<(int)mPageJsons.size(); i++) {
        auto page = (PageListItem*) listPage->item(i);
        if(page->mAttrWgt==0) continue;
        page->updateJson();
        if(page->mAttr != mPageJsons[i]) return true;
    }
    return false;
}
void ProgEditorWin::closeEvent(QCloseEvent *event) {
    mProgItem->onSetProgram();
    if(isProgChanged()) {
        auto res = QMessageBox::question(this, translate("","Tip"), tr("Do you want to save the modifications?"), QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
        if(res == QMessageBox::Yes) onSave();
        else if(res == QMessageBox::Cancel) event->ignore();
    }
#ifdef Q_OS_MAC
    if(event->isAccepted()) parentWidget()->window()->show();
#endif
}
//停止每个页面元素的播放，将节目目录下的每个页面的文件夹另命名
bool ProgEditorWin::save() {
    auto rtn = true;
    //停止每个幻灯片的元素
    int cnt = listPage->count();
    for(int i=0; i<cnt; i++) {
        auto items = static_cast<PageListItem*>(listPage->item(i))->mScene->items();
        foreach(auto item, items) static_cast<EBase*>(item)->freeFiles();
    }
    QDir progDir(mProgItem->mProgDir);
    if(! progDir.exists() && ! progDir.mkdir(mProgItem->mProgDir)) {
        QMessageBox::critical(this, translate("","Error"), tr("Create Dir failed")+": "+mProgItem->mProgDir);
        return 0;
    }
    auto pageNames = progDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    for(QString &pageName : pageNames) if(! progDir.rename(pageName, pageName + PAGEDEL_SUFFIX)) {
        rtn = 0;
        QMessageBox::critical(this, translate("","Error"), tr("Rename fail when saving")+" "+pageName);
    }
    //保存每个页面的元素和页面属性到page.json文档
    mPageJsons.clear();
    for(int i=0; i<cnt; i++) {
        auto page = (PageListItem*) listPage->item(i);
        page->mAttr["order"] = i;
        if(page->saveFiles()) {
            JObj json;
            for(auto &pair : page->mAttr) json.insert(pair.first, pair.second);
            mPageJsons.emplace_back(json);
        }
    }
    pageNames = progDir.entryList(QStringList("*" PAGEDEL_SUFFIX));
    foreach(QString pageName, pageNames) {
        if(! QDir(mProgItem->mProgDir + "/" + pageName).removeRecursively()) {
            rtn = 0;
            QMessageBox::critical(this, translate("","Error"), tr("Remove Recursively fail when saving")+" "+pageName);
        }
    }
    for(int i=0; i<cnt; i++) {
        auto items = static_cast<PageListItem*>(listPage->item(i))->mScene->items();
        foreach(auto item, items) static_cast<EBase*>(item)->loadFiles();
    }
    return rtn;
}

void ProgEditorWin::onSave() {
    auto waitingDlg = new WaitingDlg(this, tr("Saving..."), translate("","Success"));
    waitingDlg->setWindowFlag(Qt::WindowCloseButtonHint, 0);
    waitingDlg->show();
    if(save()) waitingDlg->success();
    else waitingDlg->close();
    waitingDlg->exec();
    mProgItem->onSetProgram();
}

void ProgEditorWin::onAddPage() {
    QDateTime now = QDateTime::currentDateTime();
    auto name = now.toString("yyyyMMddhhmmsszzz");
    auto pageDir = mProgItem->mProgDir+"/"+name;
    QDir dir(pageDir);
    if(! dir.exists() && ! dir.mkpath(pageDir)) return;
    JObj attr;
    attr["name"] = name;
    attr["repeat"] = 1;
    attr["validDate"] = JObj{
        {"isValid", false},
        {"start", now.date().toString("yyyy-MM-dd")},
        {"end", now.addSecs(2678400).date().toString("yyyy-MM-dd")}
    };
    auto item = new PageListItem(attr, pageDir);
    listPage->addItem(item);
    listPage->setItemWidget(item, item->itemWgt());
    listPage->setCurrentItem(item);
    if(listPage->count()==5) for(int i=0; i<4; i++) ((PageListItem*)listPage->item(i))->fdIdx->setNum(i+1);
}

ProgCreateDlg::ProgCreateDlg(QString name, int width, int height, QString remarks, QString widths, bool isVer, QWidget *parent) : QDialog(parent) {
#ifdef Q_OS_WIN
    setWindowFlag(Qt::WindowContextHelpButtonHint, 0);
#endif
    setWindowTitle(tr("Solution Info"));
    auto vBox = new VBox(this);
    vBox->setContentsMargins(6,0,6,6);
    auto hBox = new HBox(vBox);

    auto label = new QLabel(tr("Solution Name"));
    label->setMinimumWidth(90);
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    hBox->addWidget(label);

    fdName = new QLineEdit;
    if(name.isEmpty()) name = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
    else fdName->setDisabled(true);
    fdName->setText(name);
    hBox->addWidget(fdName);

    hBox = new HBox(vBox);
    hBox->setSpacing(12);

    label = new QLabel(tr("Resolution"));
    label->setMinimumWidth(90);
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    hBox->addWidget(label);
    hBox->addSpacing(6);

    auto label_4 = new QLabel(tr("Width"));
    hBox->addWidget(label_4);

    fdWidth = new QSpinBox;
    fdWidth->setMaximum(99999);
    fdWidth->setValue(width);
    hBox->addWidget(fdWidth);

    auto label_5 = new QLabel(tr("Height"));
    hBox->addWidget(label_5);

    fdHeight = new QSpinBox;
    fdHeight->setMaximum(99999);
    fdHeight->setValue(height);
    hBox->addWidget(fdHeight);
    hBox->addStretch();

    hBox = new HBox(vBox);

    label = new QLabel(tr("Remarks"));
    label->setMinimumWidth(90);
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    hBox->addWidget(label);

    fdRemark = new QTextEdit(remarks);
    fdRemark->setFixedHeight(80);
    hBox->addWidget(fdRemark);

    hBox = new HBox(vBox);
    hBox->addSpacing(72);

    edIsInsert = new QCheckBox(tr("Is Insert"));
    hBox->addWidget(edIsInsert);

    vBox->addSpacing(6);

    hBox = new HBox(vBox);
    hBox->addSpacing(72);

    fdIsUltraLong = new QCheckBox(tr("Ultra-Long Screen Split"));
    hBox->addWidget(fdIsUltraLong);

    fdHor = new QRadioButton(tr("Horizontal"));
    hBox->addWidget(fdHor);

    fdVer = new QRadioButton(tr("Vertical"));
    hBox->addWidget(fdVer);
    hBox->addStretch();

    if(isVer) fdVer->setChecked(true);
    else fdHor->setChecked(true);

    hBox = new HBox(vBox);
    auto lbSplitWidth = new QLabel(tr("Lengths of Parts"));
    lbSplitWidth->setMinimumWidth(90);
    lbSplitWidth->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    hBox->addWidget(lbSplitWidth);

    fdSplitWidths = new QLineEdit(widths.isEmpty() ? "1920" : widths);
    fdSplitWidths->setPlaceholderText("1920");
    hBox->addWidget(fdSplitWidths);

    connect(fdIsUltraLong, &QCheckBox::toggled, this, [=](bool checked) {
        fdHor->setVisible(checked);
        fdVer->setVisible(checked);
        lbSplitWidth->setVisible(checked);
        fdSplitWidths->setVisible(checked);
    });
    if(widths.isEmpty()) fdIsUltraLong->toggled(false);
    else fdIsUltraLong->setChecked(true);

    if(! gWidthSplit) {
        fdIsUltraLong->setVisible(false);
        lbSplitWidth->setVisible(false);
        fdSplitWidths->setVisible(false);
    }

    vBox->addSpacing(6);

    auto btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(btnBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    vBox->addWidget(btnBox);
}
