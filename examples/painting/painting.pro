TEMPLATE      = subdirs
SUBDIRS       = basicdrawing \
                concentriccircles \
                imagecomposition \
                painterpaths \
                transformations

!wince*:!symbian: SUBDIRS += fontsampler

contains(QT_CONFIG, svg): SUBDIRS += svgviewer

# install
target.path = $$[QT_INSTALL_EXAMPLES]/painting
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS painting.pro README
sources.path = $$[QT_INSTALL_EXAMPLES]/painting
INSTALLS += target sources

include($$QT_SOURCE_TREE/examples/examplebase.pri)
