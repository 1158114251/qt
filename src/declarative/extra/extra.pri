SOURCES += \
           extra/qnumberformat.cpp \
           extra/qmlnumberformatter.cpp \
           extra/qmldatetimeformatter.cpp \
           extra/qmlbehavior.cpp \
           extra/qmlfontloader.cpp

HEADERS += \
           extra/qnumberformat_p.h \
           extra/qmlnumberformatter_p.h \
           extra/qmldatetimeformatter_p.h \
           extra/qmlbehavior_p.h \
           extra/qmlfontloader_p.h

contains(QT_CONFIG, xmlpatterns) {
    QT+=xmlpatterns
    SOURCES += extra/qmlxmllistmodel.cpp
    HEADERS += extra/qmlxmllistmodel_p.h
}

