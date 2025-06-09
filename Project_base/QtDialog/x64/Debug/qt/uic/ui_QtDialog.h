/********************************************************************************
** Form generated from reading UI file 'QtDialog.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTDIALOG_H
#define UI_QTDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QtDialogClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *QtDialogClass)
    {
        if (QtDialogClass->objectName().isEmpty())
            QtDialogClass->setObjectName("QtDialogClass");
        QtDialogClass->resize(600, 400);
        menuBar = new QMenuBar(QtDialogClass);
        menuBar->setObjectName("menuBar");
        QtDialogClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(QtDialogClass);
        mainToolBar->setObjectName("mainToolBar");
        QtDialogClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(QtDialogClass);
        centralWidget->setObjectName("centralWidget");
        QtDialogClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(QtDialogClass);
        statusBar->setObjectName("statusBar");
        QtDialogClass->setStatusBar(statusBar);

        retranslateUi(QtDialogClass);

        QMetaObject::connectSlotsByName(QtDialogClass);
    } // setupUi

    void retranslateUi(QMainWindow *QtDialogClass)
    {
        QtDialogClass->setWindowTitle(QCoreApplication::translate("QtDialogClass", "QtDialog", nullptr));
    } // retranslateUi

};

namespace Ui {
    class QtDialogClass: public Ui_QtDialogClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTDIALOG_H
