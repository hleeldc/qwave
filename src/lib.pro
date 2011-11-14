TEMPLATE = lib
HEADERS = SndFile.h \
    SndPlayer.h \
    SndDataThread.h \
    SndBuffer.h \
    SndPlayerTicker.h \
    Waveform.h \
    WaveformScrollBar.h \
    WaveformRuler.h \
    WaveformVRuler.h \
    WaveformRegion.h \
    WaveformBar.h \
    WaveformCursorProxy.h \
    WaveformSelectionProxy.h \
    TimeLabel.h \
    Events.h \
    Utils.h \
    Exceptions.h

SOURCES = SndFile.cc \
    SndPlayer.cc \
    SndDataThread.cc \
    SndBuffer.cc \
    SndPlayerTicker.cc \
    Waveform.cc \
    WaveformScrollBar.cc \
    WaveformRuler.cc \
    WaveformVRuler.cc \
    WaveformRegion.cc \
    WaveformBar.cc \
    WaveformCursorProxy.cc \
    WaveformSelectionProxy.cc \
    TimeLabel.cc \
    Events.cc \
    Utils.cc

unix { 
    TARGET = qwave4
    LIBS += -lsndfile -lsamplerate
}
win32 { 
    TARGET = libqwave4
    HEADERS += SndPlayerDirectSound.h
    INCLUDEPATH += . "C:\Program Files\Mega-Nerd\libsndfile\include" \
        "C:\xtrans-build\libsamplerate-0.1.7\src" \
        "C:\Program Files\Microsoft DirectX SDK (June 2008)\Include"
    SOURCES += SndPlayerDirectSound.cc
    LIBS += "C:\Program Files\Microsoft DirectX SDK (June 2008)\Lib\x86\dsound.lib" \
            "C:\Program Files\Mega-Nerd\libsndfile\libsndfile-1.lib" \
            "C:\xtrans-build\libsamplerate-0.1.7\libsamplerate-0.lib"
    DEFINES += QWAVE_MAKE_DLL
}
unix:linux-g++ { 
    HEADERS += SndPlayerAlsa.h
    SOURCES += SndPlayerAlsa.cc
    INCLUDEPATH += /ldc/include
    LIBS += -L/ldc/lib -lasound
}
unix:freebsd-g++ { 
    LIBS += -L/pkg/ldc/freebsd/pkg/libsamplerate-0.1.2/lib
    INCLUDEPATH += /lib/oss/include \
        /pkg/ldc/freebsd/pkg/libsamplerate-0.1.2/include
    HEADERS += SndPlayerOSS.h
    SOURCES += SndPlayerOSS.cc
}
unix:solaris-g++ { 
    HEADERS += QWavePlayerSunAudio.h
    SOURCES += QWavePlayerSunAudio.cc
}
INCLUDEPATH += QWave4
DEPENDPATH += QWave4
CONFIG += qt \
    dll \
    warn_on \
    thread \
    release
