/********************************************************************************
** Form generated from reading UI file '_My_First_Button.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI__MY_FIRST_BUTTON_H
#define UI__MY_FIRST_BUTTON_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui__My_First_ButtonClass
{
public:

    void setupUi(QWidget *_My_First_ButtonClass)
    {
        if (_My_First_ButtonClass->objectName().isEmpty())
            _My_First_ButtonClass->setObjectName("_My_First_ButtonClass");
        _My_First_ButtonClass->resize(600, 400);

        retranslateUi(_My_First_ButtonClass);

        QMetaObject::connectSlotsByName(_My_First_ButtonClass);
    } // setupUi

    void retranslateUi(QWidget *_My_First_ButtonClass)
    {
        _My_First_ButtonClass->setWindowTitle(QCoreApplication::translate("_My_First_ButtonClass", "_My_First_Button", nullptr));
    } // retranslateUi

};

namespace Ui {
    class _My_First_ButtonClass: public Ui__My_First_ButtonClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI__MY_FIRST_BUTTON_H
