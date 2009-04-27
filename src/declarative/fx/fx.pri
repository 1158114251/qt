HEADERS += \
           fx/qfxanchors.h \
           fx/qfxanchors_p.h \
           fx/qfxanimatedimageitem.h \
           fx/qfxblendedimage.h \
           fx/qfxblurfilter.h \
           fx/qfxcomponentinstance.h \
           fx/qfxcomponentinstance_p.h \
           fx/qfxcontentwrapper.h \
           fx/qfxcontentwrapper_p.h \
           fx/qfxevents_p.h \
           fx/qfxflickable.h \
           fx/qfxflickable_p.h \
           fx/qfxfocuspanel.h \
           fx/qfxfocusrealm.h \
           fx/qfxgridview.h \
           fx/qfxhighlightfilter.h \
           fx/qfximage.h \
           fx/qfximageitem.h \
           fx/qfximageitem_p.h \
           fx/qfximage_p.h \
           fx/qfxitem.h \
           fx/qfxitem_p.h \
           fx/qfxkeyactions.h \
           fx/qfxkeyproxy.h \
           fx/qfxlayouts.h \
           fx/qfxlayouts_p.h \
           fx/qfxmouseregion.h \
           fx/qfxmouseregion_p.h \
           fx/qfxpainted.h \
           fx/qfxpainted_p.h \
           fx/qfxparticles.h \
           fx/qfxpath.h \
           fx/qfxpath_p.h \
           fx/qfxpathview.h \
           fx/qfxpathview_p.h \
           fx/qfxrect.h \
           fx/qfxrect_p.h \
           fx/qfxreflectionfilter.h \
           fx/qfxrepeater.h \
           fx/qfxrepeater_p.h \
           fx/qfxscalegrid.h \
           fx/qfxshadowfilter.h \
           fx/qfxtextedit.h \
           fx/qfxtextedit_p.h \
           fx/qfxtext.h \
           fx/qfxtext_p.h \
           fx/qfxtransform.h \
           fx/qfxpixmap.cpp \
           fx/qfxvisualitemmodel.h \
           fx/qfxlistview.h \
           fx/qfxwidgetcontainer.h \

SOURCES += \
           fx/qfxanchors.cpp \
           fx/qfxanimatedimageitem.cpp \
           fx/qfxblendedimage.cpp \
           fx/qfxblurfilter.cpp \
           fx/qfxcomponentinstance.cpp \
           fx/qfxcontentwrapper.cpp \
           fx/qfxevents.cpp \
           fx/qfxflickable.cpp \
           fx/qfxfocuspanel.cpp \
           fx/qfxfocusrealm.cpp \
           fx/qfxgridview.cpp \
           fx/qfxhighlightfilter.cpp \
           fx/qfximage.cpp \
           fx/qfximageitem.cpp \
           fx/qfxitem.cpp \
           fx/qfxkeyactions.cpp \
           fx/qfxkeyproxy.cpp \
           fx/qfxlayouts.cpp \
           fx/qfxmouseregion.cpp \
           fx/qfxpainted.cpp \
           fx/qfxparticles.cpp \
           fx/qfxpath.cpp \
           fx/qfxpathview.cpp \
           fx/qfxrect.cpp \
           fx/qfxreflectionfilter.cpp \
           fx/qfxrepeater.cpp \
           fx/qfxscalegrid.cpp \
           fx/qfxshadowfilter.cpp \
           fx/qfxtext.cpp \
           fx/qfxtextedit.cpp \
           fx/qfxtransform.cpp \
           fx/qfxpixmap.cpp \
           fx/qfxvisualitemmodel.cpp \
           fx/qfxlistview.cpp \
           fx/qfxwidgetcontainer.cpp \

contains(QT_CONFIG, webkit) {
    QT+=webkit
    SOURCES += fx/qfxwebview.cpp
    HEADERS += fx/qfxwebview.h
}

