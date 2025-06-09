#include "progitem.h"
#include "QtWidgets/qdialogbuttonbox.h"
#include "QtWidgets/qlineedit.h"
#include "gutil/qcore.h"
#include "gutil/qgui.h"
#include "gutil/qwaitingdlg.h"
#include "gutil/qwaitingdlg.h"
#include "gentmpthread.h"
#include <main.h>
#include "sendprogramdialog.h"
#include "program/progeditorwin.h"
#include <QJsonArray>
#include <QMessageBox>
#include <QStorageInfo>

ProgItem::ProgItem(const QString &progsDir, const QString &name, int w, int h, const QString &remarks, std::vector<int> &partLens, int maxLen, bool isVer, LoQTreeWidget *tree) : TreeWidgetItem(tree),
    mName(name), mWidth(w), mHeight(h), mRemark(remarks), partLens(partLens), maxLen(maxLen), isVer(isVer), mProgsDir(progsDir) {
    setText("lasttime", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    mProgDir = progsDir + "/" + mName;
    m_orgName = mName;
    init();
}

void ProgItem::init() {
    setFlags(flags() & ~Qt::ItemIsUserCheckable);
    setCheckState("check", Qt::Unchecked);
    auto ft = font(0);
    ft.setPixelSize(14);
    for(int i=1; i<treeWidget()->columnCount()-2; i++) {
        setFont(i, ft);
        setTextAlignment(i, Qt::AlignCenter);
    }
    setForeground("name"**treeWidget(), Qt::transparent);
    setText("name", mName);
    setText("resolution", QString("%1 x %2").arg(mWidth).arg(mHeight));
    setText("size", m_fsize<=0 ? "100B" : byteSizeStr(m_fsize));

    btnName = new QPushButton(mName);
    btnName->setFont(ft);
    btnName->setCursor(QCursor(Qt::PointingHandCursor));
    btnName->setStyleSheet(R"delimiter(
QPushButton{border-radius: 4px;}
QPushButton:hover {
    background-color: #ccc;
    text-decoration: underline;
}
)delimiter");
    QObject::connect(btnName, &QPushButton::clicked, treeWidget(), [=] {
        auto editor = new ProgEditorWin(this, treeWidget()->parentWidget());
        editor->show();
    });
    setCellWidget("name", btnName);

    auto btnExport = new QPushButton;
    btnExport->setCursor(QCursor(Qt::PointingHandCursor));
    btnExport->setStyleSheet(R"rrr(
QPushButton {
    border-radius: 4px;
    image: url(:/res/program/bnExport_u.png);
}
QPushButton:pressed{image: url(:/res/program/bnExport_s.png);}
QPushButton:hover{background-color: #cccccc;}
)rrr");
    QObject::connect(btnExport, &QPushButton::clicked, treeWidget(), [=] {
        QDialog dlg(treeWidget());
        dlg.setWindowTitle(QObject::tr("USB Update Program"));
#ifdef Q_OS_WIN
        dlg.setWindowFlag(Qt::WindowContextHelpButtonHint, 0);
#endif
        dlg.resize(300, 260);

        auto vBox = new VBox(&dlg);
        auto hBox = new HBox(vBox);

        hBox->addWidget(new QLabel(QObject::tr("Password")));

        auto fdPassword = new QLineEdit;
        fdPassword->setEchoMode(QLineEdit::Password);
        fdPassword->setPlaceholderText(translate("","Input Password"));
        hBox->addWidget(fdPassword);

        auto fdDrives = new ListWgt;
        fdDrives->setSelectionRectVisible(true);
        vBox->addWidget(fdDrives);

        auto btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        QObject::connect(btnBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        QObject::connect(btnBox, &QDialogButtonBox::accepted, &dlg, [=, &dlg] {
            auto selects = fdDrives->selectedItems();
            if(selects.count() > 0) {
                foreach(auto select, selects) {
                    auto strPath = select->data(Qt::UserRole).toString();
                    auto waitingDlg = new WaitingDlg(&dlg, QObject::tr("Convertering")+" ...");
                    auto gen = new GenTmpThread(this, mName, strPath + (strPath.endsWith('/') ? "program.zip" : "/program.zip"), fdPassword->text());
                    QObject::connect(gen, &GenTmpThread::onErr, &dlg, [=, &dlg](QString err) {
                        QMessageBox::warning(&dlg, "GenTmpThread Error", err);
                    });
                    QObject::connect(gen, &QThread::finished, waitingDlg, &WaitingDlg::success);
                    gen->start();
                    waitingDlg->exec();
                }
                dlg.accept();
                return;
            }
            if(fdDrives->count() <= 0) QMessageBox::warning(&dlg, translate("","Tip"), QObject::tr("No checked USB device"));
            else QMessageBox::warning(&dlg, translate("","Tip"), QObject::tr("please select usb device in list"));
        });
        vBox->addWidget(btnBox);

        fdDrives->clear();
        auto volumes = QStorageInfo::mountedVolumes();
        foreach(auto volume, volumes) {
            auto text = volume.displayName();
            auto rootPath = volume.rootPath();
            if(text!=rootPath) text += " ("+rootPath+")";
            fdDrives->addItem(text, rootPath);
        }

        dlg.exec();
    });
    setCellWidget("usb", btnExport);

    auto btnSend = new QPushButton;
    btnSend->setCursor(QCursor(Qt::PointingHandCursor));
    btnSend->setStyleSheet(R"rrr(
QPushButton{
    border-radius: 4px;
    image: url(:/res/program/bnSend_u.png);
}
QPushButton:pressed{image: url(:/res/program/bnSend_s.png);}
QPushButton:hover{background-color: #cccccc;}
)rrr");
    QObject::connect(btnSend, &QPushButton::clicked, treeWidget(), [this] {
        auto waitingDlg = new WaitingDlg(treeWidget(), QObject::tr("Convertering")+" ...");
        auto gen = new GenTmpThread(this, mName, "", "");
        QObject::connect(gen, &GenTmpThread::onErr, treeWidget(), [=](QString err) {
            QMessageBox::warning(treeWidget(), "GenTmpThread Error", err);
        });
        QObject::connect(gen, &QThread::finished, waitingDlg, &WaitingDlg::close);
        gen->start();
        waitingDlg->exec();
        SendProgramDialog dlg(mName, treeWidget()->window());
        dlg.exec();
    });
    setCellWidget("send", btnSend);
}

void ProgItem::save() {
    QDir dRoot(mProgDir);
    QDir dParent(mProgsDir);
    if(!dRoot.exists()) {
        dParent.mkdir(mName);
    } else if(mName != m_orgName) {
        dParent.rename(m_orgName, mName);
        m_orgName = mName;
        mProgDir = mProgsDir + "/" + mName;
    }

    dRoot = QDir(mProgDir);
    if(dRoot.exists()) {
        JObj json;
        json["name"]       = mName;
        json["resolution"] = JObj{{"w", mWidth}, {"h", mHeight}};
        json["remarks"]    = mRemark;
        json["isInsert"]   = isInsert;
        json["isVer"]      = isVer;
        JArray partLens;
        for(auto partLen : this->partLens) partLens.append(partLen);
        json["splitWidths"] = partLens;
        json["file_size"] = m_fsize;
        QFile file(mProgDir + "/pro.json");
        if(file.open(QIODevice::WriteOnly)) {
            file.write(JToBytes(json, "\t"));
            file.close();
        }
    }
}

void ProgItem::del() {
    QDir dRoot(mProgDir);
    if(dRoot.exists()) dRoot.removeRecursively();
}
//设置列表项的值
void ProgItem::onSetProgram() {
    btnName->setText(mName);
    m_fsize = dirFileSize(mProgDir);
    setText("resolution", QString("%1 x %2").arg(mWidth).arg(mHeight));
    setText("size", byteSizeStr(m_fsize<=0 ? 100 : m_fsize));
    setText("lasttime", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    save();
}
