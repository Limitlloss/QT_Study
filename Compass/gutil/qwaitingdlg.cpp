#include "qwaitingdlg.h"
#include "qgui.h"
#include <QTimerEvent>
#include <QPainter>
#include <QPainterPath>

WaitingDlg::WaitingDlg(QWidget *parent, QString text, QString sucText) : QDialog{parent, Qt::Tool}, sucText(sucText) {
    setAttribute(Qt::WA_DeleteOnClose);
    setModal(true);

    auto vBox = new VBox(this);

    mIndicator = new WaitingIndicator(this);
    mIndicator->setFixedSize(100, 100);
    vBox->addWidget(mIndicator, 0, Qt::AlignCenter);

    fdText = new QLabel(text);
    fdText->setAlignment(Qt::AlignCenter);
    gFont(fdText, 18, true);
    vBox->addWidget(fdText);
}

void WaitingDlg::closeEvent(QCloseEvent *event) {
    if(showTimerId) {
        killTimer(showTimerId);
        showTimerId = 0;
    }
    if(closeTimerId) {
        killTimer(closeTimerId);
        closeTimerId = 0;
    }
    QDialog::closeEvent(event);
}
void WaitingDlg::timerEvent(QTimerEvent *event) {
    if(showTimerId==event->timerId()) {
        killTimer(showTimerId);
        showTimerId = 0;
        show();
    } else if(closeTimerId==event->timerId()) {
        killTimer(closeTimerId);
        closeTimerId = 0;
        close();
    } else QDialog::timerEvent(event);
}
void WaitingDlg::show() {
    QDialog::show();
    raise();
    activateWindow();
}
void WaitingDlg::showLater() {
    if(isVisible()) return;
    if(showTimerId) killTimer(showTimerId);
    showTimerId = startTimer(200);
}
void WaitingDlg::success() {
    fdText->setText(sucText.isEmpty() ? tr("Success") : sucText);
    mIndicator->success();
    if(showTimerId) {
        killTimer(showTimerId);
        showTimerId = 0;
    }
    if(! isVisible()) show();
    if(closeTimerId) killTimer(closeTimerId);
    closeTimerId = startTimer(keepTime);
}


void WaitingIndicator::success() {
    if(timerId > 0) killTimer(timerId);
    timerId = -1;
    angle = 0;
    update();
}

void WaitingIndicator::timerEvent(QTimerEvent *event) {
    if(timerId!=event->timerId()) QWidget::timerEvent(event);
    else if(isVisible()) {
        angle += 30;
        if(angle>=360) angle -= 360;
        update();
    } else if(timerId > 0) {
        killTimer(timerId);
        timerId = 0;
        angle = 0;
    }
}

void WaitingIndicator::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if(timerId > 0) {
        int radius = qMin(width(), height()) * 0.33;
        int innerRadius = radius >> 1;
        QPen pen(mColor, radius / 6, Qt::SolidLine, Qt::RoundCap);
        painter.translate(width()>>1, height()>>1);
        auto color = mColor;
        for(int i=0; i<12; ++i) {
            if(i) {
                color.setAlphaF(1 - i/12.0);
                pen.setColor(color);
                painter.rotate(-30);
            } else if(angle) painter.rotate(angle);
            painter.setPen(pen);
            painter.drawLine(0, innerRadius, 0, radius);
        }
    } else if(timerId==0) timerId = startTimer(33);
    else {
        int radius = qMin(width(), height()) >> 1;
        int lineWidth = radius / 8;
        radius -= lineWidth>>1;
        QPen pen(QColor(0x00aa00), lineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter.setPen(pen);
        painter.translate(width()>>1, height()>>1);
        painter.drawEllipse(QPoint(0, 0), radius, radius);
        QPainterPath path({-0.8*radius, 0});
        path.lineTo(-0.25*radius, 0.6*radius);
        path.lineTo(0.6*radius, -0.5*radius);
        painter.drawPath(path);
    }
}
