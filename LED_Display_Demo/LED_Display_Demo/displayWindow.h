#ifndef DISPLAYWINDOW_H
#define DISPLAYWINDOW_H

#include <QWidget>
#include <QColor>
#include <QTimer>

class DisplayWindow : public QWidget {
    Q_OBJECT


public:
    explicit DisplayWindow(QWidget *parent = nullptr);
    void setLockGeometry(bool locked);
    bool isGeometryLocked() const;

    void setEnableDoubleClickFullScreen(bool enable);
    void closeEvent(QCloseEvent *event);
    void setBackgroundColor(const QColor &color);
    void setShowRGB(bool show);
    void setLineStyle(bool hLine, bool vLine, bool dLine);
    void setLineColor(const QColor &color);
    void enableAutoScan(bool enable);
    void contextMenuEvent(QContextMenuEvent *event);
    //QColor getBaseColor() const;
    void setTopMost(bool enable);
protected:
    void mouseDoubleClickEvent(QMouseEvent* event);
    void moveEvent(QMoveEvent* event);
    void resizeEvent(QResizeEvent* event);
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:    
    QAction* hideAction = nullptr;
    bool lockGeometry = false;  // 默认为不锁定
    bool enableDoubleClickFullScreen = false;
    bool isFullScreenMode = false; // 当前是否处于全屏

    bool dragging = false;
    QPoint dragPosition;
    QColor baseColor = Qt::black;
    QColor bgColor = Qt::black;
    QColor lineColor = Qt::white;
    int brightnessValue = 255;
    bool showRgbText = true;
    bool showHLine = false, showVLine = false, showDLine = false;
    QTimer scanTimer;
    int offset = 0;
signals:
    void visibilityChangedExternally(bool isVisible); // 触发给主窗口同步 UI

    void geometryChanged(const QRect& newGeometry);  // 通知主窗口更新
};

#endif // DISPLAYWINDOW_H
