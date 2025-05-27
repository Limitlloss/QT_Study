#include "my_widget.h"
#include <QPushButton>
#include <QVBoxLayout>	
class MyWidget :public QWidget {
	Q_OBJECT
public:
	MyWidget(QWidget* parent = 0);
	~MyWidget();
};