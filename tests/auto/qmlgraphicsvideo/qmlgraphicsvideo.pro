load(qttest_p4)
SOURCES += tst_qmlgraphicsvideo.cpp

QT += multimedia
requires(contains(QT_CONFIG, multimedia))
requires(contains(QT_CONFIG, declarative))
