TEMPLATE      = subdirs
SUBDIRS       = blockingfortuneclient \
                broadcastreceiver \
                broadcastsender \
                download \
                downloadmanager \
                fortuneclient \
                fortuneserver \
                ftp \
                http \
                loopback \
                threadedfortuneserver \
                googlesuggest \
                torrent

symbian: SUBDIRS = ftp
# no QProcess
!vxworks:!qnx:SUBDIRS += network-chat

contains(QT_CONFIG, openssl):SUBDIRS += securesocketclient

# install
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS network.pro README
sources.path = $$[QT_INSTALL_EXAMPLES]/network
INSTALLS += sources

include($$QT_SOURCE_TREE/examples/examplebase.pri)
