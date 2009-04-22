TARGET     = QtDeclarative
QPRO_PWD   = $$PWD
QT         = core gui xml script network
contains(QT_CONFIG, svg): QT += svg
DEFINES   += QT_BUILD_DECLARATIVE_LIB
DEFINES   += QT_NO_USING_NAMESPACE
win32-msvc*|win32-icc:QMAKE_LFLAGS += /BASE:0x66000000
solaris-cc*:QMAKE_CXXFLAGS_RELEASE -= -O2

unix:QMAKE_PKGCONFIG_REQUIRES = QtCore QtGui QtXml

include(../qbase.pri)

#modules
include(3rdparty/3rdparty.pri)
include(util/util.pri)
include(fx/fx.pri)
include(canvas/canvas.pri)
include(qml/qml.pri)
include(timeline/timeline.pri)
include(extra/extra.pri)
include(widgets/widgets.pri)
include(test/test.pri)

contains(QT_CONFIG, opengles2)|contains(QT_CONFIG, opengles1):include(opengl/opengl.pri)
