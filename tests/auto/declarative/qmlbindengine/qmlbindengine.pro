load(qttest_p4)
contains(QT_CONFIG,declarative): QT += declarative
macx:CONFIG -= app_bundle
SOURCES += tst_qmlbindengine.cpp \
           testtypes.cpp
HEADERS += testtypes.h

# QMAKE_CXXFLAGS = -fprofile-arcs -ftest-coverage
# LIBS += -lgcov
