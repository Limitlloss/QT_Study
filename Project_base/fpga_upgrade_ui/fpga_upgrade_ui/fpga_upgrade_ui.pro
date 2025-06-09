QT += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = fpga_upgrade_gui
TEMPLATE = app

SOURCES += \
    Protocol.cpp \
    UpgradeWorker.cpp \
    main.cpp \
    MainWindow.cpp \
    Protocol.cpp \
    UpgradeWorker.cpp

HEADERS += \
    MainWindow.h \
    Protocol.h \
    Protocol.h \
    UpgradeWorker.h \
    UpgradeWorker.h

FORMS += MainWindow.ui
