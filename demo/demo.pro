TEMPLATE = app
HEADERS = qwave2.h
SOURCES = qwave2.cc
TARGET = qwave2

INCLUDEPATH += ../src
DEPENDPATH += ../src ../src/QWave4

unix {
        LIBS += -L../src -L/ldc/lib -lqwave4
    INCLUDEPATH += /ldc/include
}

win32 {
	LIBS += ../src/Debug/libqwave2.lib
}

CONFIG += qt thread

