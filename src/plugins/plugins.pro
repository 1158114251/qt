TEMPLATE = subdirs

SUBDIRS	*= accessible imageformats sqldrivers iconengines script
unix:!symbian {
        contains(QT_CONFIG,iconv)|contains(QT_CONFIG,gnu-libiconv):SUBDIRS *= codecs
} else {
        SUBDIRS *= codecs
}
!embedded:SUBDIRS *= graphicssystems
embedded:SUBDIRS *=  gfxdrivers decorations mousedrivers kbddrivers
symbian:SUBDIRS += s60
contains(QT_CONFIG, phonon): SUBDIRS *= phonon
