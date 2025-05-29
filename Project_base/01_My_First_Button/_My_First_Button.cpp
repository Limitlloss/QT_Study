#include "_My_First_Button.h"
#include "QPushButton.h"
#include <QDebug>
_My_First_Button::_My_First_Button(QWidget* parent)
	: QWidget(parent)
{
	QPushButton* btn1 = new QPushButton(this);
	btn1->setText("按钮1");
	//btn1->resize(150,30);
	btn1->move(0, 0);
	QPushButton* btn2 = new QPushButton("按钮2", this);
	btn2->move(0,100);
	resize(600,400);
	setWindowTitle("我的第一个窗口");
	setFixedSize(600,400);
}

_My_First_Button::~_My_First_Button()
{
	qDebug() << "我的Widget析构函数被调用";
}
