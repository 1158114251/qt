load(qttest_p4)
contains(QT_CONFIG,declarative): QT += declarative gui
SOURCES += tst_qmltimer.cpp

DEFINES += SRCDIR=\\\"$$PWD\\\"
