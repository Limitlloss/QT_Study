#ifndef QWAITINGDLG_H
#define QWAITINGDLG_H

#include <QLabel>
#include <QNetworkReply>
#include <QDialog>

class WaitingIndicator : public QWidget {
    Q_OBJECT
public:
    using QWidget::QWidget;
    QColor mColor{0x0088ff};
public slots:
    void success();
protected:
    void timerEvent(QTimerEvent * event) override;
    void paintEvent(QPaintEvent * event) override;

    int angle{0};
    int timerId{0};
};

class WaitingDlg : public QDialog {
    Q_OBJECT
public:
    explicit WaitingDlg(QWidget *parent = nullptr, QString text = 0, QString sucText = 0);

    QLabel *fdText;
    QString sucText;
    WaitingIndicator *mIndicator;
    int keepTime = 750;
public slots:
    void show();
    void showLater();
    void success();
protected:
    void timerEvent(QTimerEvent *) override;
    void closeEvent(QCloseEvent *) override;
private:
    int closeTimerId{0}, showTimerId{0};
};

#endif // QWAITINGDLG_H
