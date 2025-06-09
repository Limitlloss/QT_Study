#include "_My_First_Button.h"
#include "QPushButton.h"
#include <QDebug>
#include <QString>  
#include <string>  
_My_First_Button::_My_First_Button(QWidget* parent)
	: QWidget(parent)
{
	QPushButton* btn1 = new QPushButton(this);
	btn1->setText("按钮1");
	//btn1->resize(150,30);
	btn1->move(0, 0);
	QPushButton* btn2 = new QPushButton("关闭", this);
	btn2->move(0,100);
	std::string str = btn1->text().toStdString();


	resize(600,400);
	setWindowTitle("我的第一个窗口");
	setFixedSize(600,400);
	connect(btn2, &QPushButton::clicked, this, [=]() {
		qDebug() << "槽函数被调用！";
		btn1->setText("关闭按钮已按下！"); 
		qDebug() << "按钮一的文本:" << str;
		});



}

_My_First_Button::~_My_First_Button()
{
	qDebug() << "我的Widget析构函数被调用";
}
