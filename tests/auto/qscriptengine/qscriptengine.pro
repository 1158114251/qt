load(qttest_p4)
QT += script
SOURCES += tst_qscriptengine.cpp 
DEFINES += SRCDIR=\\\"$$PWD\\\"

wince*: {
   addFiles.sources = script
   addFiles.path = .
   DEPLOYMENT += addFiles
}

