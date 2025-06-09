#include "main.h"
#include "mainwindow.h"
#include "QApplication"
#include "QImageReader"
#include "QStandardPaths"
#include "QSettings"

QString gFileHome;
QString gProgFile;
JObj gProg;
const QString UpdVerUrl;
QGraphicsScene* gScene;
EditView* gEditView;
double gScale = 0.1;
QPoint gOrigin{ 20, 20 };
OutputPanel* gOutPanel;
TableWidget* gTable, * gTableH, * gTableV;
int gPlayinC = 0;
//Tick *gTick;
int gProgWidth, gProgHeight;

// 主函数，程序入口
int main(int argc, char* argv[]) {
    QImageReader::setAllocationLimit(0);
    QApplication::setOrganizationName("Sysolution");
    QApplication::setStyle("Fusion");
    QApplication a(argc, argv);
    {
        QSettings settings;
        gFileHome = settings.value("FileHome").toString();
        settings.setValue("BackgroundColor", "blue"); // 自动写入注册表
        if (gFileHome.isEmpty()) gFileHome = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

        auto pal = a.palette();
        // 自定义应用程序的调色板样式
        pal.setBrush(QPalette::Active, QPalette::WindowText, QBrush({ 255,255,255,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Disabled, QPalette::WindowText, QBrush({ 157,157,157,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Inactive, QPalette::WindowText, QBrush({ 255,255,255,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Active, QPalette::Button, QBrush({ 60,60,60,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Disabled, QPalette::Button, QBrush({ 60,60,60,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Inactive, QPalette::Button, QBrush({ 60,60,60,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Active, QPalette::Light, QBrush({ 120,120,120,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Disabled, QPalette::Light, QBrush({ 120,120,120,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Inactive, QPalette::Light, QBrush({ 120,120,120,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Active, QPalette::Midlight, QBrush({ 90,90,90,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Disabled, QPalette::Midlight, QBrush({ 90,90,90,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Inactive, QPalette::Midlight, QBrush({ 90,90,90,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Active, QPalette::Dark, QBrush({ 30,30,30,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Disabled, QPalette::Dark, QBrush({ 30,30,30,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Inactive, QPalette::Dark, QBrush({ 30,30,30,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Active, QPalette::Mid, QBrush({ 40,40,40,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Disabled, QPalette::Mid, QBrush({ 40,40,40,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Inactive, QPalette::Mid, QBrush({ 40,40,40,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Active, QPalette::Text, QBrush({ 255,255,255,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Disabled, QPalette::Text, QBrush({ 157,157,157,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Inactive, QPalette::Text, QBrush({ 255,255,255,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Active, QPalette::BrightText, QBrush({ 95,255,165,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Disabled, QPalette::BrightText, QBrush({ 95,255,165,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Inactive, QPalette::BrightText, QBrush({ 95,255,165,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Active, QPalette::ButtonText, QBrush({ 255,255,255,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Disabled, QPalette::ButtonText, QBrush({ 157,157,157,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Inactive, QPalette::ButtonText, QBrush({ 255,255,255,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Active, QPalette::Base, QBrush({ 45,45,45,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Disabled, QPalette::Base, QBrush({ 30,30,30,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Inactive, QPalette::Base, QBrush({ 45,45,45,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Active, QPalette::Window, QBrush({ 30,30,30,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Disabled, QPalette::Window, QBrush({ 30,30,30,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Inactive, QPalette::Window, QBrush({ 30,30,30,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Active, QPalette::Shadow, QBrush({ 0,0,0,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Disabled, QPalette::Shadow, QBrush({ 0,0,0,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Inactive, QPalette::Shadow, QBrush({ 0,0,0,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Active, QPalette::Highlight, QBrush({ 0,204,106,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Disabled, QPalette::Highlight, QBrush({ 0,204,106,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Inactive, QPalette::Highlight, QBrush({ 30,30,30,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Active, QPalette::HighlightedText, QBrush({ 255,255,255,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Disabled, QPalette::HighlightedText, QBrush({ 255,255,255,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Inactive, QPalette::HighlightedText, QBrush({ 255,255,255,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Active, QPalette::Link, QBrush({ 0,204,106,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Disabled, QPalette::Link, QBrush({ 48,140,198,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Inactive, QPalette::Link, QBrush({ 0,204,106,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Active, QPalette::LinkVisited, QBrush({ 0,63,19,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Disabled, QPalette::LinkVisited, QBrush({ 255,0,255,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Inactive, QPalette::LinkVisited, QBrush({ 0,63,19,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Active, QPalette::AlternateBase, QBrush({ 0,63,19,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Disabled, QPalette::AlternateBase, QBrush({ 52,52,52,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Inactive, QPalette::AlternateBase, QBrush({ 0,63,19,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Active, QPalette::NoRole, QBrush({ 0,0,0,255 }, Qt::NoBrush));
        pal.setBrush(QPalette::Disabled, QPalette::NoRole, QBrush({ 0,0,0,255 }, Qt::NoBrush));
        pal.setBrush(QPalette::Inactive, QPalette::NoRole, QBrush({ 0,0,0,255 }, Qt::NoBrush));
        pal.setBrush(QPalette::Active, QPalette::ToolTipBase, QBrush({ 60,60,60,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Disabled, QPalette::ToolTipBase, QBrush({ 255,255,220,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Inactive, QPalette::ToolTipBase, QBrush({ 60,60,60,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Active, QPalette::ToolTipText, QBrush({ 212,212,212,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Disabled, QPalette::ToolTipText, QBrush({ 0,0,0,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Inactive, QPalette::ToolTipText, QBrush({ 212,212,212,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Active, QPalette::PlaceholderText, QBrush({ 255,255,255,128 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Disabled, QPalette::PlaceholderText, QBrush({ 255,255,255,128 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Inactive, QPalette::PlaceholderText, QBrush({ 255,255,255,128 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Active, QPalette::NColorRoles, QBrush({ 157,157,157,255 }, Qt::SolidPattern));
        pal.setBrush(QPalette::Disabled, QPalette::NColorRoles, QBrush({ 255,255,255,255 }, Qt::SolidPattern));

        pal.setBrush(QPalette::Inactive, QPalette::Highlight, pal.brush(QPalette::Active, QPalette::Highlight));
        a.setPalette(pal);

        //OpenDlg dlg;
        //if(dlg.exec()!=QDialog::Accepted) return 0;
    }
    MainWindow w;
    return a.exec();
}
