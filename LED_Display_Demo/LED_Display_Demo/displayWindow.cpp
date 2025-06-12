#include "DisplayWindow.h"
#include <QPainter>
#include <QMenu>
#include <QAction>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QApplication>
#include <QMainWindow>

DisplayWindow::DisplayWindow(QWidget* parent) : QWidget(parent) {
    setWindowTitle("显示窗口");
    resize(600, 400);
    setMinimumSize(200, 150);
    setMouseTracking(true);
    show();

    connect(&scanTimer, &QTimer::timeout, this, [this]() {
        offset += 5;
        if (offset > width()) offset = 0;
        update();
        });
}
void DisplayWindow::setCurrentColor(QColor& color) {
    currentColor = color;
    update();
}
void DisplayWindow::setLinePattern(bool h, bool v, bool d1, bool d2) {
    lineRenderer.setLineOptions(h, v, d1, d2);
    update(); // 触发重绘
}

void DisplayWindow::setLockGeometry(bool locked) {
    lockGeometry = locked;
}

void DisplayWindow::setEnableDoubleClickFullScreen(bool enable) {
    enableDoubleClickFullScreen = enable;
}

void DisplayWindow::setTopMost(bool enable) {
    setWindowFlag(Qt::WindowStaysOnTopHint, enable);
    show();  // Apply window flag
}

void DisplayWindow::setBackgroundColor(const QColor& color) {
    bgColor = color;
    update();
}

void DisplayWindow::setShowRGB(bool show) {
    showRgbText = show;
    update();
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
    if (enable) {
        scanTimer.start(50);
    }
    else {
        scanTimer.stop();
    }
}

void DisplayWindow::mouseDoubleClickEvent(QMouseEvent* event) {
    if (!enableDoubleClickFullScreen || lockGeometry) return;
    isFullScreenMode ? showNormal() : showFullScreen();
    isFullScreenMode = !isFullScreenMode;
    QWidget::mouseDoubleClickEvent(event);
}

void DisplayWindow::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        dragging = true;
        dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void DisplayWindow::mouseMoveEvent(QMouseEvent* event) {
    if (!lockGeometry && dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPos() - dragPosition);
        emit geometryChanged(geometry());
        event->accept();
    }
}

void DisplayWindow::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        dragging = false;
        event->accept();
    }
}

void DisplayWindow::moveEvent(QMoveEvent* event) {
    emit geometryChanged(geometry());
    QWidget::moveEvent(event);
}

void DisplayWindow::resizeEvent(QResizeEvent* event) {
    emit geometryChanged(geometry());
    QWidget::resizeEvent(event);
}

void DisplayWindow::closeEvent(QCloseEvent* event) {
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
            setVisible(!checked);
            emit visibilityChangedExternally(!checked);
            });
    }

    hideAction->setChecked(!isVisible());
    menu.addAction(hideAction);
    menu.addSeparator();
    menu.addAction("关闭", this, &DisplayWindow::close);
    menu.exec(event->globalPos());
}
void DisplayWindow::paintEvent(QPaintEvent* event) {
    QPainter painter(this);

    // 正确选择背景色
    QColor colorToUse = useCurrentColor ? currentColor : bgColor;
    painter.fillRect(rect(), colorToUse);

    // 显示 RGB 数值
    if (showRgbText) {
        QString text = QString("R=%1 G=%2 B=%3")
            .arg(colorToUse.red())
            .arg(colorToUse.green())
            .arg(colorToUse.blue());

        int brightness = (colorToUse.red() * 299 + colorToUse.green() * 587 + colorToUse.blue() * 114) / 1000;
        QColor textColor = (brightness > 128) ? Qt::black : Qt::white;

        painter.setPen(textColor);
        painter.drawText(rect(), Qt::AlignCenter, text);
    }

    // 画线
    painter.setPen(lineColor);
    if (showHLine)
        painter.drawLine(0, height() / 2 + offset % height(), width(), height() / 2 + offset % height());
    if (showVLine)
        painter.drawLine(width() / 2 + offset % width(), 0, width() / 2 + offset % width(), height());
    if (showDLine)
        painter.drawLine(offset % width(), 0, 0, offset % height());

    // 如果有扩展线条类（如 agingFeature/lineRenderer）
    if (lineRendererEnabled)
        lineRenderer.renderLines(painter, size());
}

