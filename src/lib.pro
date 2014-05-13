TEMPLATE = lib
HEADERS = \
    QWave4/WaveformVRuler.h \
    QWave4/WaveformSelectionProxy.h \
    QWave4/WaveformScrollBar.h \
    QWave4/WaveformRuler.h \
    QWave4/WaveformRegion.h \
    QWave4/WaveformCursorProxy.h \
    QWave4/WaveformBar.h \
    QWave4/Waveform.h \
    QWave4/Utils.h \
    QWave4/TimeLabel.h \
    QWave4/SndPlayerTicker.h \
    QWave4/SndPlayer.h \
    QWave4/SndFile.h \
    QWave4/SndDataThread.h \
    QWave4/SndBuffer.h \
    QWave4/qwavedefs.h \
    QWave4/Exceptions.h \
    QWave4/Events.h

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
    HEADERS += QWave4/SndPlayerDirectSound.h
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
    HEADERS += QWave4/SndPlayerPa.h
    SOURCES += SndPlayerPa.cc
    LIBS += -lpulse-simple
}
unix:freebsd-g++ { 
    LIBS += -L/pkg/ldc/freebsd/pkg/libsamplerate-0.1.2/lib
    INCLUDEPATH += /lib/oss/include \
        /pkg/ldc/freebsd/pkg/libsamplerate-0.1.2/include
    HEADERS += QWave4/SndPlayerOSS.h
    SOURCES += SndPlayerOSS.cc
}
unix:solaris-g++ { 
    HEADERS += QWave4/QWavePlayerSunAudio.h
    SOURCES += QWavePlayerSunAudio.cc
}
INCLUDEPATH += QWave4
DEPENDPATH += QWave4
CONFIG += qt \
    dll \
    warn_on \
    thread \
    debug
