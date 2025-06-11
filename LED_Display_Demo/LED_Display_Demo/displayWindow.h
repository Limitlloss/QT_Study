#ifndef DISPLAYWINDOW_H
#define DISPLAYWINDOW_H

#include <QWidget>
#include <QColor>
#include <QTimer>

class DisplayWindow : public QWidget {
    Q_OBJECT


public:
    explicit DisplayWindow(QWidget *parent = nullptr);


    void closeEvent(QCloseEvent *event);
    void setBackgroundColor(const QColor &color);
    void setShowRGB(bool show);
    void setLineStyle(bool hLine, bool vLine, bool dLine);
    void setLineColor(const QColor &color);
    void enableAutoScan(bool enable);
    void contextMenuEvent(QContextMenuEvent *event);
    QColor getBaseColor() const;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
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
};

#endif // DISPLAYWINDOW_H
