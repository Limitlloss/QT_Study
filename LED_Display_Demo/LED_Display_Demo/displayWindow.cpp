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

void DisplayWindow::setBackgroundColor(const QColor& color) {
	bgColor = color;
	update();
}

void DisplayWindow::mousePressEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton) {
		dragging = true;
		dragPosition = event->globalPos() - frameGeometry().topLeft();
		event->accept();
	}
}

void DisplayWindow::mouseMoveEvent(QMouseEvent* event) {
	if (dragging && (event->buttons() & Qt::LeftButton)) {
		move(event->globalPos() - dragPosition);
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

	QAction* closeAction = menu.addAction("关闭");
	QAction* result = menu.exec(event->globalPos());

	if (result == closeAction) {
		// 关闭本窗口
		this->close();

		// 如果 MainWindow 也需要关闭
		QWidgetList topLevels = QApplication::topLevelWidgets();
		for (QWidget* w : topLevels) {
			if (auto* mainWin = qobject_cast<QMainWindow*>(w)) {
				mainWin->close();
			}
		}
	}
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

void DisplayWindow::enableAutoScan(bool enable) {
	if (enable) scanTimer.start(50);
	else scanTimer.stop();
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
