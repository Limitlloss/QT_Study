/********************************************************************************
** Form generated from reading UI file 'UI_Learn.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UI_LEARN_H
#define UI_UI_LEARN_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_UI_LearnClass
{
public:
    QAction *actionNewFile;
    QWidget *centralWidget;
    QMenuBar *menuBar;
    QMenu *menu;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *UI_LearnClass)
    {
        if (UI_LearnClass->objectName().isEmpty())
            UI_LearnClass->setObjectName("UI_LearnClass");
        UI_LearnClass->resize(600, 400);
        actionNewFile = new QAction(UI_LearnClass);
        actionNewFile->setObjectName("actionNewFile");
        centralWidget = new QWidget(UI_LearnClass);
        centralWidget->setObjectName("centralWidget");
        UI_LearnClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(UI_LearnClass);
        menuBar->setObjectName("menuBar");
        menuBar->setGeometry(QRect(0, 0, 600, 21));
        menu = new QMenu(menuBar);
        menu->setObjectName("menu");
        UI_LearnClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(UI_LearnClass);
        mainToolBar->setObjectName("mainToolBar");
        UI_LearnClass->addToolBar(Qt::ToolBarArea::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(UI_LearnClass);
        statusBar->setObjectName("statusBar");
        UI_LearnClass->setStatusBar(statusBar);

        menuBar->addAction(menu->menuAction());
        menu->addAction(actionNewFile);

        retranslateUi(UI_LearnClass);

        QMetaObject::connectSlotsByName(UI_LearnClass);
    } // setupUi

    void retranslateUi(QMainWindow *UI_LearnClass)
    {
        UI_LearnClass->setWindowTitle(QCoreApplication::translate("UI_LearnClass", "UI_Learn", nullptr));
        actionNewFile->setText(QCoreApplication::translate("UI_LearnClass", "\346\226\260\345\273\272", nullptr));
        menu->setTitle(QCoreApplication::translate("UI_LearnClass", "\346\226\207\344\273\266", nullptr));
    } // retranslateUi

};

namespace Ui {
    class UI_LearnClass: public Ui_UI_LearnClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_UI_LEARN_H
