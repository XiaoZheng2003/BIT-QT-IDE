QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    astyle.cpp \
    codeeditor.cpp \
    compilationthread.cpp \
    foldlistwidget.cpp \
    foldlistwidgetitem.cpp \
    highlighter.cpp \
    main.cpp \
    mainwindow.cpp \
    search.cpp \
    tab.cpp\
    replace.cpp

HEADERS += \
    astyle.h \
    codeeditor.h \
    compilationthread.h \
    foldlistwidget.h \
    foldlistwidgetitem.h \
    headers.h \
    highlighter.h \
    mainwindow.h \
    search.h \
    tab.h\
    replace.h

FORMS += \
    astyle.ui \
    mainwindow.ui \
    search.ui \
    tab.ui\
    replace.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc

helpFile = $$PWD/cppreference.chm
helpFile = $$replace(helpFile, /, \\)
formatFile = $$PWD/clang-format.exe
formatFile = $$replace(formatFile, /, \\)
OutputDir = $$OUT_PWD
OutputDir = $$replace(OutputDir, /, \\)
QMAKE_POST_LINK += $$QMAKE_COPY_FILE $$helpFile $$OutputDir \
                && $$QMAKE_COPY_FILE $$formatFile $$OutputDir

#程序版本
VERSION = 1.0.3
#程序图标
RC_ICONS = icon.ico
#公司名称
QMAKE_TARGET_COMPANY ="基因重组"
#版权信息
QMAKE_TARGET_COPYRIGHT = "Copyright(C) 2023"
#程序名称
QMAKE_TARGET_PRODUCT = "CEditor"
#程序语言
#0x0800代表和系统当前语言一致
RC_LANG = 0x0800
