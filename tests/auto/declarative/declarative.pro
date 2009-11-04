TEMPLATE = subdirs
SUBDIRS += anchors \
           animatedimage \
           animations \
           behaviors \
           datetimeformatter \
           examples \
           layouts \
           listview \
           numberformatter \
           pathview \
           qfxloader \
           qfxpixmapcache \
           qfxtextedit \
           qfxtextinput \
           qfxwebview \
           qmetaobjectbuilder \
           qmlcontext \
           qmldom \
           qmlecmascript \
           qmlgraphicstext \
           qmlgraphicsborderimage \
           qmlfontloader \
           qmllanguage \
           qmllist \
           qmllistaccessor \
           qmlmetaproperty \
           qmlmetatype \
           qmlpropertymap \
           qmltimer \
           repeater \
           sql \
           states \
           visual

# Tests which should run in Pulse
PULSE_TESTS = $$SUBDIRS

