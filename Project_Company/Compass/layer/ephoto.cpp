#include "ephoto.h"
#include "tools.h"
#include "main.h"
#include <QComboBox>
#include <QFileDialog>
#include <QImageReader>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QSpinBox>

EPhoto *EPhoto::create(const QString &file, PageListItem *pageItem, EBase *multiWin) {
    QImageReader reader(file);
    reader.setAutoTransform(true);
    auto img = reader.read();
    if(img.isNull()) {
        QMessageBox::critical(pageItem->listWidget(), "Image Error", Tools::readErrStr(reader.error())+": "+reader.errorString()+"\n"+file);
        return 0;
    }
    QFileInfo info(file);
    return new EPhoto(img, info.absolutePath(), info.fileName(), JObj(), pageItem, multiWin);
}
EPhoto *EPhoto::create(const JObj &json, PageListItem *pageItem, EBase *multiWin) {
    auto widget = json["widget"];
    auto dir = (widget.isNull() ? json["dir"] : widget["path"]).toString();
    auto name = (widget.isNull() ? json["name"] : widget["file"]).toString();
    if(! QFileInfo(dir).isDir()) dir = pageItem->mPageDir;
    QString file = dir + "/" + name;
    QFileInfo fileInfo(file);
    if(! fileInfo.isFile()) {
        QString file2 = dir + "/" + widget["yuanshi_file"].toString();
        if(QFileInfo(file2).isFile()) QFile::rename(file2, file);
        else if(QFileInfo(file2 = widget["computer_pic_file"].toString()).isFile()) QFile::copy(file2, file);
        else if(QFileInfo(file2 = dir + "/card_"+name).isFile()) QFile::rename(file2, file);
        else return nullptr;
    }
    QImageReader reader(file);
    reader.setAutoTransform(true);
    auto img = reader.read();
    if(img.isNull()) return 0;
    auto ins = new EPhoto(img, dir, name, json, pageItem, multiWin);
    ins->setBaseAttr(json);
    return ins;
}

EPhoto::EPhoto(const QImage &img, const QString &dir, const QString &name, const JObj &json, PageListItem *pageItem, EBase *multiWin) : EBase(multiWin), img(img), mDir(dir), mName(name), mPageItem(pageItem) {
    mType = EBase::Image;
    direct = json["direct"].toString();
    speed = json["speed"].toInt();
    tailSpacing = json["tailSpacing"].toInt();
    scaleImgIfNeed();
}
JObj EPhoto::attrJson() const {
    JObj json;
    addBaseAttr(json);
    json["elementType"] = "Image";
    json["dir"] = mDir;
    json["name"] = mName;
    json["direct"] = direct;
    json["speed"] = speed;
    json["tailSpacing"] = tailSpacing;
    return json;
}
void EPhoto::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->drawImage(innerRect(), img);
    EBase::paint(painter, option, widget);
}
void EPhoto::freeFiles() {
    if(img.isNull()) return;
    img = QImage();
}
void EPhoto::loadFiles() {
    if(! img.isNull()) return;
    if(! QFileInfo::exists(mDir + "/" + mName)) return;
    img = QImage(mDir + "/" + mName);
}
bool EPhoto::save(const QString &pageDir) {
    QString newName = mName;
    QString newFile = pageDir + "/" + newName;
    if(QFileInfo::exists(newFile)) {
        newFile = Tools::addSufix(newFile);
        newName = QFileInfo(newFile).fileName();
    }
    QString oldFile = mDir + PAGEDEL_SUFFIX + "/" + mName;
    if(QFileInfo::exists(oldFile)) QFile(oldFile).copy(newFile);
    else if(pageDir!=mDir && QFileInfo::exists(oldFile = mDir + "/" + mName)) QFile(oldFile).copy(newFile);
    else if(QFileInfo::exists(pageDir + "/" + mName)) newName = mName;
    mDir = pageDir;
    mName = newName;
    return true;
}

QWidget* EPhoto::attrWgt() {
    auto wgtAttr = new QWidget;
    auto vBox = new VBox(wgtAttr);
    vBox->setContentsMargins(6, 0, 6, 0);
    if(mMultiWin) vBox->setSpacing(3);

    addBaseAttrWgt(vBox);

    auto hBox = new HBox(vBox);
    hBox->addWidget(new QLabel(translate("","Basic Properties")));

    auto line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    hBox->addWidget(line, 1);

    hBox = new HBox(vBox);
    hBox->addSpacing(6);
    hBox->addWidget(new QLabel(tr("File")));

    auto fdFile = new QLineEdit(mName);
    fdFile->setReadOnly(true);
    hBox->addWidget(fdFile);

    auto bnSelectFile = new QPushButton("...");
    bnSelectFile->setFixedWidth(30);
    bnSelectFile->setObjectName("bnSelectFile");
    connect(bnSelectFile, &QPushButton::clicked, wgtAttr, [=] {
        QString home = mDir.startsWith(gProgItem->mProgDir) ? gFileHome : mDir;
        QString file = QFileDialog::getOpenFileName(wgtAttr, translate("","Select File"), home, EPhoto::filters());
        if(file.isEmpty()) return;
        QImageReader reader(file);
        reader.setAutoTransform(true);
        auto aimg = reader.read();
        if(aimg.isNull()) {
            QMessageBox::critical(wgtAttr, tr("Image Read Error"), Tools::readErrStr(reader.error())+": "+reader.errorString()+"\n"+file);
            return;
        }
        img = aimg;
        QFileInfo info(file);
        mDir = info.absolutePath();
        gFileHome = mDir;
        mName = info.fileName();
        scaleImgIfNeed();
        fdFile->setText(mName);
    });
    hBox->addWidget(bnSelectFile);


    hBox = new HBox(vBox);
    auto label = hBox->addLabel(tr("Scroll")+": "+tr("Direction"));
    label->setMinimumWidth(80);
    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

    auto edDirect = new QComboBox;
    edDirect->addItem(tr("No"), "");
    edDirect->addItem(tr("Right -> Left"), "left");
    edDirect->addItem(tr("Bottom -> Top"), "top");
    edDirect->addItem(tr("Left -> Right"), "right");
    edDirect->addItem(tr("Top -> Bottom"), "bottom");
    SetCurData(edDirect, direct);
    connect(edDirect, (void(QComboBox::*)(int))&QComboBox::currentIndexChanged, this, [=] {
        direct = edDirect->currentData().toString();
    });
    hBox->addWidget(edDirect);

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

    // hBox = new HBox(vBox);
    // label = hBox->addLabel(tr("Tail Spacing"));
    // label->setMinimumWidth(80);
    // label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

    // auto edTailSpacing = new QSpinBox;
    // edTailSpacing->setRange(0, 9999);
    // edTailSpacing->setValue(tailSpacing);
    // connect(edTailSpacing, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, [this](int value) {
    //     tailSpacing = value;
    // });
    // hBox->addWidget(edTailSpacing);
    // hBox->addStretch();

    // hBox = new HBox(vBox);
    // hBox->addWidget(new QLabel(tr("Play Properties")));

    // line = new QFrame();
    // line->setFrameShape(QFrame::HLine);
    // line->setFrameShadow(QFrame::Sunken);
    // hBox->addWidget(line, 1);

    // hBox = new HBox(vBox);
    // hBox->setSpacing(2);
    // hBox->addSpacing(6);
    // hBox->addWidget(new QLabel(tr("Entry Effect")));

    // auto fdEntryEff = new QComboBox;
    // fdEntryEff->addItem(tr("None"), "");
    // fdEntryEff->addItem(tr("Move to left"), "moving_left");
    // fdEntryEff->addItem(tr("Move to top"), "moving_top");
    // fdEntryEff->addItem(tr("Move to right"), "moving_right");
    // fdEntryEff->addItem(tr("Move to bottom"), "moving_bottom");
    // fdEntryEff->addItem(tr("Alpha in"), "alpha_in");
    // fdEntryEff->addItem(tr("Zoom in"), "zoom_in");
    // fdEntryEff->addItem(tr("Zoom in from left-top"), "zoom_in_left_top");
    // fdEntryEff->addItem(tr("Zoom in from right-top"), "zoom_in_right_top");
    // fdEntryEff->addItem(tr("Zoom in from right-bottom"), "zoom_in_right_bottom");
    // fdEntryEff->addItem(tr("Zoom in from left-bottom"), "zoom_in_left_bottom");
    // fdEntryEff->addItem(tr("Rotate to right"), "rotate_right");
    // fdEntryEff->addItem(tr("Rotate to left"), "rotate_left");
    // fdEntryEff->addItem(tr("Random"), "Random");
    // SetCurData(fdEntryEff, mEntryEffect);
    // connect(fdEntryEff, (void(QComboBox::*)(int))&QComboBox::currentIndexChanged, this, [=] {
    //     mEntryEffect = fdEntryEff->currentData().toString();
    // });
    // hBox->addWidget(fdEntryEff);
    // hBox->addStretch();

    // hBox->addLabel(tr("Dur"));

    // auto fdEntryDur = new QSpinBox;
    // fdEntryDur->setRange(1, 99);
    // fdEntryDur->setValue(mEntryDur);
    // connect(fdEntryDur, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, [=](int value) {
    //     mEntryDur = value;
    //     if(mDuration < value) {
    //         mDuration = value;
    //         fdDuration->setValue(value);
    //     }
    // });
    // hBox->addWidget(fdEntryDur);
    // hBox->addLabel(tr("s"));

    // hBox = new HBox(vBox);
    // hBox->setSpacing(2);
    // hBox->addSpacing(6);
    // hBox->addWidget(new QLabel(tr("Exit Effect")));

    // auto fdExitEff = new QComboBox;
    // fdExitEff->addItem(tr("None"), "");
    // fdExitEff->addItem(tr("Move to left"), "moving_left");
    // fdExitEff->addItem(tr("Move to top"), "moving_top");
    // fdExitEff->addItem(tr("Move to right"), "moving_right");
    // fdExitEff->addItem(tr("Move to bottom"), "moving_bottom");
    // fdExitEff->addItem(tr("Alpha out"), "alpha_out");
    // fdExitEff->addItem(tr("Zoom out"), "zoom_out");
    // fdExitEff->addItem(tr("Zoom out to left-top"), "zoom_out_left_top");
    // fdExitEff->addItem(tr("Zoom out to right-top"), "zoom_out_right_top");
    // fdExitEff->addItem(tr("Zoom out to right-bottom"), "zoom_out_right_bottom");
    // fdExitEff->addItem(tr("Zoom out to left-bottom"), "zoom_out_left_bottom");
    // fdExitEff->addItem(tr("Rotate to right"), "rotate_right");
    // fdExitEff->addItem(tr("Rotate to left"), "rotate_left");
    // fdExitEff->addItem(tr("Random"), "Random");
    // SetCurData(fdExitEff, mExitEffect);
    // connect(fdExitEff, (void(QComboBox::*)(int))&QComboBox::currentIndexChanged, this, [=] {
    //     mExitEffect = fdExitEff->currentData().toString();
    // });
    // hBox->addWidget(fdExitEff);
    // hBox->addStretch();

    // hBox->addLabel(tr("Dur"));

    // auto fdExitDur = new QSpinBox;
    // fdExitDur->setRange(1, 99);
    // fdExitDur->setValue(mExitDur);
    // connect(fdExitDur, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, [=](int value) {
    //     mExitDur = value;
    //     if(mDuration < value) {
    //         mDuration = value;
    //         fdDuration->setValue(value);
    //     }
    // });
    // hBox->addWidget(fdExitDur);
    // hBox->addLabel(tr("s"));

    vBox->addStretch();
    return wgtAttr;
}

void EPhoto::scaleImgIfNeed() {
    if(img.width() > 4096 || img.height() > 4096) {
        if(img.width() > img.height()) img = img.scaledToWidth(3840, Qt::SmoothTransformation);
        else img = img.scaledToHeight(3840, Qt::SmoothTransformation);
        if(mName.endsWith(".jpg", Qt::CaseInsensitive)) mName = mName.chopped(3)+"png";
        else if(mName.endsWith(".jpeg", Qt::CaseInsensitive)) mName = mName.chopped(4)+"png";
        mDir = mPageItem->mPageDir;
        QString save = mDir + "/" + mName;
        if(QFileInfo::exists(save)) {
            save = Tools::addSufix(save);
            mName = QFileInfo(save).fileName();
        }
        img.save(save);
    }
}
