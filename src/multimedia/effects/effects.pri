


system(pkg-config --exists \'libpulse >= 0.9.10\') {
    DEFINES += QT_MULTIMEDIA_PULSEAUDIO
    HEADERS += $$PWD/qsoundeffect_pulse_p.h
    SOURCES += $$PWD/qsoundeffect_pulse_p.cpp
    LIBS += -lpulse
} else:x11 {
    DEFINES += QT_MULTIMEDIA_QMEDIAPLAYER
    HEADERS += $$PWD/qsoundeffect_qmedia_p.h
    SOURCES += $$PWD/qsoundeffect_qmedia_p.cpp
} else {
    HEADERS += $$PWD/qsoundeffect_qsound_p.h
    SOURCES += $$PWD/qsoundeffect_qsound_p.cpp
}

HEADERS += \
    $$PWD/qsoundeffect_p.h \
    $$PWD/wavedecoder_p.h

SOURCES += \
    $$PWD/qsoundeffect.cpp \
    $$PWD/wavedecoder_p.cpp
