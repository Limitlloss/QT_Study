#include "qgui.h"
#include <QPainter>
#include <QResizeEvent>
#include <QProxyStyle>

const Qt::Alignment AlignRight = Qt::AlignRight | Qt::AlignVCenter;

ColItem TreeWidget::addCol(const QString& field, const QString& text, int width, QHeaderView::ResizeMode resizeMode) {
    int i = (int) fdmap.size();
    auto item = headerItem();
    item->setText(i, text);
    item->setData(i, FieldRole, field);
    if(width > 0) {
        blocked = true;
        if(header()->minimumSectionSize() > width) header()->setMinimumSectionSize(width);
        header()->resizeSection(i, width);
        blocked = false;
    }
    if(resizeMode != QHeaderView::Interactive) {
        if(resizeMode==QHeaderView::Stretch && width > 0) {
            item->setData(i, WidthRole, width);
            if(noStretch) {
                noStretch = false;
                connect(header(), &QHeaderView::sectionResized, this, &TreeWidget::onSectionResized);
                header()->installEventFilter(this);
            }
        } else header()->setSectionResizeMode(i, resizeMode);
    }
    fdmap.emplace(field, i);
    if(i==0) hasRowNum = field=="#";
    return {item, i};
}

bool TreeWidget::eventFilter(QObject *watched, QEvent *event) {
    if(watched==header()) {
        if(event->type()==QEvent::Resize) {
            auto eve = (QResizeEvent *) event;
            if(eve->size().width() != eve->oldSize().width()) adjSections(-1, 0);
            return true;
        } else if(isSectionResized && event->type()==QEvent::Leave) {
            isSectionResized = false;
            auto item = headerItem();
            for(int cc=0; cc<columnCount(); cc++) if(item->data(cc, WidthRole).isValid()) item->setData(cc, WidthRole, header()->sectionSize(cc));
            return true;
        }
    }
    return QTreeWidget::eventFilter(watched, event);
}
void TreeWidget::rowsInserted(const QModelIndex &parent, int start, int end) {
    QTreeWidget::rowsInserted(parent, start, end);
    if(parent.internalId()) return;
    if(minRowHeight) for(int rr=start; rr<=end; rr++) {
        auto item = topLevelItem(rr);
        if(item->sizeHint(0).height() < minRowHeight) item->setSizeHint(0, {0, minRowHeight});
    }
    int align;
    for(int cc=0; cc<columnCount(); cc++) if((align = headerItem()->data(cc, AlignRole).toInt())) for(int rr=start; rr<=end; rr++) topLevelItem(rr)->setTextAlignment(cc, (Qt::Alignment)align);
}
void TreeWidget::drawRow(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const {
    QTreeWidget::drawRow(painter, options, index);
    if(hasRowNum) {
        QRect rect(columnViewportPosition(0), options.rect.top(), columnWidth(0), options.rect.height());
        painter->fillRect(rect, header()->palette().button());
        painter->drawText(rect, Qt::AlignCenter, QString::number(index.row()+1));
    }
    if(hasGrid) {
        QBrush color({128, 128, 128, 128});
        QRect rec(options.rect.left()-1-horizontalOffset(), options.rect.top(), 1, options.rect.height());
        auto last = columnCount() - 1;
        if(hasGrid > 1) for(int i=0; i<last; i++) {
            rec.translate(columnWidth(header()->logicalIndex(i)), 0);
            painter->fillRect(rec, color);
        } else {
            int ttlwidth = columnWidth(header()->logicalIndex(last));
            for(int i=0; i<last; i++) {
                auto width = columnWidth(header()->logicalIndex(i));
                rec.translate(width, 0);
                painter->fillRect(rec, color);
                ttlwidth += width;
            }
            painter->fillRect(options.rect.left(), options.rect.bottom(), ttlwidth, 1, color);
        }
    }
}
void TreeWidget::onSectionResized(int logicalIndex, int oldSize, int newSize) {
    if(blocked || newSize==0 || oldSize==0) return;
    if(! headerItem()->data(logicalIndex, WidthRole).isValid()) return;
    if(adjSections(logicalIndex, newSize)) isSectionResized = true;
}
bool TreeWidget::adjSections(int index, int size) {
    auto item = headerItem();
    int remain = header()->width() - size, stretchWidth = 0, width;
    for(int cc=0; cc<columnCount(); cc++) if(cc!=index && ! header()->isSectionHidden(cc)) {
        width = item->data(cc, WidthRole)|-1;
        if(width > -1) {
            if(width==0) item->setData(cc, WidthRole, width = header()->sectionSize(cc));
            stretchWidth += width;
        } else remain -= header()->sectionSize(cc);
    }
    if(remain<=0 || stretchWidth==0) return false;
    auto min = header()->minimumSectionSize();
    blocked = true;
    for(int cc=0; cc<columnCount(); cc++) if(cc!=index && ! header()->isSectionHidden(cc) && (width = item->data(cc, WidthRole)|-1) > -1) header()->resizeSection(cc, qMax(min, width * remain / stretchWidth));
    blocked = false;
    return true;
}

class TreeItemMarginStyle : public QProxyStyle {
public:
    TreeItemMarginStyle(TreeWidget *wgt) : QProxyStyle(wgt->style()), _wgt(wgt) {}
#if(QT_VERSION_MAJOR > 5)
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override { //draw big indicator and cell focus mask
        if(option && option->type==QStyleOption::SO_ViewItem && ((QStyleOptionViewItem*) option)->features & QStyleOptionViewItem::HasCheckIndicator && widget==_wgt) { //PE_PanelItemViewItem  PE_IndicatorItemViewItemCheck
            auto opt = (QStyleOptionViewItem*) option;
            auto add = _wgt->headerItem()->data(opt->index.column(), MarginRole).toInt();
            if(add) {
                if(element==PE_IndicatorItemViewItemCheck) opt->rect.translate(add, 0); //move big indicator
                else if((opt->features & QStyleOptionViewItem::HasDisplay)==0) opt->rect.setLeft(opt->rect.right() + 1); //remove cell focus mask
                else {
                    opt->rect.setLeft(opt->rect.x() + (add << 1)); //move cell focus mask
                    if(opt->rect.width() < 0) opt->rect.setWidth(0);
                }
            }
        }
        QProxyStyle::drawPrimitive(element, option, painter, widget);
    }
    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override { //draw text, small indicator and focus mask around text
        if(option && widget==_wgt) {
            auto opt = (QStyleOptionViewItem*) option;
            opt->state &= ~State_HasFocus; //remove focus mask around text
            if(element==CE_ItemViewItem && option->type==QStyleOption::SO_ViewItem) {
                auto add = _wgt->headerItem()->data(opt->index.column(), MarginRole).toInt();
                if(add) { //move text and small indicator
                    if(opt->displayAlignment & Qt::AlignRight) opt->rect.setRight(opt->rect.right() - add);
                    if(opt->features & QStyleOptionViewItem::HasCheckIndicator) opt->rect.setLeft(opt->rect.x() + (add<<1));
                    else if((opt->displayAlignment & (Qt::AlignHCenter | Qt::AlignRight))==0) opt->rect.setLeft(opt->rect.x() + add);
                    if(opt->rect.width() < 0) opt->rect.setWidth(0);
                }
            }
        }
        QProxyStyle::drawControl(element, option, painter, widget);
    }
#else
    QRect subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const override {
        auto res = QProxyStyle::subElementRect(element, option, widget);
        auto width = res.width();
        if(width && option && option->type==QStyleOption::SO_ViewItem && widget==_wgt) {
            if(element==SE_ItemViewItemText) {
                auto index = ((QStyleOptionViewItem*)option)->index;
                auto add = ((TreeWidget*)widget)->headerItem()->data(index.column(), MarginRole).toInt();
                if(add) {
                    auto align = ((TreeWidget*)widget)->item(index.row())->textAlignment(index.column());
                    if(align & Qt::AlignRight) res.setWidth(width < add ? 0 : width - add);
                    else if((align & Qt::AlignHCenter)==0) {
                        if(((QStyleOptionViewItem*)option)->features & QStyleOptionViewItem::HasCheckIndicator) add += add>>1;
                        res.setLeft(res.x() + add);
                        if(width < add) res.setWidth(0);
                    }
                }
            } else if(((QStyleOptionViewItem*)option)->features & QStyleOptionViewItem::HasCheckIndicator) {
                auto index = ((QStyleOptionViewItem*)option)->index;
                auto add = ((TreeWidget*)widget)->headerItem()->data(index.column(), MarginRole).toInt();
                if(add) {
                    if(width < (add+3)<<1) add = width;
                    else add += add>>1;
                    res.setLeft(res.x() + add);
                    if(width < add) res.setWidth(0);
                }
            }
        }
        return res;
    }
    int pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const override {
        auto res = QProxyStyle::pixelMetric(metric, option, widget);
        if(metric==PM_FocusFrameHMargin && option && option->type==QStyleOption::SO_ViewItem && ((QStyleOptionViewItem*)option)->features & QStyleOptionViewItem::HasCheckIndicator && widget==_wgt) res += ((TreeWidget*)widget)->headerItem()->data(((QStyleOptionViewItem*)option)->index.column(), MarginRole).toInt();
        return res;
    }
#endif
    TreeWidget *_wgt = 0;
};

ColItem &ColItem::margin(int margin) {
    item->setData(i, MarginRole, margin);
    auto tree = (TreeWidget*) item->treeWidget();
    if(tree->noMargin) {
        tree->noMargin = false;
        tree->setStyle(new TreeItemMarginStyle(tree));
    }
    return *this;
}

TableWidget::TableWidget(std::initializer_list<ColAttr> colAttrs, int rows, QWidget *parent) : QTableWidget{rows, (int)colAttrs.size(), parent} {
    int i = 0;
    for(auto attr = colAttrs.begin(); attr < colAttrs.end(); ++attr) {
        auto item = horizontalHeaderItem(i);
        if(item==0) setHorizontalHeaderItem(i, item = new QTableWidgetItem);
        item->setText(attr->text);
        item->setData(FieldRole, attr->field);
        if(attr->width > 0) horizontalHeader()->resizeSection(i, attr->width);
        if(attr->resizeMode != QHeaderView::Interactive) {
            if(attr->resizeMode==QHeaderView::Stretch && attr->width > 0) {
                item->setData(WidthRole, attr->width);
                if(noStretch) {
                    noStretch = false;
                    connect(horizontalHeader(), &QHeaderView::sectionResized, this, &TableWidget::onSectionResized);
                    horizontalHeader()->installEventFilter(this);
                }
            } else horizontalHeader()->setSectionResizeMode(i, attr->resizeMode);
        }
        fdmap.emplace(attr->field, i++);
    }
}
bool TableWidget::eventFilter(QObject *watched, QEvent *event) {
    if(watched==horizontalHeader()) {
        if(event->type()==QEvent::Resize) {
            auto eve = (QResizeEvent *) event;
            if(eve->size().width() != eve->oldSize().width()) adjSections(-1, 0);
            return true;
        } else if(isSectionResized && event->type()==QEvent::Leave) {
            isSectionResized = false;
            QTableWidgetItem *item;
            for(int cc=0; cc<columnCount(); cc++) if((item = horizontalHeaderItem(cc)) && item->data(WidthRole).isValid()) item->setData(WidthRole, horizontalHeader()->sectionSize(cc));
            return true;
        }
    }
    return QTableWidget::eventFilter(watched, event);
}
void TableWidget::onSectionResized(int logicalIndex, int oldSize, int newSize) {
    if(blocked || newSize==0 || oldSize==0) return;
    if(! horizontalHeaderItem(logicalIndex)->data(WidthRole).isValid()) return;
    if(adjSections(logicalIndex, newSize)) isSectionResized = true;
}
bool TableWidget::adjSections(int index, int size) {
    QTableWidgetItem *item;
    int remain = horizontalHeader()->width() - size, stretchWidth = 0, width;
    for(int cc=0; cc<columnCount(); cc++) if(cc!=index && ! horizontalHeader()->isSectionHidden(cc) && (item = horizontalHeaderItem(cc))) {
        width = item->data(WidthRole)|-1;
        if(width > -1) {
            if(width==0) item->setData(WidthRole, width = horizontalHeader()->sectionSize(cc));
            stretchWidth += width;
        } else remain -= horizontalHeader()->sectionSize(cc);
    }
    if(remain<=0 || stretchWidth==0) return false;
    auto min = horizontalHeader()->minimumSectionSize();
    blocked = true;
    for(int cc=0; cc<columnCount(); cc++) if(cc!=index && ! horizontalHeader()->isSectionHidden(cc) && (item = horizontalHeaderItem(cc)) && (width = item->data(WidthRole)|-1) > -1) horizontalHeader()->resizeSection(cc, qMax(min, width * remain / stretchWidth));
    blocked = false;
    return true;
}
