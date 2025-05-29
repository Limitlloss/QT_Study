#include "mediapanel.h"
#include "main.h"
#include <QApplication>
#include <QMessageBox>
#include <QStandardPaths>
#include <QProgressBar>
#include <QFileDialog>
#include <QLineEdit>
#include <QProcess>
#include <QInputDialog>
#include <QDropEvent>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QVideoSink>
#include <QVideoFrame>

MediaTree *gMediaTree;

MediaPanel::MediaPanel(QWidget *parent) : QWidget(parent) {
    auto vBox = new VBox(this);
    vBox->setContentsMargins(0, 0, 0, 0);
    vBox->setSpacing(0);
    auto hBox = new HBox(vBox);

    auto bnAdd = new QPushButton("🞥");
    bnAdd->setMaximumWidth(50);
    hBox->addWidget(bnAdd);
    connect(bnAdd, &QPushButton::clicked, this, [this] {
        auto file = QFileDialog::getOpenFileName(this, 0, gFileHome);
        if(file.isEmpty()) return;
        QFileInfo info(file);
        auto dir = info.absolutePath();
        auto name = info.fileName();
        gFileHome = dir;
        auto suffix = info.suffix().toLower();
        MediaItem *item = 0;
        if(suffix.startsWith("mp")) {
            auto player = new QMediaPlayer;
            player->setSource(QUrl::fromLocalFile(file));
            item = new MediaItem(file, gMediaTree);
            item->setText("type", "Video");
            auto videoWgt = new QVideoWidget;
            player->setVideoOutput(videoWgt);
            auto videoSink = videoWgt->videoSink();
            connect(videoSink, &QVideoSink::videoFrameChanged, player, [=](const QVideoFrame &frame) {
                disconnect(videoSink, &QVideoSink::videoFrameChanged, player, 0);
                player->stop();
                player->deleteLater();
                videoWgt->deleteLater();
                qDebug()<<"pixelFormat"<<frame.pixelFormat();
                item->setText("size", QString("%1×%2").arg(frame.width()).arg(frame.height()));
                item->setText("dur", QTime::fromMSecsSinceStartOfDay(player->duration()).toString("hh:mm:ss.zzz"));
                item->profile = frame.toImage().scaledToHeight(60, Qt::SmoothTransformation);
                auto edProfile = new QLabel;
                edProfile->setPixmap(QPixmap::fromImage(item->profile));
                edProfile->setScaledContents(true);
                edProfile->setMaximumHeight(24);
                item->setCellWidget("profile", edProfile);
            });
            player->play();
        } else if(suffix == "png" || suffix.startsWith("jp") || suffix == "gif") {
            QImageReader reader(file);
            reader.setAutoTransform(true);
            auto img = reader.read();
            if(img.isNull()) {
                QMessageBox::critical(this, "Image Read Error", QString::number(reader.error())+" "+reader.errorString());
                return;
            }
            item = new MediaItem(file, gMediaTree);
            item->setText("type", "Image");
            item->setText("dur", "00:10:00.000");
            item->setText("size", QString("%1×%2").arg(img.width()).arg(img.height()));
            item->profile = img.scaledToHeight(60, Qt::SmoothTransformation);
            auto edProfile = new QLabel;
            edProfile->setPixmap(QPixmap::fromImage(item->profile));
            edProfile->setScaledContents(true);
            edProfile->setMaximumHeight(24);
            item->setCellWidget("profile", edProfile);
        }
        if(item) {
            item->setText("name", name);
        }
    });

    auto bnDelet = new QPushButton("🗑");
    bnDelet->setMaximumWidth(50);
    hBox->addWidget(bnDelet);
    connect(bnDelet, &QPushButton::clicked, this, [=] {
        // for(int i=0; i<tree->topLevelItemCount(); i++) if(tree->item(i)->checkState("check") == Qt::Checked) {
        //     auto item = (MediaItem*) tree->topLevelItem(i--);
        //     item->del();
        //     delete item;
        // }
    });

    hBox->addStretch();

    auto fdSearch = new QLineEdit;
    fdSearch->setMinimumWidth(100);
    auto search = new QAction;
    search->setIcon(QIcon(":/res/program/bnSearch.png"));
    fdSearch->addAction(search, QLineEdit::LeadingPosition);
    fdSearch->setClearButtonEnabled(true);
    //fdSearch->setStyleSheet("border: 1px solid #888;");
    connect(fdSearch, &QLineEdit::textChanged, this, [](const QString &text) {
        auto cnt = gMediaTree->topLevelItemCount();
        for(int i=0; i<cnt; i++) {
            auto item = gMediaTree->item(i);
            item->setHidden(! (text.isEmpty() || item->text("name").contains(text) || item->text("resolution").contains(text)));
        }
    });
    hBox->addWidget(fdSearch);

    gMediaTree = new MediaTree;
    gMediaTree->addCol("#", "", 20);
    gMediaTree->addCol("profile", "", 42);
    gMediaTree->addCol("name", "", 140);
    gMediaTree->addCol("type", "", 45);
    gMediaTree->addCol("size", "", 60);
    gMediaTree->addCol("dur", "", 80);
    gMediaTree->setDefs()->setHeaderAlignC();
    gMediaTree->minRowHeight = 26;
    gMediaTree->setSortingEnabled(true);
    gMediaTree->setDragEnabled(true);
    gMediaTree->setAcceptDrops(true);
    gMediaTree->setDropIndicatorShown(true);
    vBox->addWidget(gMediaTree);

    auto dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if(mProgsDir.isEmpty()) return;
    gMediaTree->clear();
    // for(auto &progName : progNames) {
    //     auto file = mProgsDir + "/" + progName + "/pro.json";
    //     QFile qFile(file);
    //     if(! qFile.exists()) continue;
    //     if(! qFile.open(QIODevice::ReadOnly)) continue;
    //     auto data = qFile.readAll();
    //     qFile.close();
    //     QString error;
    //     auto json = JFrom(data, &error);
    //     if(! error.isEmpty()) continue;
    //     auto item = new MediaItem(tree);
    //     item->dir = dir;
    //     item->setText("name", name);
    //     item->setText("resolution", QString("%1 x %2").arg(mWidth).arg(mHeight));
    // }

    QSettings settings;
    gMediaTree->sortByColumn(settings.value("MediaSortColumn").toInt(), (Qt::SortOrder)settings.value("MediaSortOrder").toInt());

    transUi();
}

void MediaPanel::changeEvent(QEvent *event) {
    QWidget::changeEvent(event);
    if(event->type() == QEvent::LanguageChange) transUi();
}
void MediaPanel::transUi() {
    gMediaTree->headerItem()->setText("name"**gMediaTree, tr("Name"));
    gMediaTree->headerItem()->setText("type"**gMediaTree, tr("Type"));
    gMediaTree->headerItem()->setText("size"**gMediaTree, tr("Size"));
    gMediaTree->headerItem()->setText("dur"**gMediaTree, tr("Duration"));
}

void MediaTree::dropEvent(QDropEvent *event) {
    if(MediaTree::OnItem==dropIndicatorPosition()) {
        event->ignore();
        return;
    }
    TreeWidget::dropEvent(event);
}
