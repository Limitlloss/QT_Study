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

// 定义全局媒体树指针
MediaTree* gMediaTree;

// MediaPanel 类构造函数，初始化媒体面板
MediaPanel::MediaPanel(QWidget* parent) : QWidget(parent) {
    // 设置垂直布局和水平布局
    auto vBox = new VBox(this);
    vBox->setContentsMargins(0, 0, 0, 0);
    vBox->setSpacing(0);
    auto hBox = new HBox(vBox);

    // 创建添加按钮并设置其属性
    auto bnAdd = new QPushButton("🞥");
    bnAdd->setMaximumWidth(50);
    hBox->addWidget(bnAdd);
    connect(bnAdd, &QPushButton::clicked, this, [this] {
        // 打开文件对话框选择文件
        auto file = QFileDialog::getOpenFileName(this, 0, gFileHome);
        if (file.isEmpty()) return;
        QFileInfo info(file);
        auto dir = info.absolutePath();
        auto name = info.fileName();
        gFileHome = dir;
        auto suffix = info.suffix().toLower();
        MediaItem* item = 0;
        // 根据文件后缀处理媒体文件
        if (suffix.startsWith("mp")) {
            auto player = new QMediaPlayer;
            player->setSource(QUrl::fromLocalFile(file));
            item = new MediaItem(file, gMediaTree);
            item->setText("type", "Video");
            auto videoWgt = new QVideoWidget;
            player->setVideoOutput(videoWgt);
            auto videoSink = videoWgt->videoSink();
            connect(videoSink, &QVideoSink::videoFrameChanged, player, [=](const QVideoFrame& frame) {
                // 处理视频帧
                player->stop();
                player->deleteLater();
                videoWgt->deleteLater();
                qDebug() << "pixelFormat" << frame.pixelFormat();
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
        }
        else if (suffix == "png" || suffix.startsWith("jp") || suffix == "gif") {
            // 处理图片文件
            QImageReader reader(file);
            reader.setAutoTransform(true);
            auto img = reader.read();
            if (img.isNull()) {
                QMessageBox::critical(this, "Image Read Error", QString::number(reader.error()) + " " + reader.errorString());
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
        // 设置媒体项名称
        if (item) {
            item->setText("name", name);
        }
        });

    // 创建删除按钮并设置其属性
    auto bnDelet = new QPushButton("🗑");
    bnDelet->setMaximumWidth(50);
    hBox->addWidget(bnDelet);
    connect(bnDelet, &QPushButton::clicked, this, [=] {
        // 删除选中的媒体项
        });

    // 添加水平布局拉伸
    hBox->addStretch();

    // 创建搜索框并设置其属性
    auto fdSearch = new QLineEdit;
    fdSearch->setMinimumWidth(100);
    auto search = new QAction;
    search->setIcon(QIcon(":/res/program/bnSearch.png"));
    fdSearch->addAction(search, QLineEdit::LeadingPosition);
    fdSearch->setClearButtonEnabled(true);
    connect(fdSearch, &QLineEdit::textChanged, this, [](const QString& text) {
        // 根据输入的文本搜索媒体项
        });
    hBox->addWidget(fdSearch);

    // 初始化媒体树
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

    // 获取应用程序数据目录并初始化媒体树
    auto dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (mProgsDir.isEmpty()) return;
    gMediaTree->clear();

    // 根据设置对媒体树进行排序
    QSettings settings;
    gMediaTree->sortByColumn(settings.value("MediaSortColumn").toInt(), (Qt::SortOrder)settings.value("MediaSortOrder").toInt());

    // 翻译用户界面
    transUi();
}

// 处理语言改变事件
void MediaPanel::changeEvent(QEvent* event) {
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) transUi();
}

// 翻译用户界面元素
void MediaPanel::transUi() {
    gMediaTree->headerItem()->setText("name" * *gMediaTree, tr("Name"));
    gMediaTree->headerItem()->setText("type" * *gMediaTree, tr("Type"));
    gMediaTree->headerItem()->setText("size" * *gMediaTree, tr("Size"));
    gMediaTree->headerItem()->setText("dur" * *gMediaTree, tr("Duration"));
}

// 处理拖放事件
void MediaTree::dropEvent(QDropEvent* event) {
    if (MediaTree::OnItem == dropIndicatorPosition()) {
        event->ignore();
        return;
    }
    TreeWidget::dropEvent(event);
}
