#include "progresspanel.h"
#include "mediapanel.h"
#include "main.h"
#include "layer.h"
#include <QDropEvent>
#include <QMimeData>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QGraphicsVideoItem>
#include <QMessageBox>
#include <QScrollBar>
#include <QButtonGroup>
#include <QPainter>

// 进度面板类，用于显示媒体播放进度和控制按钮
ProgressPanel::ProgressPanel(QWidget* parent) : QWidget{ parent } {
    auto hBox = new HBox(this);
    hBox->setContentsMargins(0, 0, 0, 0);
    hBox->setSpacing(0);
    auto btnPlay = new QPushButton("||>");
    btnPlay->setMaximumWidth(100);
    btnPlay->setFixedHeight(60);
    connect(btnPlay, &QPushButton::clicked, this, &ProgressPanel::play);
    hBox->addWidget(btnPlay);

    auto scrollArea = new QScrollArea;
    hBox->addWidget(scrollArea);
    vBox = new VBox(scrollArea);
    vBox->setContentsMargins(0, 0, 0, 0);
    vBox->setSpacing(0);
    //connect(gTable->verticalScrollBar(), &QScrollBar::valueChanged, gTableV->verticalScrollBar(), &QScrollBar::setValue);
}

// 添加新的行到进度面板，每行包含一个播放按钮和一个进度滑块
void ProgressPanel::addRow() {
    auto hBox = new HBox(vBox);
    auto btnPlay = new QPushButton("||>");
    connect(btnPlay, &QPushButton::clicked, this, [=] {

        });
    hBox->addWidget(btnPlay);

    auto slider = new QSlider(Qt::Horizontal);
    slider->setRange(0, 100);

    auto lb = hBox->addLabel();
    connect(slider, &QSlider::valueChanged, lb, [lb](int value) {
        lb->setText(QTime::fromMSecsSinceStartOfDay(value).toString("HH:mm:ss.zzz"));
        });
    lb = hBox->addLabel();
}

// 播放按钮的槽函数，用于处理播放逻辑
void ProgressPanel::play() {

}
