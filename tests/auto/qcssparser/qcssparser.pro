load(qttest_p4)
SOURCES += tst_qcssparser.cpp
DEFINES += SRCDIR=\\\"$$PWD\\\"
QT += xml

requires(contains(QT_CONFIG,private_tests))

wince*: {
   addFiles.sources = testdata
   addFiles.path = .
   DEPLOYMENT += addFiles
}
