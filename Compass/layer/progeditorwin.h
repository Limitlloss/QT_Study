#ifndef PROGEDITORWIN_H
#define PROGEDITORWIN_H
#include "program/pageeditor.h"
#include <QCheckBox>
#include <QRadioButton>
#include <QDialog>
#include <QSpinBox>
#include <QTextEdit>
#include <QListWidget>
#include "gutil/qjson.h"

class ProgPanel;
class ProgItem;
class ProgEditorWin : public QWidget {
    Q_OBJECT
public:
    explicit ProgEditorWin(ProgItem *progItem, QWidget *);
public slots:
    void onSave();
    void onAddPage();
protected:
    void closeEvent(QCloseEvent *event) override;
private:
    bool isProgChanged();
    bool save();

    QListWidget *listPage;
    PageListItem *mPageItem = 0;
    PageEditor *mPageEditor;
    QTabWidget *mTabsAttr;
    std::vector<JObj> mPageJsons;
    ProgItem *mProgItem;
    int mNewEleX = 0;
    int mNewEleY = 0;
};

class ProgCreateDlg : public QDialog {
    Q_OBJECT
public:
    ProgCreateDlg(QString name, int width, int height, QString remarks, QString, bool isVer, QWidget *parent = nullptr);
    QLineEdit *fdName;
    QSpinBox *fdWidth;
    QSpinBox *fdHeight;
    QTextEdit *fdRemark;
    QCheckBox *edIsInsert, *fdIsUltraLong;
    QRadioButton *fdHor, *fdVer;
    QLineEdit *fdSplitWidths;
};

#endif // PROGEDITORWIN_H
