#ifndef QGUI_H
#define QGUI_H

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QListWidget>
#include <QSplitter>
#include <QStackedLayout>
#include <QTableWidget>
#include <QTextEdit>
#include <QTreeWidget>

#define MainMust \
#if(QT_VERSION_MAJOR > 5) \
    QImageReader::setAllocationLimit(0);\
#else\
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);\
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);\
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);\
#endif

extern const Qt::Alignment AlignRight;

inline int operator|(const QVariant &value, int def) {
    return value.isValid() ? value.toInt() : def;
}

inline QWidget *parentWgt(QObject *obj) {
    while(obj && ! obj->isWidgetType()) obj = obj->parent();
    return (QWidget*) obj;
}
inline QWidget *parentWin(QObject *obj) {
    while(obj) {
        if(obj->isWidgetType()) return ((QWidget*) obj)->window();
        obj = obj->parent();
    }
    return (QWidget*) obj;
}

inline int SetCurData(QComboBox *combo, const QVariant &data) {
    auto idx = combo->findData(data);
    if(idx>-1) combo->setCurrentIndex(idx);
    return idx;
}
inline int SetCurText(QComboBox *combo, const QString& text) {
    auto idx = combo->findText(text);
    if(idx>-1) combo->setCurrentIndex(idx);
    return idx;
}
inline void gFont(QWidget *wgt, int size, bool bold = false, bool italic = false) {
    auto ft = wgt->font();
    ft.setPixelSize(size);
    if(bold) ft.setBold(true);
    if(italic) ft.setItalic(true);
    wgt->setFont(ft);
}
inline void gFont(QWidget *wgt, const QString& family, int size = 0, bool bold = false, bool italic = false) {
    auto ft = wgt->font();
    ft.setFamily(family);
    if(size) ft.setPixelSize(size);
    if(bold) ft.setBold(true);
    if(italic) ft.setItalic(true);
    wgt->setFont(ft);
}
inline QFont qfont(const QString& family, int pixelSize, bool bold = false, bool italic = false) {
    QFont ft(family);
    ft.setPixelSize(pixelSize);
    if(bold) ft.setBold(true);
    if(italic) ft.setItalic(true);
    return ft;
}
inline void gAppendText(QTextEdit *wgt, const QString& text, const QColor &color) {
    auto c0 = wgt->textColor();
    wgt->setTextColor(color);
    wgt->append(text);
    wgt->setTextColor(c0);
}

class VBox : public QBoxLayout {
public:
    VBox(QWidget *parent=nullptr) : QBoxLayout(TopToBottom, parent) {}
    VBox(QBoxLayout *parent) : QBoxLayout(TopToBottom) {
        parent->addLayout(this);
    };
    VBox(QStackedLayout *stack) : QBoxLayout(TopToBottom, new QWidget) {
        stack->addWidget(parentWidget());
        setContentsMargins(0,0,0,0);
    };
    VBox(QSplitter *splitter) : QBoxLayout(TopToBottom, new QWidget) {
        splitter->addWidget(parentWidget());
        setContentsMargins(0,0,0,0);
    };
    QLabel *addLabel() {
        auto lb = new QLabel;
        addWidget(lb);
        return lb;
    }
    QLabel *addLabel(const QString& text) {
        auto lb = new QLabel(text);
        addWidget(lb);
        return lb;
    }
    QDialogButtonBox *addBtnBox(QDialog *dlg = 0) {
        auto btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        if(dlg) connect(btnBox, &QDialogButtonBox::rejected, dlg, &QDialog::reject);
        addWidget(btnBox);
        return btnBox;
    }
};
class HBox : public QBoxLayout {
public:
    HBox(QWidget *parent=nullptr) : QBoxLayout(LeftToRight, parent) {}
    HBox(QBoxLayout *parent) : QBoxLayout(LeftToRight) {
        parent->addLayout(this);
    };
    HBox(QStackedLayout *stack) : QBoxLayout(LeftToRight, new QWidget) {
        stack->addWidget(parentWidget());
        setContentsMargins(0,0,0,0);
    };
    HBox(QSplitter *splitter) : QBoxLayout(LeftToRight, new QWidget) {
        splitter->addWidget(parentWidget());
        setContentsMargins(0,0,0,0);
    };
    QLabel *addLabel() {
        auto lb = new QLabel;
        addWidget(lb);
        return lb;
    }
    QLabel *addLabel(const QString& text) {
        auto lb = new QLabel(text);
        addWidget(lb);
        return lb;
    }
};
class Grid : public QGridLayout {
public:
    using QGridLayout::QGridLayout;
    Grid(QBoxLayout *parent) {
        parent->addLayout(this);
    };
    Grid(QStackedLayout *stack) : QGridLayout(new QWidget) {
        stack->addWidget(parentWidget());
    };
    Grid(QSplitter *splitter) : QGridLayout(new QWidget) {
        splitter->addWidget(parentWidget());
    };
    QLabel *addLabel(const QString& text, int row, int column, Qt::Alignment align = Qt::Alignment()) {
        auto lb = new QLabel(text);
        addWidget(lb, row, column, align);
        return lb;
    }
};

class ListWgt : public QListWidget {
public:
    using QListWidget::QListWidget;

    using QListWidget::addItem;
    auto addItem(const QString& text, const QVariant &value) {
        auto item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, value);
        insertItem(count(), item);
        return this;
    }
};

struct ColAttr {
    ColAttr(const QString& field, const QString& text, int width=0, QHeaderView::ResizeMode resizeMode = QHeaderView::Interactive) : field(field), text(text), width(width), resizeMode(resizeMode) {}
    ColAttr(const QString& field, const QString& text, QHeaderView::ResizeMode resizeMode) : field(field), text(text), resizeMode(resizeMode) {}
    QString field;
    QString text;
    int width = 0;
    QHeaderView::ResizeMode resizeMode;
};
enum ItemDataRole {
    MarginRole = 0xfc,
    AlignRole,
    FieldRole,
    WidthRole
};

struct ColItem {
    QTreeWidgetItem *item;
    int i;
    auto &align(int align) {
        item->setData(i, AlignRole, align);
        return *this;
    }
    auto &alignC() {
        return align(Qt::AlignCenter);
    }
    auto &alignR() {
        return align(Qt::AlignVCenter | Qt::AlignRight);
    }
    ColItem &margin(int margin);
};
class TreeWidgetItem;

class TreeWidget : public QTreeWidget {
    friend ColItem;
    Q_OBJECT
public:
    using QTreeWidget::QTreeWidget;
    ColItem addCol(const QString& field, const QString& text, int width = 0, QHeaderView::ResizeMode resizeMode = QHeaderView::Interactive);
    ColItem addCol(const QString& field, const QString& text, QHeaderView::ResizeMode resizeMode) {
        return addCol(field, text, 0, resizeMode);
    }

    auto setDefs() {
        if(! hasRowNum) setIndentation(0);
        setAlternatingRowColors(true);
        header()->setStretchLastSection(false);
        return this;
    }
    auto setHeaderAlignC() {
        auto item = headerItem();
        for(int cc=0; cc<columnCount(); ++cc) item->setTextAlignment(cc, Qt::AlignCenter);
    }

    auto setColFit() {
        header()->setSectionResizeMode(QHeaderView::ResizeToContents);
        return this;
    }
    auto setColWidth(int value, QHeaderView::ResizeMode mode = QHeaderView::Interactive) {
        header()->setDefaultSectionSize(value);
        if(mode!=QHeaderView::Interactive) header()->setSectionResizeMode(mode);
        return this;
    }

    auto setHeaderText(const QString& column, const QString& text) {
        headerItem()->setText(fdmap.at(column), text);
        return this;
    }

    using QTreeWidget::showColumn;
    void showColumn(const QString& column) {
        showColumn(fdmap.at(column));
    }
    using QTreeWidget::hideColumn;
    void hideColumn(const QString& column) {
        hideColumn(fdmap.at(column));
    }

    auto item(int idx) const {
        return (TreeWidgetItem*) topLevelItem(idx);
    }
    auto selectedItem() const {
        auto is = selectedItems();
        return is.isEmpty() ? 0 : (TreeWidgetItem*) is.at(0);
    }
    auto curItem() const {
        return (TreeWidgetItem*) currentItem();
    }

    QString field(int column) const {
        return headerItem()->data(column, FieldRole).toString();
    }
    QString sortField() const {
        return field(sortColumn());
    }
    using QTreeWidget::sortItems;
    void sortItems(const QString& column, Qt::SortOrder order = Qt::AscendingOrder) {
        sortItems(fdmap.at(column), order);
    }

    std::unordered_map<QString, int> fdmap;
    int minRowHeight = 0;
    char hasGrid = 1;
    bool hasRowNum = false;

    int sizeHintForColumn(int column) const override {
        return QTreeWidget::sizeHintForColumn(column);
    }
    bool adjSections(int index, int size);
signals:
    void updGeos();
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void updateGeometries() override {
        QTreeWidget::updateGeometries();
        emit updGeos();
    }
    void rowsInserted(const QModelIndex &parent, int start, int end) override;
    void drawRow(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const override;
    void onSectionResized(int logicalIndex, int oldSize, int newSize);
    bool noStretch = true;
    bool noMargin = true;
    bool isSectionResized = false;
    bool blocked = false;
};

inline int operator*(const QString& key, QTreeView &table) {
    if((size_t)&table==0) return -1;
    return ((TreeWidget&)table).fdmap.at(key);
}
inline int operator*(const char *key, QTreeView &table) {
    if((size_t)&table==0) return -1;
    return ((TreeWidget&)table).fdmap.at(key);
}

class TreeWidgetItem : public QTreeWidgetItem {
public:
    using QTreeWidgetItem::QTreeWidgetItem;

    using QTreeWidgetItem::checkState;
    auto checkState(const QString& column) const {
        return checkState(column**treeWidget());
    }
    using QTreeWidgetItem::setCheckState;
    auto setCheckState(const QString& column, Qt::CheckState state) {
        setCheckState(column**treeWidget(), state);
        return this;
    }

    using QTreeWidgetItem::text;
    auto text(const QString& column) const {
        return text(column**treeWidget());
    }
    using QTreeWidgetItem::setText;
    auto setText(const QString& column, const QString& text) {
        setText(column**treeWidget(), text);
        return this;
    }
    auto setText(const QString& column, const QString& text, const QVariant &value) {
        auto idx = column**treeWidget();
        setText(idx, text);
        setData(idx, Qt::UserRole, value);
        return this;
    }
    auto setText(const QString& column, const QString& text, int alignment) {
        auto idx = column**treeWidget();
        setText(idx, text);
        setTextAlignment(idx, (Qt::Alignment)alignment);
        return this;
    }

    using QTreeWidgetItem::background;
    auto background(const QString& column) const {
        return background(column**treeWidget());
    }
    using QTreeWidgetItem::setBackground;
    auto setBackground(const QString& column, const QBrush &brush) {
        setBackground(column**treeWidget(), brush);
        return this;
    }
    using QTreeWidgetItem::foreground;
    auto foreground(const QString& column) const {
        return foreground(column**treeWidget());
    }
    using QTreeWidgetItem::setForeground;
    auto setForeground(const QString& column, const QBrush &brush) {
        setForeground(column**treeWidget(), brush);
        return this;
    }

    using QTreeWidgetItem::data;
    auto data(int col) const {
        return data(col, Qt::UserRole);
    }
    auto data(const QString& column, int role = Qt::UserRole) const {
        return data(column**treeWidget(), role);
    }
    using QTreeWidgetItem::setData;
    auto setData(int col, const QVariant &value) {
        setData(col, Qt::UserRole, value);
        return this;
    }
    auto setData(const QString& column, const QVariant &value) {
        setData(column**treeWidget(), Qt::UserRole, value);
        return this;
    }
    auto setData(const QString& column, int role, const QVariant &value) {
        setData(column**treeWidget(), role, value);
        return this;
    }

    auto cellWidget(int column) {
        return treeWidget()->itemWidget(this, column);
    }
    auto cellWidget(const QString& column) {
        return treeWidget()->itemWidget(this, column**treeWidget());
    }
    void setCellWidget(int column, QWidget *widget) {
        treeWidget()->setItemWidget(this, column, widget);
    }
    void setCellWidget(const QString& column, QWidget *widget) {
        treeWidget()->setItemWidget(this, column**treeWidget(), widget);
    }
};

class TableWidget : public QTableWidget {
    Q_OBJECT
public:
    using QTableWidget::QTableWidget;
    TableWidget() {}
    TableWidget(std::initializer_list<ColAttr> colAttrs, int rows = 0, QWidget *parent = 0);

    auto setDefs() {
        setSelectionBehavior(QTableWidget::SelectRows);
        setEditTriggers(QAbstractItemView::NoEditTriggers);
        setAlternatingRowColors(true);
        return this;
    }
    auto setColStretch() {
        horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        return this;
    }
    auto setRowStretch() {
        verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        return this;
    }
    auto setColFit() {
        horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        return this;
    }
    auto setRowFit() {
        verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        return this;
    }

    auto setColWidth(int value) {
        if(horizontalHeader()->minimumSectionSize() > value) horizontalHeader()->setMinimumSectionSize(value);
        horizontalHeader()->setDefaultSectionSize(value);
        return this;
    }
    auto setColResize(QHeaderView::ResizeMode mode) {
        horizontalHeader()->setSectionResizeMode(mode);
        return this;
    }
    auto setRowHeight(int value) {
        if(verticalHeader()->minimumSectionSize() > value) verticalHeader()->setMinimumSectionSize(value);
        verticalHeader()->setDefaultSectionSize(value);
        return this;
    }
    auto setRowResize(QHeaderView::ResizeMode mode) {
        verticalHeader()->setSectionResizeMode(mode);
        return this;
    }

    auto setHeaderText(int col, const QString& text) {
        auto item = horizontalHeaderItem(col);
        if(item==0) setHorizontalHeaderItem(col, item = new QTableWidgetItem());
        item->setText(text);
        return item;
    }
    auto setHeaderText(const QString& column, const QString& text) {
        return setHeaderText(fdmap.at(column), text);
    }

    auto setVHeaderText(int row, const QString& text) {
        auto item = verticalHeaderItem(row);
        if(item==0) setVerticalHeaderItem(row, item = new QTableWidgetItem());
        item->setText(text);
        return item;
    }

    auto appendRow() {
        auto value = rowCount();
        setRowCount(value + 1);
        return value;
    }

    using QTableWidget::item;
    auto item(int row, const QString& column) {
        return item(row, fdmap.at(column));
    }
    auto itemValid(int row, int col) {
        auto itm = item(row, col);
        if(itm==0) setItem(row, col, itm = new QTableWidgetItem);
        return itm;
    }
    auto itemValid(int row, const QString& column) {
        return itemValid(row, fdmap.at(column));
    }
    using QTableWidget::setItem;
    void setItem(int row, const QString& column, QTableWidgetItem *item) {
        setItem(row, fdmap.at(column), item);
    }

    auto text(int row, int col) {
        auto itm = item(row, col);
        if(itm==0) return QString();
        return itm->text();
    }
    auto text(int row, const QString& column) {
        return text(row, fdmap.at(column));
    }

    auto setText(int row, int col, const QString& text) {
        auto itm = item(row, col);
        if(itm) itm->setText(text);
        else setItem(row, col, itm = new QTableWidgetItem(text));
        return itm;
    }
    auto setText(int row, const QString& column, const QString& text) {
        return setText(row, fdmap.at(column), text);
    }
    auto setText(int row, int col, const QString& text, const QVariant &value) {
        auto itm = item(row, col);
        if(itm) itm->setText(text);
        else setItem(row, col, itm = new QTableWidgetItem(text));
        itm->setData(Qt::UserRole, value);
        return itm;
    }
    auto setText(int row, const QString& column, const QString& text, const QVariant &value) {
        return setText(row, fdmap.at(column), text, value);
    }

    auto data(int row, int col) {
        auto itm = item(row, col);
        if(itm==0) return QVariant();
        return itm->data(Qt::UserRole);
    }
    auto data(int row, const QString& column) {
        return data(row, fdmap.at(column));
    }

    auto setData(int row, int col, const QVariant &value) {
        auto itm = item(row, col);
        if(itm==0) setItem(row, col, itm = new QTableWidgetItem);
        itm->setData(Qt::UserRole, value);
        return itm;
    }
    auto setData(int row, const QString& column, const QVariant &value) {
        return setData(row, fdmap.at(column), value);
    }

    using QTableWidget::cellWidget;
    auto cellWidget(int row, const QString& column) {
        return cellWidget(row, fdmap.at(column));
    }
    using QTableWidget::setCellWidget;
    void setCellWidget(int row, const QString& column, QWidget *widget) {
        setCellWidget(row, fdmap.at(column), widget);
    }

    using QTableWidget::sortItems;
    void sortItems(const QString& column, Qt::SortOrder order) {
        sortItems(fdmap.at(column), order);
    }

    std::unordered_map<QString, int> fdmap;
public Q_SLOTS:
    void clearRows() {setRowCount(0);}
signals:
    void updGeos();
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void updateGeometries() override {
        QTableWidget::updateGeometries();
        emit updGeos();
    };
    void onSectionResized(int logicalIndex, int oldSize, int newSize);
    bool adjSections(int index, int size);
    bool noStretch = true;
    bool isSectionResized = false;
    bool blocked = false;
};


template<class T>
class Wrp {
public:
    T *obj;
    Wrp(T *obj = nullptr){
        this->obj = obj;
    };
    Wrp& operator()(T *obj){
        this->obj = obj;
        return *this;
    }
    Wrp& operator()(T *obj, QLayout *layout){
        this->obj = obj;
        layout->addWidget(obj);
        return *this;
    }
    Wrp& addTo(QLayout *layout){
        layout->addWidget(obj);
        return *this;
    }
    Wrp& margin(int a){
        obj->setMargin(a);
        return *this;
    }
    Wrp& font(const QFont &font){
        obj->setFont(font);
        return *this;
    }
    Wrp& font(int size){
        auto font = obj->font();
        font.setPixelSize(size);
        obj->setFont(font);
        return *this;
    }

    Wrp& width(int w){
        obj->setFixedWidth(w);
        return *this;
    }
    Wrp& height(int h){
        obj->setFixedHeight(h);
        return *this;
    }
    Wrp& padding(int wAdd, int hAdd, int minW = 32, int minH = 16){
        wAdd+=8;
        hAdd+=8;
        QSize size = obj->fontMetrics().size(Qt::TextShowMnemonic, obj->text());
        int &rwidth = size.rwidth();
        rwidth += wAdd;
        if(rwidth < minW) rwidth = minW;
        int &rheight = size.rheight();
        rheight += hAdd;
        if(rheight < minH) rheight = minH;
        obj->setFixedSize(size);
        return *this;
    }

    Wrp& alignC(){
        obj->setAlignment(Qt::AlignCenter);
        return *this;
    }
    Wrp& alignR(){
        obj->setAlignment(Qt::AlignRight);
        return *this;
    }
};

#endif
