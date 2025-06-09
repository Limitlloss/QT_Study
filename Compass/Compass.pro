QT += core gui widgets
QT += network multimedia multimediawidgets openglwidgets

CONFIG += c++17
CONFIG -= debug_and_release

win32:RC_ICONS = icon.ico
RESOURCES += res.qrc

win32 {
    EXE_SUFFIX = .exe

    # copy.files += $$files(ffmpeg/bin/*.dll)
    # copy.files += 7z/7z.dll
    # copy.files += 7z/7z.exe

    copy.path = $$OUT_PWD
    copydir.path = $$OUT_PWD
    CONFIG += file_copies
    COPIES += copy
    COPIES += copydir
}
osx {
    DIR_SUFFIX = -mac

    copy.path = Contents/MacOS
    copydir.path = Contents/MacOS
    QMAKE_BUNDLE_DATA += copy
    QMAKE_BUNDLE_DATA += copydir
}
# copy.files += ffmpeg$$DIR_SUFFIX/bin/ffmpeg$$EXE_SUFFIX

# INCLUDEPATH += $$PWD/ffmpeg$$DIR_SUFFIX/include
# LIBS += -L$$PWD/ffmpeg$$DIR_SUFFIX/lib/\
#         -lavcodec \
#         -lavdevice \
#         -lavfilter \
#         -lavformat \
#         -lavutil \
#         -lswresample \
#         -lswscale


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    gutil/cu.cpp \
    gutil/qaesencryption.cpp \
    gutil/qcore.cpp \
    gutil/qgui.cpp \
    gutil/qjson.cpp \
    gutil/qnetwork.cpp \
    layer.cpp \
    liveeditor.cpp \
    main.cpp \
    mainwindow.cpp \
    mediapanel.cpp \
    opendlg.cpp \
    outputpanel.cpp \
    planpanel.cpp \
    progresspanel.cpp

HEADERS += \
    gutil/cu.h \
    gutil/qaesencryption.h \
    gutil/qcore.h \
    gutil/qgui.h \
    gutil/qjson.h \
    gutil/qnetwork.h \
    layer.h \
    liveeditor.h \
    main.h \
    mainwindow.h \
    mediapanel.h \
    opendlg.h \
    outputpanel.h \
    planpanel.h \
    progresspanel.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
