TARGET = qcorewlanbearer
include(../../qpluginbase.pri)

QT += network
LIBS += -framework Foundation -framework SystemConfiguration

contains(QT_CONFIG, corewlan) {
    isEmpty(QMAKE_MAC_SDK)|contains(QMAKE_MAC_SDK, "/Developer/SDKs/MacOSX10.6.sdk") {
         LIBS += -framework CoreWLAN
         DEFINES += MAC_SDK_10_6
    }
}

DEFINES += BEARER_ENGINE

HEADERS += qcorewlanengine.h
SOURCES += qcorewlanengine.mm main.cpp

QTDIR_build:DESTDIR = $$QT_BUILD_TREE/plugins/bearer
target.path += $$[QT_INSTALL_PLUGINS]/bearer
INSTALLS += target
