#include "layer.h"
#include "main.h"
#include "planpanel.h"
#include <QDirIterator>
#include <QLabel>
#include <QPainter>
#include <QSpinBox>
#include <QtMath>
#include <QMouseEvent>
#include <QGraphicsVideoItem>
#include <QOpenGLWidget>

// Layer类的绘制事件处理函数，负责绘制图层及其选中状态下的调整句柄
void Layer::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    if (gEditView->selected == this) {
        painter.setPen(item ? Qt::cyan : Qt::green);
        painter.drawRect(0, 0, width(), height());
        painter.drawRect(hdlLT);
        painter.drawRect(hdlT);
        painter.drawRect(hdlRT);
        painter.drawRect(hdlL);
        painter.drawRect(hdlR);
        painter.drawRect(hdlLB);
        painter.drawRect(hdlB);
        painter.drawRect(hdlRB);
    }
    else {
        painter.setPen(item ? Qt::darkCyan : Qt::darkGreen);
        painter.drawRect(0, 0, width(), height());
    }

    QRectF rect(8, 8, width() - 16, height() - 16);
    if (item) painter.drawText(rect, QString("%1\n%2×%3").arg(name).arg(sSize.width()).arg(sSize.height()), QTextOption(Qt::AlignCenter));
    else {
        painter.drawText(rect, QString("%1 %2").arg(name).arg(idx), QTextOption(Qt::AlignLeft | Qt::AlignTop));
        painter.drawText(rect, QString("%1×%2").arg(sSize.width()).arg(sSize.height()), QTextOption(Qt::AlignRight | Qt::AlignBottom));
    }

    //吸附 high light
    if (gEditView->selected != this)painter.setPen(item ? Qt::cyan : Qt::green);
    if (snapLR == 1) painter.drawLine(0, 0, 0, height());
    else if (snapLR == 2) painter.drawLine(width(), 0, width(), height());
    if (snapTB == 1) painter.drawLine(0, 0, width(), 0);
    else if (snapTB == 2) painter.drawLine(0, height(), width(), height());
}

// Layer类的鼠标按下事件处理函数，用于图层的选择和调整准备
void Layer::mousePressEvent(QMouseEvent* e) {
    QWidget::mousePressEvent(e);
    if (e->button() != Qt::LeftButton) return;
    if (item) {
        if (!gOutPanel->isVisible()) return;
        if (gEditView->selected != this) {
            gOutPanel->enCurChanged = false;
            gOutPanel->tree->setCurrentItem(item);
            gOutPanel->enCurChanged = true;
        }
    }
    else if (gOutPanel->isVisible()) return;
    gEditView->select(this);
    setFrmSec(e->pos());
    auto mousePos = e->globalPosition();
    auto elePos = pos();
    if (mFrmSec == Qt::TitleBarArea || mFrmSec == Qt::TopSection || mFrmSec == Qt::LeftSection || mFrmSec == Qt::TopLeftSection) mPressRel = elePos - mousePos;
    else if (mFrmSec == Qt::BottomRightSection) mPressRel = QPointF(width() - mousePos.x(), height() - mousePos.y());
    else if (mFrmSec == Qt::RightSection) mPressRel = QPointF(width() - mousePos.x(), height());
    else if (mFrmSec == Qt::BottomSection) mPressRel = QPointF(width(), height() - mousePos.y());
    else if (mFrmSec == Qt::TopRightSection) mPressRel = QPointF(elePos.x() + width() - mousePos.x(), elePos.y() - mousePos.y());
    else if (mFrmSec == Qt::BottomLeftSection) mPressRel = QPointF(elePos.x() - mousePos.x(), elePos.y() + height() - mousePos.y());
    else if (mFrmSec == Qt::NoSection) mPressRel.setX(FLT_MAX);

    rowIdx = 0;
    for (; rowIdx < gTableV->rowCount(); ++rowIdx) if ((Layer*)gTableV->data(rowIdx, 0).toULongLong() == this) break;
}

// Layer类的鼠标释放事件处理函数，用于结束图层的调整
void Layer::mouseReleaseEvent(QMouseEvent* event) {
    QWidget::mouseReleaseEvent(event);
    if (Qt::LeftButton == event->button()) {
        mPressRel.setX(FLT_MAX);
        for (auto layer : gEditView->layers) layer->clearSnap();
    }
}
#define SnapSpace 6
// Layer类的鼠标移动事件处理函数，用于图层的拖动和调整大小
void Layer::mouseMoveEvent(QMouseEvent* e) {
    if (!(e->buttons() & Qt::LeftButton)) {
        setFrmSec(e->pos());
        return;
    }
    if (gEditView->selected != this) return;
    if (mFrmSec == Qt::NoSection || mPressRel.x() >= FLT_MAX) return;
    auto mousePos = e->globalPosition();
    auto dstHor = mPressRel.x() + mousePos.x();
    auto dstVer = mPressRel.y() + mousePos.y();
    bool posChanged = false, sizeChanged = false;
    for (auto layer : gEditView->layers) layer->clearSnap();
    if (mFrmSec == Qt::TitleBarArea) {
        if (snapLR == 0) for (auto layer : gEditView->layers) if (layer != this) {//左右
            if (fabs(dstHor - layer->x()) < SnapSpace) {
                dstHor = layer->x();
                snapLR = 1;
                layer->snapLR = 1;
                layer->update();
                break;
            }
            auto eleRight = layer->x() + layer->width();
            if (fabs(dstHor - eleRight) < SnapSpace) {
                dstHor = eleRight;
                snapLR = 1;
                layer->snapLR = 2;
                layer->update();
                break;
            }
            auto right = dstHor + width();
            if (fabs(right - layer->x()) < SnapSpace && layer->x() - width() >= 0) {
                dstHor = layer->x() - width();
                snapLR = 2;
                layer->snapLR = 1;
                layer->update();
                break;
            }
            if (fabs(right - eleRight) < SnapSpace && eleRight - width() >= 0) {
                dstHor = eleRight - width();
                snapLR = 2;
                layer->snapLR = 2;
                layer->update();
                break;
            }
        }
        if (snapTB == 0) for (auto layer : gEditView->layers) if (layer != this) {//上下
            if (fabs(dstVer - layer->y()) < SnapSpace) {
                dstVer = layer->y();
                snapTB = 1;
                layer->snapTB = 1;
                layer->update();
                break;
            }
            auto eleBtm = layer->y() + layer->height();
            if (fabs(dstVer - eleBtm) < SnapSpace) {
                dstVer = eleBtm;
                snapTB = 1;
                layer->snapTB = 2;
                layer->update();
                break;
            }
            auto btm = dstVer + height();
            if (fabs(btm - layer->y()) < SnapSpace && layer->y() - height() >= 0) {
                dstVer = layer->y() - height();
                snapTB = 2;
                layer->snapTB = 1;
                layer->update();
                break;
            }
            if (fabs(btm - eleBtm) < SnapSpace && eleBtm - height() >= 0) {
                dstVer = eleBtm - height();
                snapTB = 2;
                layer->snapTB = 2;
                layer->update();
                break;
            }
        }
        move(dstHor, dstVer);
        posChanged = true;
    }
    else if (mFrmSec == Qt::BottomRightSection) {
        if (dstHor < HandleSize) dstHor = HandleSize;
        if (dstVer < HandleSize) dstVer = HandleSize;
        resize(dstHor, dstVer);
        sizeChanged = true;
    }
    else if (mFrmSec == Qt::RightSection) {
        if (dstHor < HandleSize) dstHor = HandleSize;
        auto right = x() + dstHor;
        for (Layer* layer : gEditView->layers) if (layer != this) {//左右
            if (fabs(right - layer->x()) < SnapSpace) {
                dstHor = layer->x() - x();
                snapLR = 2;
                layer->snapLR = 1;
                layer->update();
                break;
            }
            auto eleRight = layer->x() + layer->width();
            if (fabs(right - eleRight) < SnapSpace) {
                dstHor = eleRight - x();
                snapLR = 2;
                layer->snapLR = 2;
                layer->update();
                break;
            }
        }
        resize(dstHor, mPressRel.y());
        sizeChanged = true;
    }
    else if (mFrmSec == Qt::BottomSection) {
        if (dstVer < HandleSize) dstVer = HandleSize;
        auto btm = y() + dstVer;
        for (Layer* layer : gEditView->layers) if (layer != this) {//上下
            auto eleBtm = layer->y() + layer->height();
            if (fabs(btm - layer->y()) < SnapSpace) {
                dstVer = layer->y() - y();
                snapTB = 2;
                layer->snapTB = 1;
                layer->update();
                break;
            }
            if (fabs(btm - eleBtm) < SnapSpace) {
                dstVer = eleBtm - y();
                snapTB = 2;
                layer->snapTB = 2;
                layer->update();
                break;
            }
        }
        resize(mPressRel.rx(), dstVer);
        sizeChanged = true;
    }
    else {
        QRectF geo(x(), y(), width(), height());
        if (mFrmSec == Qt::LeftSection) {
            dstHor = qMin(dstHor, geo.right() - HandleSize);
            if (dstHor > 8) for (auto layer : gEditView->layers) if (layer != this) {//左右
                if (fabs(dstHor - layer->x()) < SnapSpace) {
                    dstHor = layer->x();
                    snapLR = 1;
                    layer->snapLR = 1;
                    layer->update();
                    break;
                }
                auto eleRight = layer->x() + layer->width();
                if (fabs(dstHor - eleRight) < SnapSpace) {
                    dstHor = eleRight;
                    snapLR = 1;
                    layer->snapLR = 2;
                    layer->update();
                    break;
                }
            }
            geo.setLeft(dstHor);
        }
        else if (mFrmSec == Qt::TopSection) {
            dstVer = qMin(dstVer, geo.bottom() - HandleSize);
            if (dstVer > 8) for (Layer* layer : gEditView->layers) if (layer != this) {//上下
                if (fabs(dstVer - layer->y()) < SnapSpace) {
                    dstVer = layer->y();
                    snapTB = 1;
                    layer->snapTB = 1;
                    layer->update();
                    break;
                }
                auto eleBtm = layer->y() + layer->height();
                if (fabs(dstVer - eleBtm) < SnapSpace) {
                    dstVer = eleBtm;
                    snapTB = 1;
                    layer->snapTB = 2;
                    layer->update();
                    break;
                }
            }
            geo.setTop(dstVer);
        }
        else if (mFrmSec == Qt::TopLeftSection) {
            dstHor = qMin(dstHor, geo.right() - HandleSize);
            dstVer = qMin(dstVer, geo.bottom() - HandleSize);
            geo.setLeft(dstHor);
            geo.setTop(dstVer);
        }
        else if (mFrmSec == Qt::TopRightSection) {
            dstHor = qMax(dstHor, geo.x() + HandleSize);
            dstVer = qMin(dstVer, geo.bottom() - HandleSize);
            geo.setRight(dstHor);
            geo.setTop(dstVer);
        }
        else if (mFrmSec == Qt::BottomLeftSection) {
            dstHor = qMin(dstHor, geo.right() - HandleSize);
            dstVer = qMax(dstVer, geo.y() + HandleSize);
            geo.setLeft(dstHor);
            geo.setBottom(dstVer);
        }
        setGeometry(geo.toRect());
        posChanged = true;
        sizeChanged = true;
    }
    if (posChanged) sPos = (pos() - gOrigin) / gScale;
    if (sizeChanged) sSize = size() / gScale;
    if (item) {
        view->setSceneRect({ sPos, sSize });
        if (posChanged) {
            gOutPanel->edX->blockSignals(true);
            gOutPanel->edY->blockSignals(true);
            gOutPanel->edX->setValue(sPos.x());
            gOutPanel->edY->setValue(sPos.y());
            gOutPanel->edX->blockSignals(false);
            gOutPanel->edY->blockSignals(false);
        }
        if (sizeChanged) {
            view->resize(sSize);
            gOutPanel->edW->blockSignals(true);
            gOutPanel->edH->blockSignals(true);
            gOutPanel->edW->setValue(sSize.width());
            gOutPanel->edH->setValue(sSize.height());
            gOutPanel->edW->blockSignals(false);
            gOutPanel->edH->blockSignals(false);
            item->setText("size" * *gOutPanel->tree, QString("%1×%2").arg(sSize.width()).arg(sSize.height()));
        }
    }
    else {
        auto cell = (Cell*)gTable->data(rowIdx, gPlayinC).toULongLong();
        if (cell) {
            if (posChanged) cell->wgt->setPos(sPos);
            if (sizeChanged) {
                if (cell->type == 'V') {
                    ((QGraphicsVideoItem*)cell->wgt)->setSize(sSize);
                }
                else if (cell->type == 'I') {
                    ((ImgItem*)cell->wgt)->size = sSize;
                }
            }
        }
    }
}

// Layer类的鼠标离开事件处理函数，用于重置调整状态
void Layer::leaveEvent(QEvent*) {
    setFrmSecIfNeed(Qt::NoSection, Qt::ArrowCursor);
    mPressRel.setX(FLT_MAX);
}

// 设置当前调整的窗口区域
void Layer::setFrmSec(const QPointF& pos) {
    if (hdlLT.contains(pos))      setFrmSecIfNeed(Qt::TopLeftSection, Qt::SizeFDiagCursor);
    else if (hdlT.contains(pos))  setFrmSecIfNeed(Qt::TopSection, Qt::SizeVerCursor);
    else if (hdlRT.contains(pos)) setFrmSecIfNeed(Qt::TopRightSection, Qt::SizeBDiagCursor);
    else if (hdlL.contains(pos))  setFrmSecIfNeed(Qt::LeftSection, Qt::SizeHorCursor);
    else if (hdlR.contains(pos))  setFrmSecIfNeed(Qt::RightSection, Qt::SizeHorCursor);
    else if (hdlLB.contains(pos)) setFrmSecIfNeed(Qt::BottomLeftSection, Qt::SizeBDiagCursor);
    else if (hdlB.contains(pos))  setFrmSecIfNeed(Qt::BottomSection, Qt::SizeVerCursor);
    else if (hdlRB.contains(pos)) setFrmSecIfNeed(Qt::BottomRightSection, Qt::SizeFDiagCursor);
    else if (pos.x() >= 0 && pos.x() <= width() && pos.y() >= 0 && pos.y() <= height()) setFrmSecIfNeed(Qt::TitleBarArea, Qt::SizeAllCursor);
    else setFrmSecIfNeed(Qt::NoSection, Qt::ArrowCursor);
}

// 根据需要设置调整的窗口区域和鼠标形状
void Layer::setFrmSecIfNeed(Qt::WindowFrameSection frmSec, Qt::CursorShape cursor) {
    if (mFrmSec == frmSec) return;
    mFrmSec = frmSec;
    if (cursor == Qt::ArrowCursor) unsetCursor();
    else setCursor(cursor);
}

// 清除吸附状态
void Layer::clearSnap() {
    if (snapLR == 0 && snapTB == 0) return;
    snapLR = snapTB = 0;
    update();
}

// OutputView类的构造函数，初始化OpenGL视口
OutputView::OutputView(QWidget* parent) : QGraphicsView(parent) {
    auto format = QSurfaceFormat::defaultFormat();
    format.setSwapBehavior(QSurfaceFormat::SingleBuffer);
    format.setRenderableType(QSurfaceFormat::OpenGL);
    auto glWgt = new QOpenGLWidget;
    glWgt->setFormat(format);

    setViewport(glWgt);
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_AlwaysStackOnTop);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setBackgroundBrush(Qt::black);
    setAlignment(Qt::AlignTop | Qt::AlignLeft);
    setTransformationAnchor(QGraphicsView::NoAnchor);
    setInteractive(false);
    setScene(gScene);
}

// OutputView类的鼠标按下事件处理函数，用于视图的移动准备
void OutputView::mousePressEvent(QMouseEvent* e) {
    QWidget::mousePressEvent(e);
    if (e->button() != Qt::LeftButton) return;
    auto mousePos = e->globalPosition();
    auto elePos = pos();
    mPressRel = elePos - mousePos;
}

// OutputView类的鼠标释放事件处理函数，用于结束视图的移动
void OutputView::mouseReleaseEvent(QMouseEvent* event) {
    QWidget::mouseReleaseEvent(event);
    if (Qt::LeftButton == event->button()) {
        mPressRel.setX(FLT_MAX);
    }
    setAttribute(Qt::WA_AlwaysStackOnTop);
}
#define SnapSpace 6
// OutputView类的鼠标移动事件处理函数，用于视图的移动
void OutputView::mouseMoveEvent(QMouseEvent* e) {
    if (!(e->buttons() & Qt::LeftButton)) return;
    if (mPressRel.x() >= FLT_MAX) return;
    auto mousePos = e->globalPosition();
    auto dstHor = mPressRel.x() + mousePos.x();
    auto dstVer = mPressRel.y() + mousePos.y();
    // if(snapLR==0) for(auto layer : gEditView->layers) if(layer!=this) {//左右
    //     if(fabs(dstHor - layer->x()) < SnapSpace) {
    //         dstHor = layer->x();
    //         snapLR = 1;
    //         layer->snapLR = 1;
    //         layer->update();
    //         break;
    //     }
    //     auto eleRight = layer->x() + layer->width();
    //     if(fabs(dstHor - eleRight) < SnapSpace) {
    //         dstHor = eleRight;
    //         snapLR = 1;
    //         layer->snapLR = 2;
    //         layer->update();
    //         break;
    //     }
    //     auto right = dstHor + width();
    //     if(fabs(right - layer->x()) < SnapSpace && layer->x() - width() >= 0) {
    //         dstHor = layer->x() - width();
    //         snapLR = 2;
    //         layer->snapLR = 1;
    //         layer->update();
    //         break;
    //     }
    //     if(fabs(right - eleRight) < SnapSpace && eleRight - width() >= 0) {
    //         dstHor = eleRight - width();
    //         snapLR = 2;
    //         layer->snapLR = 2;
    //         layer->update();
    //         break;
    //     }
    // }
    // if(snapTB==0) for(auto layer : gEditView->layers) if(layer!=this) {//上下
    //     if(fabs(dstVer-layer->y()) < SnapSpace) {
    //         dstVer = layer->y();
    //         snapTB = 1;
    //         layer->snapTB = 1;
    //         layer->update();
    //         break;
    //     }
    //     auto eleBtm = layer->y() + layer->height();
    //     if(fabs(dstVer - eleBtm) < SnapSpace) {
    //         dstVer = eleBtm;
    //         snapTB = 1;
    //         layer->snapTB = 2;
    //         layer->update();
    //         break }
    //     }
    //     auto btm = dstVer + height();
    //     if(fabs(btm - layer->y()) < SnapSpace && layer->y() - height() >= 0) {
    //         dstVer = layer->y() - height();
    //         snapTB = 2;
    //         layer->snapTB = 1;
    //         layer->update();
    //         break;
    //     }
    //     if(fabs(btm - eleBtm) < SnapSpace && eleBtm - height() >= 0) {
    //         dstVer = eleBtm - height();
    //         snapTB = 2;
    //         layer->snapTB = 2;
    //         layer->update();
    //         break;
    //     }
    // }
    move(dstHor, dstVer);
}

// OutputView类的鼠标离开事件处理函数，用于重置移动状态
void OutputView::leaveEvent(QEvent*) {
    //setFrmSecIfNeed(Qt::NoSection, Qt::ArrowCursor);
    mPressRel.setX(FLT_MAX);
}
