#ifndef DISPLAYWINDOW_H
#define DISPLAYWINDOW_H

#include <QWidget>
#include <QColor>
#include <QTimer>

class DisplayWindow : public QWidget {
    Q_OBJECT

public:
    explicit DisplayWindow(QWidget* parent = nullptr);

    void setLockGeometry(bool locked);
    void setEnableDoubleClickFullScreen(bool enable);
    void setTopMost(bool enable);
    void setBackgroundColor(const QColor& color);
    void setShowRGB(bool show);
    void setLineStyle(bool hLine, bool vLine, bool dLine);
    void setLineColor(const QColor& color);
    void enableAutoScan(bool enable);

signals:
    void visibilityChangedExternally(bool isVisible);
    void geometryChanged(const QRect& newGeometry);

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void moveEvent(QMoveEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    bool lockGeometry = false;
    bool enableDoubleClickFullScreen = false;
    bool isFullScreenMode = false;

    bool dragging = false;
    QPoint dragPosition;
    QColor bgColor = Qt::black;
    QColor lineColor = Qt::white;
    bool showRgbText = true;
    bool showHLine = false, showVLine = false, showDLine = false;

    QTimer scanTimer;
    int offset = 0;

    QAction* hideAction = nullptr;
};

#endif // DISPLAYWINDOW_H
