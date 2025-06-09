#include "SubWindow.h"
#include <QDebug>

SubWindow::SubWindow(int id, QWidget* parent)
    : QWidget(parent), windowId(id)
{
    setAttribute(Qt::WA_DeleteOnClose);  // 关键设置[1](@ref)

    QLabel* label = new QLabel(QString("窗口 #%1").arg(id), this);
    label->setAlignment(Qt::AlignCenter);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(label);

    qDebug() << "窗口" << id << "已创建";
    setWindowTitle(QString("子窗口 #%1").arg(id));
}

SubWindow::~SubWindow() {
    qDebug() << "窗口" << windowId << "已安全销毁";
}