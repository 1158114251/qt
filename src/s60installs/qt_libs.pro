# Use subdirs template to suppress generation of unnecessary files
TEMPLATE = subdirs

symbian: {
    load(data_caging_paths)

    SUBDIRS=
    TARGET = "QtLibs pre-release"
    contains(QT_EDITION, OpenSource) {
        # Can't use UID in protected range for self signed packages.
        TARGET.UID3 = 0xE001E61C
    } else {
        TARGET.UID3 = 0x2001E61C
    }
    VERSION=$${QT_MAJOR_VERSION}.$${QT_MINOR_VERSION}.$${QT_PATCH_VERSION}

    qtresources.sources = $${EPOCROOT}$$HW_ZDIR$$APP_RESOURCE_DIR/s60main.rsc 
    qtresources.path = $$APP_RESOURCE_DIR

    qtlibraries.sources = \ 
        QtCore.dll \
        QtXml.dll \
        QtGui.dll \
        QtNetwork.dll \
        QtScript.dll \
        QtTest.dll \
        QtSql.dll \
        qts60plugin_3_1.dll \
        qts60plugin_3_2.dll \
        qts60plugin_5_0.dll
        

    # TODO: This should be conditional in PKG file, see commented code below
    # However we don't yet have such mechanism in place
    contains(S60_VERSION, 3.1)|contains(S60_VERSION, 3.2)|contains(S60_VERSION, 5.0) {
        contains(CONFIG, system-sqlite): qtlibraries.sources += sqlite3.dll 
    }    
    
    #; EXISTS statement does not resolve !. Lets check the most common drives
    #IF NOT EXISTS("c:\sys\bin\sqlite3.dll") AND NOT EXISTS("e:\sys\bin\sqlite3.dll") AND NOT EXISTS("z:\sys\bin\sqlite3.dll")
    #"\Epoc32\release\armv5\UREL\sqlite3.dll"-"!:\sys\bin\sqlite3.dll"
    #ENDIF      

    qtlibraries.path = /sys/bin
    qtlibraries.depends = "(0x20013851), 1, 5, 1, {\"PIPS Installer\"}"
    contains(QT_CONFIG, openssl) | contains(QT_CONFIG, openssl-linked) {
        qtlibraries.depends += "(0x200110CB), 1, 5, 1, {\"Open C LIBSSL Common\"}"
    }
    contains(QT_CONFIG, stl) {
        qtlibraries.depends += "(0x2000F866), 1, 0, 0, {\"Standard C++ Library Common\"}"
    }
    
    !contains(QT_CONFIG, no-jpeg): imageformats_plugins.sources += qjpeg.dll
    !contains(QT_CONFIG, no-gif):  imageformats_plugins.sources += qgif.dll
    !contains(QT_CONFIG, no-mng):  imageformats_plugins.sources += qmng.dll
    !contains(QT_CONFIG, no-tiff): imageformats_plugins.sources += qtiff.dll
    !contains(QT_CONFIG, no-ico):  imageformats_plugins.sources += qico.dll
    imageformats_plugins.path = $$QT_PLUGINS_BASE_DIR/imageformats

    codecs_plugins.sources = qcncodecs.dll qjpcodecs.dll qtwcodecs.dll qkrcodecs.dll
    codecs_plugins.path = $$QT_PLUGINS_BASE_DIR/codecs
 
    DEPLOYMENT += qtresources qtlibraries imageformats_plugins codecs_plugins

    contains(QT_CONFIG, svg): {	
       qtlibraries.sources += QtSvg.dll
       imageformats_plugins.sources += qsvg.dll
       iconengines_plugins.sources = qsvgicon.dll
       iconengines_plugins.path = $$QT_PLUGINS_BASE_DIR/iconengines
       DEPLOYMENT += iconengines_plugins
    }

    contains(QT_CONFIG, phonon): {
    	qtlibraries.sources += QtPhonon.dll
    }    

}
