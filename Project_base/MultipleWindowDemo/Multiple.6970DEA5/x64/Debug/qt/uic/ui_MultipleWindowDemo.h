/********************************************************************************
** Form generated from reading UI file 'MultipleWindowDemo.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MULTIPLEWINDOWDEMO_H
#define UI_MULTIPLEWINDOWDEMO_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MultipleWindowDemoClass
{
public:

    void setupUi(QWidget *MultipleWindowDemoClass)
    {
        if (MultipleWindowDemoClass->objectName().isEmpty())
            MultipleWindowDemoClass->setObjectName("MultipleWindowDemoClass");
        MultipleWindowDemoClass->resize(600, 400);

        retranslateUi(MultipleWindowDemoClass);

        QMetaObject::connectSlotsByName(MultipleWindowDemoClass);
    } // setupUi

    void retranslateUi(QWidget *MultipleWindowDemoClass)
    {
        MultipleWindowDemoClass->setWindowTitle(QCoreApplication::translate("MultipleWindowDemoClass", "MultipleWindowDemo", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MultipleWindowDemoClass: public Ui_MultipleWindowDemoClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MULTIPLEWINDOWDEMO_H
