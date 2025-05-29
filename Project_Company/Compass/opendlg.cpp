#include "opendlg.h"
#include "main.h"
#include "gutil/qgui.h"
#include "QPushButton"
#include "QFileDialog"

OpenDlg::OpenDlg(QWidget *parent) : QDialog{parent} {
#ifdef Q_OS_WIN
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
#endif
    resize(500, 400);

    auto vBox = new VBox(this);

    auto hBox = new HBox(vBox);
    auto listPlan = new QListWidget;
    hBox->addWidget(listPlan);

    auto vv = new VBox(hBox);
    auto btnNew = new QPushButton("新建");
    connect(btnNew, &QPushButton::clicked, this, [=] {
        auto file = QFileDialog::getSaveFileName(this, "选择保存目录", gFileHome+"/aaa.cps", "File (*.cps)");
        if(file.isEmpty()) return;
        gFileHome = QFileInfo(file).absolutePath();
        gProgFile = file;
        accept();
    });
    vv->addWidget(btnNew);

    auto btnOpen = new QPushButton("打开");
    connect(btnOpen, &QPushButton::clicked, this, [=] {
        auto file = QFileDialog::getOpenFileName(this, "选择文件", gFileHome, "File (*.cps)");
        if(file.isEmpty()) return;
        gFileHome = QFileInfo(file).absolutePath();
        gProgFile = file;
        accept();
    });
    vv->addWidget(btnOpen);

    auto btnExit = new QPushButton("退出");
    connect(btnExit, &QPushButton::clicked, this, &QDialog::reject);
    vv->addWidget(btnExit);
}

RecentListItem::RecentListItem(const JObj &attr, const QString &pageDir) {

}
