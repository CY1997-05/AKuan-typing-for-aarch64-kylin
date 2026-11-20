#-------------------------------------------------
#
# HZTest - 中文打字速度测试软件 (Qt 5.5.1 复刻版)
#
# 由原 Delphi/SUIPack 版 HZTest.exe V1.41 逆向还原
#
#-------------------------------------------------

QT       += core gui network widgets

TARGET   = HZTest
TEMPLATE = app

CONFIG  += windows
CONFIG  -= console

DESTDIR     = bin
OBJECTS_DIR = build/obj
MOC_DIR     = build/moc
RCC_DIR     = build/rcc

# link.exe 不会自动创建 DESTDIR，缺 bin\ 时会报
#   LNK1104: 无法打开文件 "bin\HZTest.exe"
# 这里在链接前确保目录存在（Qt Creator 直接构建也能用）。
# 用相对路径，避免中文/空格路径在 nmake 命令行里被转码。
win32 {
    QMAKE_PRE_LINK += if not exist bin mkdir bin
}

# Linux：输入法桥接需要一个“ibus-daemon 空壳”（原因见 src/main.cpp 里的
# setupLinuxInputMethod 注释），随程序一起放到 bin/im/ 下。
unix:!macx {
    QMAKE_POST_LINK += mkdir -p $$DESTDIR/im && cp -f $$PWD/data/im/ibus-daemon $$DESTDIR/im/ibus-daemon && chmod 0755 $$DESTDIR/im/ibus-daemon
}

INCLUDEPATH += src

SOURCES += \
    src/main.cpp \
    src/common.cpp \
    src/config.cpp \
    src/store.cpp \
    src/textgen.cpp \
    src/typing.cpp \
    src/theme.cpp \
    src/suiform.cpp \
    src/widgets.cpp \
    src/forms/firstform.cpp \
    src/forms/optionform.cpp \
    src/forms/loginform.cpp \
    src/forms/testform.cpp \
    src/forms/resultform.cpp \
    src/forms/reportform.cpp \
    src/forms/candidateform.cpp \
    src/forms/gentxtform.cpp \
    src/forms/selectipform.cpp \
    src/forms/serverform.cpp \
    src/forms/hintform.cpp \
    src/forms/aboutform.cpp \
    src/forms/passform.cpp \
    src/net/lanserver.cpp \
    src/net/lanclient.cpp

HEADERS += \
    src/common.h \
    src/config.h \
    src/store.h \
    src/textgen.h \
    src/typing.h \
    src/theme.h \
    src/suiform.h \
    src/widgets.h \
    src/forms/firstform.h \
    src/forms/optionform.h \
    src/forms/loginform.h \
    src/forms/testform.h \
    src/forms/resultform.h \
    src/forms/reportform.h \
    src/forms/candidateform.h \
    src/forms/gentxtform.h \
    src/forms/selectipform.h \
    src/forms/serverform.h \
    src/forms/hintform.h \
    src/forms/aboutform.h \
    src/forms/passform.h \
    src/net/lanserver.h \
    src/net/lanclient.h

RESOURCES += resources.qrc

win32: RC_ICONS =
