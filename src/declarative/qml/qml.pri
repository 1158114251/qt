INCLUDEPATH += $$PWD

SOURCES += \
    $$PWD/qmlparser.cpp \
    $$PWD/qmlinstruction.cpp \
    $$PWD/qmlvmemetaobject.cpp \
    $$PWD/qmlengine.cpp \
    $$PWD/qmlexpression.cpp \
    $$PWD/qmlbinding.cpp \
    $$PWD/qmlmetaproperty.cpp \
    $$PWD/qmlcomponent.cpp \
    $$PWD/qmlcontext.cpp \
    $$PWD/qmlcustomparser.cpp \
    $$PWD/qmlpropertyvaluesource.cpp \
    $$PWD/qmlpropertyvalueinterceptor.cpp \
    $$PWD/qmlproxymetaobject.cpp \
    $$PWD/qmlvme.cpp \
    $$PWD/qmlcompiler.cpp \
    $$PWD/qmlcompileddata.cpp \
    $$PWD/qmlboundsignal.cpp \
    $$PWD/qmldom.cpp \
    $$PWD/qmlrefcount.cpp \
    $$PWD/qmlprivate.cpp \
    $$PWD/qmlmetatype.cpp \
    $$PWD/qmlstringconverters.cpp \
    $$PWD/qmlclassfactory.cpp \
    $$PWD/qmlparserstatus.cpp \
    $$PWD/qmlcompositetypemanager.cpp \
    $$PWD/qmlinfo.cpp \
    $$PWD/qmlerror.cpp \
    $$PWD/qmlscriptparser.cpp \
    $$PWD/qmlenginedebug.cpp \
    $$PWD/qmlrewrite.cpp \
    $$PWD/qmlbasicscript.cpp \
    $$PWD/qmlvaluetype.cpp \
    $$PWD/qmlbindingoptimizations.cpp \
    $$PWD/qmlxmlhttprequest.cpp \
    $$PWD/qmlsqldatabase.cpp \
    $$PWD/qmetaobjectbuilder.cpp \
    $$PWD/qmlwatcher.cpp \
    $$PWD/qmlscript.cpp \
    $$PWD/qmlcleanup.cpp \
    $$PWD/qmlpropertycache.cpp \
    $$PWD/qmlintegercache.cpp \
    $$PWD/qmltypenamecache.cpp \
    $$PWD/qmlscriptstring.cpp \
    $$PWD/qmlobjectscriptclass.cpp \
    $$PWD/qmlcontextscriptclass.cpp \
    $$PWD/qmlglobalscriptclass.cpp \
    $$PWD/qmlvaluetypescriptclass.cpp \
    $$PWD/qmltypenamescriptclass.cpp \
    $$PWD/qmllistscriptclass.cpp \
    $$PWD/qmlworkerscript.cpp

HEADERS += \
    $$PWD/qmlparser_p.h \
    $$PWD/qmlglobal_p.h \
    $$PWD/qmlinstruction_p.h \
    $$PWD/qmlvmemetaobject_p.h \
    $$PWD/qml.h \
    $$PWD/qmlbinding.h \
    $$PWD/qmlbinding_p.h \
    $$PWD/qmlmetaproperty.h \
    $$PWD/qmlcomponent.h \
    $$PWD/qmlcomponent_p.h \
    $$PWD/qmlcustomparser_p.h \
    $$PWD/qmlcustomparser_p_p.h \
    $$PWD/qmlpropertyvaluesource.h \
    $$PWD/qmlpropertyvalueinterceptor.h \
    $$PWD/qmlboundsignal_p.h \
    $$PWD/qmlparserstatus.h \
    $$PWD/qmlproxymetaobject_p.h \
    $$PWD/qmlvme_p.h \
    $$PWD/qmlcompiler_p.h \
    $$PWD/qmlengine_p.h \
    $$PWD/qmlexpression_p.h \
    $$PWD/qmlprivate.h \
    $$PWD/qmldom.h \
    $$PWD/qmldom_p.h \
    $$PWD/qmlrefcount_p.h \
    $$PWD/qmlmetatype.h \
    $$PWD/qmlengine.h \
    $$PWD/qmlcontext.h \
    $$PWD/qmlexpression.h \
    $$PWD/qmlstringconverters_p.h \
    $$PWD/qmlclassfactory_p.h \
    $$PWD/qmlinfo.h \
    $$PWD/qmlmetaproperty_p.h \
    $$PWD/qmlcontext_p.h \
    $$PWD/qmlcompositetypedata_p.h \
    $$PWD/qmlcompositetypemanager_p.h \
    $$PWD/qmllist.h \
    $$PWD/qmldeclarativedata_p.h \
    $$PWD/qmlerror.h \
    $$PWD/qmlscriptparser_p.h \
    $$PWD/qmlbasicscript_p.h \
    $$PWD/qmlenginedebug_p.h \
    $$PWD/qmlrewrite_p.h \
    $$PWD/qpodvector_p.h \
    $$PWD/qbitfield_p.h \
    $$PWD/qmlvaluetype_p.h \
    $$PWD/qmlbindingoptimizations_p.h \
    $$PWD/qmlxmlhttprequest_p.h \
    $$PWD/qmlsqldatabase_p.h \
    $$PWD/qmetaobjectbuilder_p.h \
    $$PWD/qmlwatcher_p.h \
    $$PWD/qmlcleanup_p.h \
    $$PWD/qmlpropertycache_p.h \
    $$PWD/qmlintegercache_p.h \
    $$PWD/qmltypenamecache_p.h \
    $$PWD/qmlscriptstring.h \
    $$PWD/qmlobjectscriptclass_p.h \
    $$PWD/qmlcontextscriptclass_p.h \
    $$PWD/qmlglobalscriptclass_p.h \
    $$PWD/qmlvaluetypescriptclass_p.h \
    $$PWD/qmltypenamescriptclass_p.h \
    $$PWD/qmllistscriptclass_p.h \
    $$PWD/qmlworkerscript_p.h

QT += sql

include(parser/parser.pri)
include(rewriter/rewriter.pri)

