#include "mainwindow.h"
#include "mediapanel.h"
#include "outputpanel.h"
#include "planpanel.h"
#include "progresspanel.h"
#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDateTime>
#include <QApplication>
#include <QSettings>
#include <QToolBar>
#include <QDockWidget>
#include <QScrollArea>
#include <QTimer>

// 主窗口类的构造函数，初始化窗口及其组件
MainWindow::MainWindow() {
    resize(1400, 840);
    setWindowTitle("Compass");

    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
    setCentralWidget(liveEditor = new LiveEditor);

    tab = new QTabWidget;
    tab->setMinimumWidth(360);
    tab->setStyleSheet("QTabWidget::pane{border:none;}");

    auto scroll = new QScrollArea;
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setWidgetResizable(true);
    tab->addTab(scroll, tr("Layer Properties"));

    auto mediaPanel = new MediaPanel;
    tab->addTab(mediaPanel, tr("Media Library"));

    auto outputPanel = new OutputPanel;
    tab->addTab(outputPanel, tr("Output"));

    tab->setCurrentIndex(1);

    auto dockProperties = new QDockWidget;
    dockProperties->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    dockProperties->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dockProperties->setWidget(tab);
    addDockWidget(Qt::RightDockWidgetArea, dockProperties);

    auto dockPlan = new QDockWidget(tr("Plan"));
    dockPlan->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    dockPlan->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    dockPlan->setWidget(new PlanPanel);
    addDockWidget(Qt::BottomDockWidgetArea, dockPlan);

    auto dockProgress = new QDockWidget(tr("Progress"));
    dockProgress->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    dockProgress->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    dockProgress->setWidget(new ProgressPanel);
    addDockWidget(Qt::BottomDockWidgetArea, dockProgress);
    splitDockWidget(dockPlan, dockProgress, Qt::Vertical);

    auto toolBar = new QToolBar;
    toolBar->setFloatable(false);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->setIconSize(QSize(46, 40));

    auto action = new QAction(QIcon(":/res/program/Save.png"), tr("Save"));
    toolBar->addAction(action);
    action = new QAction(QIcon(":/res/program/Setting.png"), tr("Setting"));
    toolBar->addAction(action);
    toolBar->addSeparator();

    show();
    dockProgress->setMaximumHeight(80);

    QTimer::singleShot(68, [=] {
        dockProgress->setMaximumHeight(200);
        });
}
