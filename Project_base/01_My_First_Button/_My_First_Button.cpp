#include "_My_First_Button.h"
#include "QPushButton.h"
_My_First_Button::_My_First_Button(QWidget* parent)
	: QWidget(parent)
{
	QPushButton* btn = new QPushButton(parent);
	btn->setText("第一个按钮");
	btn->show();
	QPushButton* btn2 = new QPushButton("Button_2", this);
	btn2->move(100, 100);
	resize(1080, 200);
	//ui.setupUi(this);
}

_My_First_Button::~_My_First_Button()
{
}
