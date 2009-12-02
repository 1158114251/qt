TARGET     = QtDeclarative
QPRO_PWD   = $$PWD
QT         = core gui xml script network
contains(QT_CONFIG, svg): QT += svg
contains(QT_CONFIG, opengl): QT += opengl
DEFINES   += QT_BUILD_DECLARATIVE_LIB
win32-msvc*|win32-icc:QMAKE_LFLAGS += /BASE:0x66000000
solaris-cc*:QMAKE_CXXFLAGS_RELEASE -= -O2

unix:QMAKE_PKGCONFIG_REQUIRES = QtCore QtGui QtXml

exists("qml_enable_gcov") {
    QMAKE_CXXFLAGS = -fprofile-arcs -ftest-coverage -fno-elide-constructors
    LIBS += -lgcov
}

#INCLUDEPATH += 3rdparty util graphicsitems debugger qml

include(../qbase.pri)

#modules
include(3rdparty/3rdparty.pri)
include(util/util.pri)
include(graphicsitems/graphicsitems.pri)
include(qml/qml.pri)
include(widgets/widgets.pri)
include(debugger/debugger.pri)

symbian:TARGET.UID3=0x2001E623
