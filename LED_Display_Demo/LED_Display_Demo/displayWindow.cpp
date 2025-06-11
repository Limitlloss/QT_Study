#include "DisplayWindow.h"
#include <QPainter>
#include <QPaintEvent>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QApplication>
#include <QMainWindow>
DisplayWindow::DisplayWindow(QWidget* parent) : QWidget(parent) {
	setWindowTitle("显示窗口");
	resize(600, 400);
	setMinimumSize(200, 150); // 可选
	setMouseTracking(true);
	this->show();
	connect(&scanTimer, &QTimer::timeout, this, [this]() {
		offset += 5;
		if (offset > width()) offset = 0;
		update();
		});
}
void DisplayWindow::setLockGeometry(bool locked) {
	lockGeometry = locked;
}

bool DisplayWindow::isGeometryLocked() const {
	return lockGeometry;
}

// 修改 mouseMoveEvent
void DisplayWindow::mouseMoveEvent(QMouseEvent* event) {
	if (!lockGeometry && dragging && (event->buttons() & Qt::LeftButton)) {
		move(event->globalPos() - dragPosition);
		emit geometryChanged(geometry());
		event->accept();
	}
}

// 可选：如果你允许窗口拉伸改变大小，也需要重写 resizeEvent、mousePressEvent 等加判断

void DisplayWindow::setTopMost(bool enable) {
	setWindowFlag(Qt::WindowStaysOnTopHint, enable);
	show();  // 必须调用 show() 使新 flag 生效
}
void DisplayWindow::setEnableDoubleClickFullScreen(bool enable) {
	enableDoubleClickFullScreen = enable;
}

void DisplayWindow::mouseDoubleClickEvent(QMouseEvent* event) {
	if (!enableDoubleClickFullScreen) return;
	if (lockGeometry) return;  // 锁定时不允许切换
	if (isFullScreenMode) {
		showNormal();
		isFullScreenMode = false;
	}
	else {
		showFullScreen();
		isFullScreenMode = true;
	}
	QWidget::mouseDoubleClickEvent(event);
}

void DisplayWindow::setBackgroundColor(const QColor& color) {
	bgColor = color;
	update();
}

//QColor DisplayWindow::getBaseColor() const {
//	return baseColor;
//}

void DisplayWindow::mousePressEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton) {
		dragging = true;
		dragPosition = event->globalPos() - frameGeometry().topLeft();
		event->accept();
	}
}



void DisplayWindow::mouseReleaseEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton) {
		dragging = false;
		event->accept();
	}
}
void DisplayWindow::setShowRGB(bool show) {
	showRgbText = show;
	update();
}
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QApplication>
#include <QMainWindow>

void DisplayWindow::closeEvent(QCloseEvent* event) {
	QWidget* maybeMain = parentWidget();
	QWidgetList topLevels = QApplication::topLevelWidgets();
	for (QWidget* w : topLevels) {
		if (auto* mainWin = qobject_cast<QMainWindow*>(w)) {
			mainWin->close();
		}
	}
	QWidget::closeEvent(event);
}
void DisplayWindow::contextMenuEvent(QContextMenuEvent* event) {
	QMenu menu(this);

	if (!hideAction) {
		hideAction = new QAction("隐藏显示窗口", this);
		hideAction->setCheckable(true);
		connect(hideAction, &QAction::triggered, this, [=](bool checked) {
			this->setVisible(!checked);
			emit visibilityChangedExternally(!checked);
			});
	}

	// 每次打开菜单前更新状态
	hideAction->setChecked(!this->isVisible());

	menu.addAction(hideAction);
	menu.addSeparator();
	menu.addAction("关闭", this, &DisplayWindow::close);

	menu.exec(event->globalPos());
}




void DisplayWindow::setLineStyle(bool h, bool v, bool d) {
	showHLine = h;
	showVLine = v;
	showDLine = d;
	update();
}

void DisplayWindow::setLineColor(const QColor& color) {
	lineColor = color;
	update();
}
void DisplayWindow::moveEvent(QMoveEvent* event) {
	emit geometryChanged(this->geometry());
	QWidget::moveEvent(event);
}
void DisplayWindow::enableAutoScan(bool enable) {
	if (enable) scanTimer.start(50);
	else scanTimer.stop();
}
void DisplayWindow::resizeEvent(QResizeEvent* event) {
	emit geometryChanged(this->geometry());
	QWidget::resizeEvent(event);
}
void DisplayWindow::paintEvent(QPaintEvent*) {
	QPainter painter(this);
	painter.fillRect(rect(), bgColor);

	if (showRgbText) {
		QString text = QString("R=%1 G=%2 B=%3")
			.arg(bgColor.red())
			.arg(bgColor.green())
			.arg(bgColor.blue());

		int brightness = (bgColor.red() * 299 + bgColor.green() * 587 + bgColor.blue() * 114) / 1000;
		QColor textColor = (brightness > 128) ? Qt::black : Qt::white;

		painter.setPen(textColor);
		painter.drawText(rect(), Qt::AlignCenter, text);
	}


	painter.setPen(lineColor);
	if (showHLine)
		painter.drawLine(0, height() / 2 + offset % height(), width(), height() / 2 + offset % height());
	if (showVLine)
		painter.drawLine(width() / 2 + offset % width(), 0, width() / 2 + offset % width(), height());
	if (showDLine)
		painter.drawLine(offset % width(), 0, 0, offset % height());
}
