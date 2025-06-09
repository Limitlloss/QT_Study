#include "evideo.h"
#include "main.h"
#include "base/ffutil.h"
#include <QLineEdit>
#include <QMessageBox>
#include <QMetaEnum>
#include <QProcess>
#include <QSettings>
#include <QSpinBox>
#include <QPainter>

EVideo *EVideo::create(const QString &file, PageListItem *pageItem, EBase *multiWin) {
    int64_t dur;
    AVCodecID codecId;
    QImage img;
    auto err = videoInfo(file.toUtf8(), img, &dur, &codecId);
    if(! err.isEmpty()) {
        QMessageBox::critical(pageItem->listWidget(), "Video Error", err+"\n"+file);
        return 0;
    }
    QFileInfo rawInfo(file);
    auto rawName = rawInfo.fileName();
    auto outFile = transcoding(pageItem->listWidget(), file, rawName, pageItem->mPageDir, img.width(), img.height(), codecId);
    if(outFile.isEmpty()) return 0;
    QFileInfo outInfo(outFile);
    if(! outInfo.isFile() || outInfo.size()==0) return 0;
    auto ins = new EVideo(outInfo.absolutePath(), outInfo.fileName(), rawInfo.absolutePath(), rawName, img, pageItem, multiWin);
    ins->_duration = round(dur*0.000001);
    return ins;
}
EVideo *EVideo::create(const JObj &ele, PageListItem *pageItem, EBase *multiWin) {
    auto widget = ele["widget"];
    if(widget.isNull()) widget = ele;
    auto dir = widget["path"].toString();
    auto name = widget["file"].toString();
    if(! QFileInfo::exists(dir)) dir = pageItem->mPageDir;
    auto file = dir + "/" + name;
    if(QFileInfo::exists(file)) ;
    else if(QFileInfo::exists(file = pageItem->mPageDir + "/" + name)) dir = pageItem->mPageDir;
    else return 0;
    int64_t dur;
    QImage img;
    auto err = videoInfo(file.toUtf8(), img, &dur, 0);
    if(! err.isEmpty()) return 0;
    dur = round(dur*0.000001);
    auto ins = new EVideo(dir, name, widget["pathRaw"].toString(), widget["fileRaw"].toString(), img, pageItem, multiWin);
    ins->setBaseAttr(ele);
    if(ins->_duration < 4) ins->_duration = dur;
    ins->useSW = ele["useSW"].toBool();
    auto play = ele["play"];
    ins->playTimes = (play.isNull() ? ele : play)["playTimes"].toInt(1);
    return ins;
}

EVideo::EVideo(const QString &dir, const QString &name, const QString &rawDir, const QString &rawName, QImage &coverImg, PageListItem *pageItem, EBase *multiWin)
    : EBase(multiWin), mDir(dir), mName(name), mRawDir(rawDir), mRawName(rawName), mCoverImg(coverImg), mPageItem(pageItem) {
    mType = EBase::Video;
}

void EVideo::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    auto inner = innerRect();
    painter->save();
    painter->setRenderHints(QPainter::SmoothPixmapTransform);
    painter->drawImage(inner, mCoverImg);
    double maskW = mask().width()/2, maskH = mask().height()/2;
    if(maskW>inner.width() || maskH>inner.height()) {
        double rate = qMin(inner.width() / maskW, inner.height() / maskH);
        maskW *= rate;
        maskH *= rate;
    }
    painter->drawImage(QRectF(inner.left()+(inner.width() - maskW)/2, inner.top()+(inner.height() - maskH)/2, maskW, maskH), mask());
    painter->restore();
    EBase::paint(painter, option, widget);
}

QWidget* EVideo::attrWgt() {
    auto wgtAttr = new QWidget;
    auto vBox = new QVBoxLayout(wgtAttr);
    vBox->setContentsMargins(6, 0, 6, 0);
    if(mMultiWin) vBox->setSpacing(3);

    addBaseAttrWgt(vBox);

    auto hBox = new HBox(vBox);
    hBox->addLabel(translate("","Basic Properties"));

    auto line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    hBox->addWidget(line, 1);

    hBox = new HBox(vBox);
    hBox->addSpacing(6);
    hBox->addLabel(tr("File"));

    auto fdFileName = new QLineEdit(mRawName);
    fdFileName->setReadOnly(true);
    hBox->addWidget(fdFileName);

    auto bnSelectFile = new QPushButton("...");
    bnSelectFile->setFixedWidth(30);
    bnSelectFile->setObjectName("bnSelectFile");
    connect(bnSelectFile, &QPushButton::clicked, this, [=] {
        auto rawFile = QFileDialog::getOpenFileName(wgtAttr, translate("","Select File"), gFileHome, EVideo::filters());
        if(rawFile.isEmpty()) return;
        QFileInfo rawInfo(rawFile);
        int64_t dur;
        AVCodecID codecId;
        auto err = videoInfo(rawFile.toUtf8(), mCoverImg, &dur, &codecId);
        if(! err.isEmpty()) {
            QMessageBox::critical(wgtAttr, "Video Error", err+"\n"+rawFile);
            return;
        };
        mRawDir = rawInfo.absolutePath();
        mRawName = rawInfo.fileName();
        gFileHome = mRawDir;
        fdFileName->setText(mRawName);
        _duration = round(dur*0.000001);
        edDuration->setValue(_duration);
        auto outFile = transcoding(wgtAttr, rawFile, mRawName, mPageItem->mPageDir, mCoverImg.width(), mCoverImg.height(), codecId);
        if(outFile.isEmpty()) return;
        QFileInfo outInfo(outFile);
        mDir = outInfo.absolutePath();
        mName = outInfo.fileName();
    });
    hBox->addWidget(bnSelectFile);

//    hBox = new HBox(vBox);
//    hBox->addSpacing(6);
//    hBox->addLabel(tr("AspectRatioMode"));

//    auto wAspectRatioMode = new QComboBox();
//    wAspectRatioMode->addItem(tr("IgnoreAspectRatio"));
//    wAspectRatioMode->addItem(tr("KeepAspectRatio"));
//    wAspectRatioMode->addItem(tr("KeepAspectRatioByExpanding"));
//    wAspectRatioMode->setCurrentIndex(aspectRatioMode);
//    connect(wAspectRatioMode, (void(QComboBox::*)(int))&QComboBox::currentIndexChanged, this, [this](int value) {
//        aspectRatioMode = value;
//    });
//    hBox->addWidget(wAspectRatioMode);
//    hBox->addStretch();

    hBox = new HBox(vBox);
    hBox->addLabel(tr("Play Properties"));

    line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    hBox->addWidget(line, 1);

    hBox = new HBox(vBox);
    hBox->addSpacing(6);
    hBox->addLabel(tr("Play Times"));

    auto edPlayTimes = new QSpinBox;
    edPlayTimes->setRange(1, 99999);
    edPlayTimes->setValue(playTimes);
    connect(edPlayTimes, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, [this](int value) {
        playTimes = value;
    });
    hBox->addWidget(edPlayTimes);
    hBox->addStretch();

    hBox = new HBox(vBox);
    hBox->addSpacing(6);

    auto edUseSW = new QCheckBox(tr("Use SW"));
    edUseSW->setChecked(useSW);
    connect(edUseSW, &QCheckBox::stateChanged, this, [this](int value) {
        useSW = value==Qt::Checked;
    });
    hBox->addWidget(edUseSW);
    hBox->addStretch();

    vBox->addStretch();
    return wgtAttr;
}

bool EVideo::save(const QString &pageDir) {
    QString saveFile = pageDir + "/" + mName;
    if(QFile::exists(saveFile)) return true;
    QString oldFile = mDir + PAGEDEL_SUFFIX "/" + mName;
    if(QFile::exists(oldFile)) ;
    else if(QFile::exists(oldFile = mDir + "/" + mName)) ;
    else return false;
    QFile(oldFile).copy(saveFile);
    mDir = pageDir;
    // if(gProgItem->maxLen) {
    //     auto waitingDlg = new WaitingDlg(mPageItem->listWidget()->window(), "正在转码视频 ...");
    //     auto thread = new VideoSplitThread(mWidth, mHeight, gProgItem->maxLen, gProgItem->isVer ? gProgItem->mWidth : gProgItem->mHeight, gProgItem->partLens, gProgItem->isVer, pos(), saveFile.toUtf8());
    //     connect(thread, &VideoSplitThread::emErr, this, [=](QString err) {
    //         waitingDlg->close();
    //         if(! err.isEmpty()) QMessageBox::critical(mPageItem->listWidget()->window(), "Video trans error", err+"\n"+saveFile);
    //     });
    //     connect(thread, &VideoSplitThread::emProgress, this, [saveFile, waitingDlg](int progress) {
    //         waitingDlg->fdText->setText(QString("正在转码视频 %1%").arg(progress));
    //     });
    //     thread->start();
    //     waitingDlg->exec();
    // }
    return true;
}

JObj EVideo::attrJson() const {
    JObj obj{
        {"elementType", "Video"},
        {"path", mDir},
        {"file", mName},
        {"pathRaw", mRawDir},
        {"fileRaw", mRawName},
        {"useSW", useSW},
        {"playTimes", playTimes}
    };
    addBaseAttr(obj);
    return obj;
}
