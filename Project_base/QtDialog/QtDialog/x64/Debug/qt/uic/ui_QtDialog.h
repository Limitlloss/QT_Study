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
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QtDialogClass
{
public:
    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout_4;
    QWidget *widget_4;
    QVBoxLayout *verticalLayout;
    QWidget *widget_2;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer_5;
    QWidget *widget;
    QFormLayout *formLayout;
    QLabel *label;
    QLineEdit *lineEdit_2;
    QLabel *label_2;
    QLineEdit *lineEdit;
    QSpacerItem *horizontalSpacer_4;
    QProgressBar *progressBar;
    QWidget *widget_3;
    QHBoxLayout *horizontalLayout_3;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton;
    QSpacerItem *horizontalSpacer_3;
    QPushButton *pushButton_2;
    QSpacerItem *horizontalSpacer_2;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QCheckBox *checkBox;
    QCheckBox *checkBox_2;
    QCheckBox *checkBox_3;
    QCheckBox *checkBox_4;

    void setupUi(QMainWindow *QtDialogClass)
    {
        if (QtDialogClass->objectName().isEmpty())
            QtDialogClass->setObjectName("QtDialogClass");
        QtDialogClass->resize(492, 443);
        centralWidget = new QWidget(QtDialogClass);
        centralWidget->setObjectName("centralWidget");
        horizontalLayout_4 = new QHBoxLayout(centralWidget);
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_4->setObjectName("horizontalLayout_4");
        widget_4 = new QWidget(centralWidget);
        widget_4->setObjectName("widget_4");
        verticalLayout = new QVBoxLayout(widget_4);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName("verticalLayout");
        widget_2 = new QWidget(widget_4);
        widget_2->setObjectName("widget_2");
        horizontalLayout_2 = new QHBoxLayout(widget_2);
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_5);

        widget = new QWidget(widget_2);
        widget->setObjectName("widget");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(widget->sizePolicy().hasHeightForWidth());
        widget->setSizePolicy(sizePolicy);
        formLayout = new QFormLayout(widget);
        formLayout->setSpacing(6);
        formLayout->setContentsMargins(11, 11, 11, 11);
        formLayout->setObjectName("formLayout");
        label = new QLabel(widget);
        label->setObjectName("label");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, label);

        lineEdit_2 = new QLineEdit(widget);
        lineEdit_2->setObjectName("lineEdit_2");

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, lineEdit_2);

        label_2 = new QLabel(widget);
        label_2->setObjectName("label_2");

        formLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, label_2);

        lineEdit = new QLineEdit(widget);
        lineEdit->setObjectName("lineEdit");
        lineEdit->setEchoMode(QLineEdit::EchoMode::Password);

        formLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, lineEdit);


        horizontalLayout_2->addWidget(widget);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_4);


        verticalLayout->addWidget(widget_2);

        progressBar = new QProgressBar(widget_4);
        progressBar->setObjectName("progressBar");
        progressBar->setValue(24);

        verticalLayout->addWidget(progressBar);

        widget_3 = new QWidget(widget_4);
        widget_3->setObjectName("widget_3");
        horizontalLayout_3 = new QHBoxLayout(widget_3);
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer);

        pushButton = new QPushButton(widget_3);
        pushButton->setObjectName("pushButton");

        horizontalLayout_3->addWidget(pushButton);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_3);

        pushButton_2 = new QPushButton(widget_3);
        pushButton_2->setObjectName("pushButton_2");

        horizontalLayout_3->addWidget(pushButton_2);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_2);


        verticalLayout->addWidget(widget_3);

        groupBox = new QGroupBox(widget_4);
        groupBox->setObjectName("groupBox");
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName("verticalLayout_2");
        checkBox = new QCheckBox(groupBox);
        checkBox->setObjectName("checkBox");

        verticalLayout_2->addWidget(checkBox);

        checkBox_2 = new QCheckBox(groupBox);
        checkBox_2->setObjectName("checkBox_2");

        verticalLayout_2->addWidget(checkBox_2);

        checkBox_3 = new QCheckBox(groupBox);
        checkBox_3->setObjectName("checkBox_3");

        verticalLayout_2->addWidget(checkBox_3);

        checkBox_4 = new QCheckBox(groupBox);
        checkBox_4->setObjectName("checkBox_4");

        verticalLayout_2->addWidget(checkBox_4);


        verticalLayout->addWidget(groupBox);


        horizontalLayout_4->addWidget(widget_4);

        QtDialogClass->setCentralWidget(centralWidget);

        retranslateUi(QtDialogClass);

        QMetaObject::connectSlotsByName(QtDialogClass);
    } // setupUi

    void retranslateUi(QMainWindow *QtDialogClass)
    {
        QtDialogClass->setWindowTitle(QCoreApplication::translate("QtDialogClass", "QtDialog", nullptr));
        label->setText(QCoreApplication::translate("QtDialogClass", "\347\224\250\346\210\267\345\220\215\357\274\232", nullptr));
        label_2->setText(QCoreApplication::translate("QtDialogClass", "\345\257\206   \347\240\201\357\274\232", nullptr));
        pushButton->setText(QCoreApplication::translate("QtDialogClass", "\347\231\273\351\231\206", nullptr));
        pushButton_2->setText(QCoreApplication::translate("QtDialogClass", "\351\200\200\345\207\272", nullptr));
        groupBox->setTitle(QCoreApplication::translate("QtDialogClass", "GroupBox", nullptr));
        checkBox->setText(QCoreApplication::translate("QtDialogClass", "\351\200\211\351\241\2711", nullptr));
        checkBox_2->setText(QCoreApplication::translate("QtDialogClass", "\351\200\211\351\241\2712", nullptr));
        checkBox_3->setText(QCoreApplication::translate("QtDialogClass", "\351\200\211\351\241\2713", nullptr));
        checkBox_4->setText(QCoreApplication::translate("QtDialogClass", "\351\200\211\351\241\2714", nullptr));
    } // retranslateUi

};

namespace Ui {
    class QtDialogClass: public Ui_QtDialogClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTDIALOG_H
