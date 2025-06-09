#ifndef SUBWINDOW_H
#define SUBWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

class SubWindow : public QWidget {
    Q_OBJECT
public:
    explicit SubWindow(int id, QWidget* parent = nullptr);
    ~SubWindow();

private:
    int windowId;
};

#endif // SUBWINDOW_H