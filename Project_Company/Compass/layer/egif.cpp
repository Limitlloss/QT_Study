#include "egif.h"
#include "tools.h"
#include "main.h"
#include <QBoxLayout>
#include <QDir>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMovie>
#include <QPainter>
#include <QPushButton>
#include <QSpinBox>

EGif *EGif::create(const QString &file, PageListItem *pageItem, EBase *multiWin) {
    auto movie = new QMovie(file);
    if(! movie->isValid()) {
        QMessageBox::critical(pageItem->listWidget(), "Gif Error", Tools::readErrStr(movie->lastError())+": "+movie->lastErrorString()+"\n"+file);
        delete movie;
        return 0;
    }
    movie->setCacheMode(QMovie::CacheAll);
    movie->jumpToFrame(0);
    QFileInfo info(file);
    return new EGif(movie, info.absolutePath(), info.fileName(), pageItem, multiWin);
}
EGif *EGif::create(const JObj &json, PageListItem *pageItem, EBase *multiWin) {
    auto widget = json["widget"];
    auto dir = widget["path"].toString();
    auto name = widget["file"].toString();
    if(! QFileInfo::exists(dir)) dir = pageItem->mPageDir;
    QString file = dir + "/" + name;
    if(QFileInfo::exists(file)) ;
    else if(QFileInfo::exists(file = pageItem->mPageDir + "/" + name)) dir = pageItem->mPageDir;
    else return nullptr;
    auto movie = new QMovie(file);
    if(! movie->isValid()) {
        delete movie;
        return 0;
    }
    movie->setCacheMode(QMovie::CacheAll);
    movie->jumpToFrame(0);
    auto ins = new EGif(movie, dir, name, pageItem, multiWin);
    ins->setBaseAttr(json);
    return ins;
}

EGif::EGif(QMovie *movie, const QString &dir, const QString &name, PageListItem *pageItem, EBase *multiWin) : EBase(multiWin), mMovie(movie), mDir(dir), mName(name), mPageItem(pageItem) {
    mType = EBase::Gif;
}
EGif::~EGif() {
    if(mMovie) delete mMovie;
}
void EGif::paint(QPainter *painter, const QStyleOptionGraphicsItem *a, QWidget *b) {
    if(mMovie) {
        painter->setRenderHints(QPainter::Antialiasing|QPainter::SmoothPixmapTransform);
        painter->drawPixmap(innerRect(), mMovie->currentPixmap(), QRectF());
        if(timer==0) {
            auto delay = mMovie->nextFrameDelay();
            if(delay) {
                timer = new SyncTimer(delay);
                connect(timer, &SyncTimer::timeout, this, &EGif::sltNext, Qt::BlockingQueuedConnection);
                timer->start();
            }
        }
    }
    EBase::paint(painter, a, b);
}
void EGif::sltNext() {
    if(isVisible() && mMovie) {
        mMovie->jumpToNextFrame();
        timer->inter = mMovie->nextFrameDelay();
        update();
    } else if(timer) {
        timer->stop();
        timer = 0;
    }
}
void EGif::loadFiles() {
    if(mMovie) return;
    if(! QFileInfo::exists(mDir + "/" + mName)) return;
    mMovie = new QMovie(mDir + "/" + mName, QByteArray());
    mMovie->setCacheMode(QMovie::CacheAll);
    mMovie->jumpToFrame(0);
}
void EGif::freeFiles() {
    if(mMovie==0) return;
    delete mMovie;
    mMovie = 0;
}

QWidget* EGif::attrWgt() {
    auto wgtAttr = new QWidget;
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
    hBox->addWidget(new QLabel(tr("File")));

    auto wFile = new QLineEdit(mName);
    wFile->setReadOnly(true);
    hBox->addWidget(wFile);

    auto bnSelectFile = new QPushButton("...");
    bnSelectFile->setFixedWidth(30);
    bnSelectFile->setObjectName("bnSelectFile");
    connect(bnSelectFile, &QPushButton::clicked, wgtAttr, [=] {
        auto file = QFileDialog::getOpenFileName(wgtAttr, translate("","Select File"), gFileHome, EGif::filters());
        if(file.isEmpty()) return;
        auto movie = new QMovie(file);
        if(! movie->isValid()) {
            QMessageBox::critical(wgtAttr, "Gif Error", Tools::readErrStr(movie->lastError())+": "+movie->lastErrorString()+"\n"+file);
            delete movie;
            return;
        }
        movie->setCacheMode(QMovie::CacheAll);
        movie->jumpToFrame(0);
        QFileInfo fileInfo(file);
        mName = fileInfo.fileName();
        wFile->setText(mName);
        mDir = fileInfo.absolutePath();
        gFileHome = mDir;
        if(mMovie) delete mMovie;
        mMovie = movie;
    });
    hBox->addWidget(bnSelectFile);

    vBox->addLayout(hBox);

    vBox->addStretch();
    return wgtAttr;
}

bool EGif::save(const QString &pageDir) {
    QString oldFile = mDir + PAGEDEL_SUFFIX + "/" + mName;
    if(QFileInfo::exists(oldFile)) ;
    else if(QFileInfo::exists(oldFile = mDir + "/" + mName)) ;
    else return false;
    QFile::copy(oldFile, pageDir + "/" + mName);
    mDir = pageDir;
    return true;
}

JObj EGif::attrJson() const {
    JObj oRoot;
    addBaseAttr(oRoot);
    oRoot["elementType"] = "Gif";
    oRoot["widget"] = JObj{
        {"file", mName},
        {"path", mDir}
    };
    return oRoot;
}
