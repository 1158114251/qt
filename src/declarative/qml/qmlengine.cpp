/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the either Technology Preview License Agreement or the
** Beta Release License Agreement.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#undef QT3_SUPPORT // don't want it here - it just causes bugs (which is why we removed it)

#include <QMetaProperty>
#include <private/qmlengine_p.h>
#include <private/qmlcontext_p.h>
#include <private/qobject_p.h>
#include <private/qmlcompiler_p.h>

#ifdef QT_SCRIPTTOOLS_LIB
#include <QScriptEngineDebugger>
#endif

#include <QScriptClass>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QDesktopServices>
#include <QTimer>
#include <QList>
#include <QPair>
#include <QDebug>
#include <QMetaObject>
#include "qml.h"
#include <private/qfxperf_p.h>
#include <QStack>
#include "private/qmlbasicscript_p.h"
#include "qmlengine.h"
#include "qmlcontext.h"
#include "qmlexpression.h"
#include <QtCore/qthreadstorage.h>
#include <QtCore/qthread.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qdir.h>
#include <QtGui/qcolor.h>
#include <QtGui/qvector3d.h>
#include <qmlcomponent.h>
#include "private/qmlcomponentjs_p.h"
#include "private/qmlmetaproperty_p.h"
#include <private/qmlbinding_p.h>
#include <private/qmlvme_p.h>
#include <private/qmlenginedebug_p.h>
#include <private/qmlstringconverters_p.h>
#include <private/qmlxmlhttprequest_p.h>
#include <private/qmlsqldatabase_p.h>

#ifdef Q_OS_WIN // for %APPDATA%
#include "qt_windows.h"
#include "qlibrary.h"
#define CSIDL_APPDATA		0x001a	// <username>\Application Data
#endif

Q_DECLARE_METATYPE(QmlMetaProperty)
Q_DECLARE_METATYPE(QList<QObject *>);

QT_BEGIN_NAMESPACE

DEFINE_BOOL_CONFIG_OPTION(qmlDebugger, QML_DEBUGGER)
DEFINE_BOOL_CONFIG_OPTION(qmlImportTrace, QML_IMPORT_TRACE)

QML_DEFINE_TYPE(Qt,4,6,(QT_VERSION&0x00ff00)>>8,Object,QObject)

struct StaticQtMetaObject : public QObject
{
    static const QMetaObject *get()
        { return &static_cast<StaticQtMetaObject*> (0)->staticQtMetaObject; }
};

QScriptValue desktopOpenUrl(QScriptContext *ctxt, QScriptEngine *e)
{
    if(!ctxt->argumentCount())
        return e->newVariant(QVariant(false));
    bool ret = QDesktopServices::openUrl(QUrl(ctxt->argument(0).toString()));
    return e->newVariant(QVariant(ret));
}

// XXX Something like this should be exported by Qt.
static QString userLocalDataPath(const QString& app)
{
    QString result;

#ifdef Q_OS_WIN
#ifndef Q_OS_WINCE
    QLibrary library(QLatin1String("shell32"));
#else
    QLibrary library(QLatin1String("coredll"));
#endif // Q_OS_WINCE
    typedef BOOL (WINAPI*GetSpecialFolderPath)(HWND, LPWSTR, int, BOOL);
    GetSpecialFolderPath SHGetSpecialFolderPath = (GetSpecialFolderPath)library.resolve("SHGetSpecialFolderPathW");
    if (SHGetSpecialFolderPath) {
        wchar_t path[MAX_PATH];
        SHGetSpecialFolderPath(0, path, CSIDL_APPDATA, FALSE);
        result = QString::fromWCharArray(path);
    }
#endif // Q_OS_WIN

#ifdef Q_OS_MAC
    result = QLatin1String(qgetenv("HOME"));
    result += "/Library/Application Support";
#else
    if (result.isEmpty()) {
        // Fallback: UNIX style
        result = QLatin1String(qgetenv("XDG_DATA_HOME"));
        if (result.isEmpty()) {
            result = QLatin1String(qgetenv("HOME"));
            result += QLatin1String("/.local/share");
        }
    }
#endif

    result += QLatin1Char('/');
    result += app;
    return result;
}

QmlEnginePrivate::QmlEnginePrivate(QmlEngine *e)
: rootContext(0), currentExpression(0),
  isDebugging(false), contextClass(0), objectClass(0), valueTypeClass(0),
  nodeListClass(0), namedNodeMapClass(0), sqlQueryClass(0), scriptEngine(this), rootComponent(0),
  networkAccessManager(0), typeManager(e), uniqueId(1)
{
    QScriptValue qtObject =
        scriptEngine.newQMetaObject(StaticQtMetaObject::get());
    QScriptValue desktopObject = scriptEngine.newObject();
    desktopObject.setProperty(QLatin1String("openUrl"),scriptEngine.newFunction(desktopOpenUrl, 1));
    qtObject.setProperty(QLatin1String("DesktopServices"), desktopObject);
    scriptEngine.globalObject().setProperty(QLatin1String("Qt"), qtObject);

    offlineStoragePath = userLocalDataPath(QLatin1String("Nokia/Qt/QML/OfflineStorage"));
    qt_add_qmlxmlhttprequest(&scriptEngine);
    qt_add_qmlsqldatabase(&scriptEngine);

    //types
    qtObject.setProperty(QLatin1String("rgba"), scriptEngine.newFunction(QmlEnginePrivate::rgba, 4));
    qtObject.setProperty(QLatin1String("hsla"), scriptEngine.newFunction(QmlEnginePrivate::hsla, 4));
    qtObject.setProperty(QLatin1String("rect"), scriptEngine.newFunction(QmlEnginePrivate::rect, 4));
    qtObject.setProperty(QLatin1String("point"), scriptEngine.newFunction(QmlEnginePrivate::point, 2));
    qtObject.setProperty(QLatin1String("size"), scriptEngine.newFunction(QmlEnginePrivate::size, 2));
    qtObject.setProperty(QLatin1String("vector3d"), scriptEngine.newFunction(QmlEnginePrivate::vector, 3));

    //color helpers
    qtObject.setProperty(QLatin1String("lighter"), scriptEngine.newFunction(QmlEnginePrivate::lighter, 1));
    qtObject.setProperty(QLatin1String("darker"), scriptEngine.newFunction(QmlEnginePrivate::darker, 1));
    qtObject.setProperty(QLatin1String("tint"), scriptEngine.newFunction(QmlEnginePrivate::tint, 2));
}

QmlEnginePrivate::~QmlEnginePrivate()
{
    delete rootContext;
    rootContext = 0;
    delete contextClass;
    contextClass = 0;
    delete objectClass;
    objectClass = 0;
    delete valueTypeClass;
    valueTypeClass = 0;
    delete typeNameClass;
    typeNameClass = 0;
    delete networkAccessManager;
    networkAccessManager = 0;
    delete nodeListClass;
    nodeListClass = 0;
    delete namedNodeMapClass;
    namedNodeMapClass = 0;
    delete sqlQueryClass;
    sqlQueryClass = 0;

    for(int ii = 0; ii < bindValues.count(); ++ii)
        clear(bindValues[ii]);
    for(int ii = 0; ii < parserStatus.count(); ++ii)
        clear(parserStatus[ii]);
    for(QHash<int, QmlCompiledData*>::ConstIterator iter = m_compositeTypes.begin(); iter != m_compositeTypes.end(); ++iter) 
        (*iter)->release();
}

void QmlEnginePrivate::clear(SimpleList<QmlAbstractBinding> &bvs)
{
    bvs.clear();
}

void QmlEnginePrivate::clear(SimpleList<QmlParserStatus> &pss)
{
    for (int ii = 0; ii < pss.count; ++ii) {
        QmlParserStatus *ps = pss.at(ii);
        if(ps)
            ps->d = 0;
    }
    pss.clear();
}

Q_GLOBAL_STATIC(QmlEngineDebugServer, qmlEngineDebugServer);

void QmlEnginePrivate::init()
{
    Q_Q(QmlEngine);
    scriptEngine.installTranslatorFunctions();
    contextClass = new QmlContextScriptClass(q);
    objectClass = new QmlObjectScriptClass(q);
    valueTypeClass = new QmlValueTypeScriptClass(q);
    typeNameClass = new QmlTypeNameScriptClass(q);
    rootContext = new QmlContext(q,true);
#ifdef QT_SCRIPTTOOLS_LIB
    if (qmlDebugger()){
        debugger = new QScriptEngineDebugger(q);
        debugger->attachTo(&scriptEngine);
    }
#endif

    scriptEngine.globalObject().setProperty(QLatin1String("createQmlObject"),
            scriptEngine.newFunction(QmlEnginePrivate::createQmlObject, 1));
    scriptEngine.globalObject().setProperty(QLatin1String("createComponent"),
            scriptEngine.newFunction(QmlEnginePrivate::createComponent, 1));
    scriptEngine.globalObject().setProperty(QLatin1String("vector"),
            scriptEngine.newFunction(QmlEnginePrivate::vector, 3));

    if (QCoreApplication::instance()->thread() == q->thread() &&
        QmlEngineDebugServer::isDebuggingEnabled()) {
        qmlEngineDebugServer();
        isDebugging = true;
        QmlEngineDebugServer::addEngine(q);

        qmlEngineDebugServer()->waitForClients();
    }
}

QmlEnginePrivate::CapturedProperty::CapturedProperty(const QmlMetaProperty &p)
: object(p.object()), coreIndex(p.coreIndex()), notifyIndex(p.property().notifySignalIndex())
{
}

struct QmlTypeNameBridge
{
    QObject *object;
    QmlType *type;
    QmlEnginePrivate::ImportedNamespace *ns;
};
Q_DECLARE_METATYPE(QmlTypeNameBridge);

struct QmlValueTypeReference {
    QmlValueType *type;
    QGuard<QObject> object;
    int property;
};
Q_DECLARE_METATYPE(QmlValueTypeReference);

////////////////////////////////////////////////////////////////////
QScriptClass::QueryFlags 
QmlEnginePrivate::queryContext(const QString &propName, uint *id,
                               QmlContext *bindContext)
{
    resolveData.safetyCheckId++;
    *id = resolveData.safetyCheckId;
    resolveData.clear();

    QHash<QString, int>::Iterator contextProperty = 
        bindContext->d_func()->propertyNames.find(propName);

    if (contextProperty != bindContext->d_func()->propertyNames.end()) {

        resolveData.context = bindContext;
        resolveData.contextIndex = *contextProperty;

        return QScriptClass::HandlesReadAccess;
    } 

    QmlType *type = 0; ImportedNamespace *ns = 0;
    if (currentExpression && bindContext == currentExpression->context() && 
        propName.at(0).isUpper() && resolveType(bindContext->d_func()->imports, propName.toUtf8(), &type, 0, 0, 0, &ns)) {
        
        if (type || ns) {
            // Must be either an attached property, or an enum
            resolveData.object = bindContext->d_func()->defaultObjects.first();
            resolveData.type = type;
            resolveData.ns = ns;
            return QScriptClass::HandlesReadAccess;
        }

    } 

    QScriptClass::QueryFlags rv = 0;
    for (int ii = 0; !rv && ii < bindContext->d_func()->defaultObjects.count(); ++ii) {
        rv = queryObject(propName, id, 
                         bindContext->d_func()->defaultObjects.at(ii));
    }

    return rv;
}

QScriptValue QmlEnginePrivate::propertyContext(const QScriptString &name, uint id)
{
    Q_ASSERT(id == resolveData.safetyCheckId);

    if (resolveData.type || resolveData.ns) {
        QmlTypeNameBridge tnb = { 
            resolveData.object, 
            resolveData.type, 
            resolveData.ns 
        };
        return scriptEngine.newObject(typeNameClass, scriptEngine.newVariant(qVariantFromValue(tnb)));
    } else if (resolveData.context) {
        QmlContext *bindContext = resolveData.context;
        QmlContextPrivate *contextPrivate = bindContext->d_func();
        int index = resolveData.contextIndex;

        QScriptValue rv;
        if (index < contextPrivate->idValueCount) {
            rv = scriptEngine.newObject(objectClass, scriptEngine.newVariant(QVariant::fromValue(contextPrivate->idValues[index].data())));
        } else {
            QVariant value = contextPrivate->propertyValues.at(index);
            if (QmlMetaType::isObject(value.userType())) {
                rv = scriptEngine.newObject(objectClass, scriptEngine.newVariant(value));
            } else {
                rv = scriptEngine.newVariant(value);
            }
        }
        capturedProperties << QmlEnginePrivate::CapturedProperty(bindContext, -1, index + contextPrivate->notifyIndex);
        return rv;

    } else {

        return propertyObject(name, resolveData.object, id);

    }

    return QScriptValue();
}

void QmlEnginePrivate::setPropertyContext(const QScriptValue &value, uint id)
{
    // As context properties cannot be written, we can assume that the
    // write is a object property write
    setPropertyObject(value, id); 
}

void QmlEnginePrivate::setPropertyObject(const QScriptValue &value, uint id)
{
    Q_ASSERT(id == resolveData.safetyCheckId);
    Q_Q(QmlEngine);

    resolveData.property.write(QmlScriptClass::toVariant(q, value));
}

QScriptClass::QueryFlags
QmlEnginePrivate::queryObject(const QString &propName,
                              uint *id, QObject *obj)
{
    resolveData.safetyCheckId++;
    *id = resolveData.safetyCheckId;
    resolveData.clear();

    QScriptClass::QueryFlags rv = 0;

    QmlContext *ctxt = QmlEngine::contextForObject(obj);
    if (!ctxt)
        ctxt = rootContext;
    QmlMetaProperty prop(obj, propName, ctxt);

    if (prop.type() == QmlMetaProperty::Invalid) {
        QPair<const QMetaObject *, QString> key =
            qMakePair(obj->metaObject(), propName);
        bool isFunction = false;
        if (functionCache.contains(key)) {
            isFunction = functionCache.value(key);
        } else {
            QScriptValue sobj = scriptEngine.newQObject(obj);
            QScriptValue func = sobj.property(propName);
            isFunction = func.isFunction();
            functionCache.insert(key, isFunction);
        }

        if (isFunction) {
            resolveData.object = obj;
            resolveData.isFunction = true;
            rv |= QScriptClass::HandlesReadAccess;
        } 
    } else {
        resolveData.object = obj;
        resolveData.property = prop;

        rv |= QScriptClass::HandlesReadAccess;
        if (prop.isWritable())
            rv |= QScriptClass::HandlesWriteAccess;
    }

    return rv;
}

QScriptValue QmlEnginePrivate::propertyObject(const QScriptString &propName,
                                              QObject *obj, uint id)
{
    Q_ASSERT(id == resolveData.safetyCheckId);
    Q_ASSERT(resolveData.object);

    if (resolveData.isFunction) {
        // ### Optimize
        QScriptValue sobj = scriptEngine.newQObject(obj);
        QScriptValue func = sobj.property(propName);
        return func;
    } else {
        const QmlMetaProperty &prop = resolveData.property;

        if (prop.needsChangedNotifier())
            capturedProperties << CapturedProperty(prop);

        int propType = prop.propertyType();
        if (propType < QVariant::UserType && valueTypes[propType]) {
            QmlValueTypeReference ref;
            ref.type = valueTypes[propType];
            ref.object = obj;
            ref.property = prop.coreIndex();
            return scriptEngine.newObject(valueTypeClass, scriptEngine.newVariant(QVariant::fromValue(ref)));
        }

        QVariant var = prop.read();
        QObject *varobj = (propType < QVariant::UserType)?0:QmlMetaType::toQObject(var);
        if (!varobj)
            varobj = qvariant_cast<QObject *>(var);
        if (varobj) {
            return scriptEngine.newObject(objectClass, scriptEngine.newVariant(QVariant::fromValue(varobj)));
        } else {
            return qScriptValueFromValue(&scriptEngine, var);
        }
    }

    return QScriptValue();
}

/*!
    \class QmlEngine
    \brief The QmlEngine class provides an environment for instantiating QML components.
    \mainclass

    Each QML component is instantiated in a QmlContext.  QmlContext's are
    essential for passing data to QML components.  In QML, contexts are arranged
    hierarchically and this hierarchy is managed by the QmlEngine.

    Prior to creating any QML components, an application must have created a
    QmlEngine to gain access to a QML context.  The following example shows how
    to create a simple Text item.

    \code
    QmlEngine engine;
    QmlComponent component(&engine, "Text { text: \"Hello world!\" }");
    QFxItem *item = qobject_cast<QFxItem *>(component.create());

    //add item to view, etc
    ...
    \endcode

    In this case, the Text item will be created in the engine's
    \l {QmlEngine::rootContext()}{root context}.

    \sa QmlComponent QmlContext
*/

/*!
    Create a new QmlEngine with the given \a parent.
*/
QmlEngine::QmlEngine(QObject *parent)
: QObject(*new QmlEnginePrivate(this), parent)
{
    Q_D(QmlEngine);
    d->init();

    qRegisterMetaType<QVariant>("QVariant");
}

/*!
    Destroys the QmlEngine.

    Any QmlContext's created on this engine will be invalidated, but not
    destroyed (unless they are parented to the QmlEngine object).
*/
QmlEngine::~QmlEngine()
{
    Q_D(QmlEngine);
    if (d->isDebugging)
        QmlEngineDebugServer::remEngine(this);
}

/*!
  Clears the engine's internal component cache.

  Normally the QmlEngine caches components loaded from qml files.  This method
  clears this cache and forces the component to be reloaded.
 */
void QmlEngine::clearComponentCache()
{
    Q_D(QmlEngine);
    d->typeManager.clearCache();
}

/*!
    Returns the engine's root context.

    The root context is automatically created by the QmlEngine.  Data that
    should be available to all QML component instances instantiated by the
    engine should be put in the root context.

    Additional data that should only be available to a subset of component
    instances should be added to sub-contexts parented to the root context.
*/
QmlContext *QmlEngine::rootContext()
{
    Q_D(QmlEngine);
    return d->rootContext;
}

/*!
    Sets the common QNetworkAccessManager, \a network, used by all QML elements
    instantiated by this engine.

    Any previously set manager is deleted and \a network is owned by the
    QmlEngine.  This method should only be called before any QmlComponents are
    instantiated.
*/
void QmlEngine::setNetworkAccessManager(QNetworkAccessManager *network)
{
    Q_D(QmlEngine);
    delete d->networkAccessManager;
    d->networkAccessManager = network;
}

/*!
    Returns the common QNetworkAccessManager used by all QML elements
    instantiated by this engine.

    The default implements no caching, cookiejar, etc., just a default
    QNetworkAccessManager.
*/
QNetworkAccessManager *QmlEngine::networkAccessManager() const
{
    Q_D(const QmlEngine);
    if (!d->networkAccessManager)
        d->networkAccessManager = new QNetworkAccessManager;
    return d->networkAccessManager;
}

/*!
    Return the base URL for this engine.  The base URL is only used to resolve
    components when a relative URL is passed to the QmlComponent constructor.

    If a base URL has not been explicitly set, this method returns the
    application's current working directory.

    \sa setBaseUrl()
*/
QUrl QmlEngine::baseUrl() const
{
    Q_D(const QmlEngine);
    if (d->baseUrl.isEmpty()) {
        return QUrl::fromLocalFile(QDir::currentPath() + QDir::separator());
    } else {
        return d->baseUrl;
    }
}

/*!
    Set the  base URL for this engine to \a url.

    \sa baseUrl()
*/
void QmlEngine::setBaseUrl(const QUrl &url)
{
    Q_D(QmlEngine);
    d->baseUrl = url;
}

/*!
  Returns the QmlContext for the \a object, or 0 if no context has been set.

  When the QmlEngine instantiates a QObject, the context is set automatically.
  */
QmlContext *QmlEngine::contextForObject(const QObject *object)
{
    if(!object)
        return 0;

    QObjectPrivate *priv = QObjectPrivate::get(const_cast<QObject *>(object));

    QmlDeclarativeData *data =
        static_cast<QmlDeclarativeData *>(priv->declarativeData);

    return data?data->context:0;
}

/*!
  Sets the QmlContext for the \a object to \a context.
  If the \a object already has a context, a warning is
  output, but the context is not changed.

  When the QmlEngine instantiates a QObject, the context is set automatically.
 */
void QmlEngine::setContextForObject(QObject *object, QmlContext *context)
{
    QObjectPrivate *priv = QObjectPrivate::get(object);

    QmlDeclarativeData *data =
        static_cast<QmlDeclarativeData *>(priv->declarativeData);

    if (data && data->context) {
        qWarning("QmlEngine::setContextForObject(): Object already has a QmlContext");
        return;
    }

    if (!data) {
        priv->declarativeData = new QmlDeclarativeData(context);
    } else {
        data->context = context;
    }

    context->d_func()->contextObjects.append(object);
}

void qmlExecuteDeferred(QObject *object)
{
    QmlDeclarativeData *data = QmlDeclarativeData::get(object);

    if (data && data->deferredComponent) {
        QmlVME vme;
        vme.runDeferred(object);

        data->deferredComponent->release();
        data->deferredComponent = 0;
    }
}

QmlContext *qmlContext(const QObject *obj)
{
    return QmlEngine::contextForObject(obj);
}

QmlEngine *qmlEngine(const QObject *obj)
{
    QmlContext *context = QmlEngine::contextForObject(obj);
    return context?context->engine():0;
}

QObject *qmlAttachedPropertiesObjectById(int id, const QObject *object, bool create)
{
    QmlDeclarativeData *data = QmlDeclarativeData::get(object);

    QObject *rv = data->attachedProperties?data->attachedProperties->value(id):0;
    if (rv || !create)
        return rv;

    QmlAttachedPropertiesFunc pf = QmlMetaType::attachedPropertiesFuncById(id);
    if (!pf)
        return 0;

    rv = pf(const_cast<QObject *>(object));

    if (rv) {
        if (!data->attachedProperties)
            data->attachedProperties = new QHash<int, QObject *>();
        data->attachedProperties->insert(id, rv);
    }

    return rv;
}

QmlDeclarativeData::QmlDeclarativeData(QmlContext *ctxt)
: context(ctxt), bindings(0), outerContext(0), lineNumber(0), columnNumber(0), deferredComponent(0),
  deferredIdx(0), attachedProperties(0)
{
}

void QmlDeclarativeData::destroyed(QObject *object)
{
    if (deferredComponent)
        deferredComponent->release();
    if (attachedProperties)
        delete attachedProperties;
    if (context)
        static_cast<QmlContextPrivate *>(QObjectPrivate::get(context))->contextObjects.removeAll(object);

    QmlAbstractBinding *binding = bindings;
    while (binding) {
        QmlAbstractBinding *next = binding->m_nextBinding;
        binding->m_prevBinding = 0;
        binding->m_nextBinding = 0;
        delete binding;
        binding = next;
    }

    delete this;
}

/*!
    Creates a QScriptValue allowing you to use \a object in QML script.
    \a engine is the QmlEngine it is to be created in.

    The QScriptValue returned is a QtScript Object, not a QtScript QObject, due
    to the special needs of QML requiring more functionality than a standard
    QtScript QObject.
*/
QScriptValue QmlEnginePrivate::qmlScriptObject(QObject* object,
                                               QmlEngine* engine)
{
    QScriptEngine *scriptEngine = QmlEnginePrivate::getScriptEngine(engine);
    return scriptEngine->newObject(engine->d_func()->objectClass, scriptEngine->newQObject(object, QScriptEngine::AutoOwnership));
}

/*!
    This function is intended for use inside QML only. In C++ just create a
    component object as usual.

    This function takes the URL of a QML file as its only argument. It returns
    a component object which can be used to create and load that QML file.

    Example QmlJS is below, remember that QML files that might be loaded
    over the network cannot be expected to be ready immediately.
    \code
        var component;
        var sprite;
        function finishCreation(){
            if(component.isReady()){
                sprite = component.createObject();
                if(sprite == 0){
                    // Error Handling
                }else{
                    sprite.parent = page;
                    sprite.x = 200;
                    //...
                }
            }else if(component.isError()){
                // Error Handling
            }
        }

        component = createComponent("Sprite.qml");
        if(component.isReady()){
            finishCreation();
        }else{
            component.statusChanged.connect(finishCreation);
        }
    \endcode

    If you are certain the files will be local, you could simplify to

    \code
        component = createComponent("Sprite.qml");
        sprite = component.createObject();
        if(sprite == 0){
            // Error Handling
            print(component.errorsString());
        }else{
            sprite.parent = page;
            sprite.x = 200;
            //...
        }
    \endcode

    If you want to just create an arbitrary string of QML, instead of
    loading a qml file, consider the createQmlObject() function.
*/
QScriptValue QmlEnginePrivate::createComponent(QScriptContext *ctxt,
                                               QScriptEngine *engine)
{
    QmlComponentJS* c;

    QmlEnginePrivate *activeEnginePriv =
        static_cast<QmlScriptEngine*>(engine)->p;
    QmlEngine* activeEngine = activeEnginePriv->q_func();

    QmlContext* context = activeEnginePriv->currentExpression->context();
    if(ctxt->argumentCount() != 1) {
        c = new QmlComponentJS(activeEngine);
    }else{
        QUrl url = QUrl(context->resolvedUrl(ctxt->argument(0).toString()));
        if(!url.isValid())
            url = QUrl(ctxt->argument(0).toString());
        c = new QmlComponentJS(activeEngine, url, activeEngine);
    }
    c->setContext(context);
    return engine->newQObject(c);
}

/*!
    Creates a new object from the specified string of QML. It requires a
    second argument, which is the id of an existing QML object to use as
    the new object's parent. If a third argument is provided, this is used
    as the filepath that the qml came from.

    Example (where targetItem is the id of an existing QML item):
    \code
    newObject = createQmlObject('Rectangle {color: "red"; width: 20; height: 20}',
        targetItem, "dynamicSnippet1");
    \endcode

    This function is intended for use inside QML only. It is intended to behave
    similarly to eval, but for creating QML elements.

    Returns the created object, or null if there is an error. In the case of an
    error, details of the error are output using qWarning().

    Note that this function returns immediately, and therefore may not work if
    the QML loads new components. If you are trying to load a new component,
    for example from a QML file, consider the createComponent() function
    instead. 'New components' refers to external QML files that have not yet
    been loaded, and so it is safe to use createQmlObject to load built-in
    components.
*/
QScriptValue QmlEnginePrivate::createQmlObject(QScriptContext *ctxt, QScriptEngine *engine)
{
    QmlEnginePrivate *activeEnginePriv =
        static_cast<QmlScriptEngine*>(engine)->p;
    QmlEngine* activeEngine = activeEnginePriv->q_func();

    if(ctxt->argumentCount() < 2)
        return engine->nullValue();

    QString qml = ctxt->argument(0).toString();
    QUrl url;
    if(ctxt->argumentCount() > 2)
        url = QUrl(ctxt->argument(2).toString());
    QObject *parentArg = ctxt->argument(1).data().toQObject();
    QmlContext *qmlCtxt = qmlContext(parentArg);
    url = qmlCtxt->resolvedUrl(url);
    QmlComponent component(activeEngine, qml.toUtf8(), url);
    if(component.isError()) {
        QList<QmlError> errors = component.errors();
        qWarning() <<"QmlEngine::createQmlObject():";
        foreach (const QmlError &error, errors)
            qWarning() << "    " << error;

        return engine->nullValue();
    }

    QObject *obj = component.create(qmlCtxt);

    if(component.isError()) {
        QList<QmlError> errors = component.errors();
        qWarning() <<"QmlEngine::createQmlObject():";
        foreach (const QmlError &error, errors)
            qWarning() << "    " << error;

        return engine->nullValue();
    }

    if(obj) {
        obj->setParent(parentArg);
        obj->setProperty("parent", QVariant::fromValue<QObject*>(parentArg));
        return qmlScriptObject(obj, activeEngine);
    }
    return engine->nullValue();
}

/*!
    This function is intended for use inside QML only. In C++ just create a
    QVector3D as usual.

    This function takes three numeric components and combines them into a
    QVector3D value that can be used with any property that takes a
    QVector3D argument.  The following QML code:

    \code
    transform: Rotation {
        id: Rotation
        origin.x: Container.width / 2;
        axis: vector(0, 1, 1)
    }
    \endcode

    is equivalent to:

    \code
    transform: Rotation {
        id: Rotation
        origin.x: Container.width / 2;
        axis.x: 0; axis.y: 1; axis.z: 0
    }
    \endcode
*/
QScriptValue QmlEnginePrivate::vector(QScriptContext *ctxt, QScriptEngine *engine)
{
    if(ctxt->argumentCount() < 3)
        return engine->nullValue();
    qsreal x = ctxt->argument(0).toNumber();
    qsreal y = ctxt->argument(1).toNumber();
    qsreal z = ctxt->argument(2).toNumber();
    return engine->newVariant(qVariantFromValue(QVector3D(x, y, z)));
}

QScriptValue QmlEnginePrivate::rgba(QScriptContext *ctxt, QScriptEngine *engine)
{
    int argCount = ctxt->argumentCount();
    if(argCount < 3)
        return engine->nullValue();
    qsreal r = ctxt->argument(0).toNumber();
    qsreal g = ctxt->argument(1).toNumber();
    qsreal b = ctxt->argument(2).toNumber();
    qsreal a = (argCount == 4) ? ctxt->argument(3).toNumber() : 1;
    return qScriptValueFromValue(engine, qVariantFromValue(QColor::fromRgbF(r, g, b, a)));
}

QScriptValue QmlEnginePrivate::hsla(QScriptContext *ctxt, QScriptEngine *engine)
{
    int argCount = ctxt->argumentCount();
    if(argCount < 3)
        return engine->nullValue();
    qsreal h = ctxt->argument(0).toNumber();
    qsreal s = ctxt->argument(1).toNumber();
    qsreal l = ctxt->argument(2).toNumber();
    qsreal a = (argCount == 4) ? ctxt->argument(3).toNumber() : 1;
    return qScriptValueFromValue(engine, qVariantFromValue(QColor::fromHslF(h, s, l, a)));
}

QScriptValue QmlEnginePrivate::rect(QScriptContext *ctxt, QScriptEngine *engine)
{
    if(ctxt->argumentCount() < 4)
        return engine->nullValue();
    qsreal x = ctxt->argument(0).toNumber();
    qsreal y = ctxt->argument(1).toNumber();
    qsreal w = ctxt->argument(2).toNumber();
    qsreal h = ctxt->argument(3).toNumber();
    return qScriptValueFromValue(engine, qVariantFromValue(QRectF(x, y, w, h)));
}

QScriptValue QmlEnginePrivate::point(QScriptContext *ctxt, QScriptEngine *engine)
{
    if(ctxt->argumentCount() < 2)
        return engine->nullValue();
    qsreal x = ctxt->argument(0).toNumber();
    qsreal y = ctxt->argument(1).toNumber();
    return qScriptValueFromValue(engine, qVariantFromValue(QPointF(x, y)));
}

QScriptValue QmlEnginePrivate::size(QScriptContext *ctxt, QScriptEngine *engine)
{
    if(ctxt->argumentCount() < 2)
        return engine->nullValue();
    qsreal w = ctxt->argument(0).toNumber();
    qsreal h = ctxt->argument(1).toNumber();
    return qScriptValueFromValue(engine, qVariantFromValue(QSizeF(w, h)));
}

QScriptValue QmlEnginePrivate::lighter(QScriptContext *ctxt, QScriptEngine *engine)
{
    if(ctxt->argumentCount() < 1)
        return engine->nullValue();
    QVariant v = ctxt->argument(0).toVariant();
    QColor color;
    if (v.type() == QVariant::Color)
        color = v.value<QColor>();
    else if (v.type() == QVariant::String) {
        bool ok;
        color = QmlStringConverters::colorFromString(v.toString(), &ok);
        if (!ok)
            return engine->nullValue();
    } else
        return engine->nullValue();
    color = color.lighter();
    return qScriptValueFromValue(engine, qVariantFromValue(color));
}

QScriptValue QmlEnginePrivate::darker(QScriptContext *ctxt, QScriptEngine *engine)
{
    if(ctxt->argumentCount() < 1)
        return engine->nullValue();
    QVariant v = ctxt->argument(0).toVariant();
    QColor color;
    if (v.type() == QVariant::Color)
        color = v.value<QColor>();
    else if (v.type() == QVariant::String) {
        bool ok;
        color = QmlStringConverters::colorFromString(v.toString(), &ok);
        if (!ok)
            return engine->nullValue();
    } else
        return engine->nullValue();
    color = color.darker();
    return qScriptValueFromValue(engine, qVariantFromValue(color));
}

/*!
    This function allows tinting one color with another.

    The tint color should usually be mostly transparent, or you will not be able to see the underlying color. The below example provides a slight red tint by having the tint color be pure red which is only 1/16th opaque.

    \qml
    Rectangle { x: 0; width: 80; height: 80; color: "lightsteelblue" }
    Rectangle { x: 100; width: 80; height: 80; color: Qt.tint("lightsteelblue", "#10FF0000") }
    \endqml
    \image declarative-rect_tint.png

    Tint is most useful when a subtle change is intended to be conveyed due to some event; you can then use tinting to more effectively tune the visible color.
*/
QScriptValue QmlEnginePrivate::tint(QScriptContext *ctxt, QScriptEngine *engine)
{
    if(ctxt->argumentCount() < 2)
        return engine->nullValue();
    //get color
    QVariant v = ctxt->argument(0).toVariant();
    QColor color;
    if (v.type() == QVariant::Color)
        color = v.value<QColor>();
    else if (v.type() == QVariant::String) {
        bool ok;
        color = QmlStringConverters::colorFromString(v.toString(), &ok);
        if (!ok)
            return engine->nullValue();
    } else
        return engine->nullValue();

    //get tint color
    v = ctxt->argument(1).toVariant();
    QColor tintColor;
    if (v.type() == QVariant::Color)
        tintColor = v.value<QColor>();
    else if (v.type() == QVariant::String) {
        bool ok;
        tintColor = QmlStringConverters::colorFromString(v.toString(), &ok);
        if (!ok)
            return engine->nullValue();
    } else
        return engine->nullValue();

    //tint
    QColor finalColor;
    int a = tintColor.alpha();
    if (a == 0xFF)
        finalColor = tintColor;
    else if (a == 0x00)
        finalColor = color;
    else {
        uint src = tintColor.rgba();
        uint dest = color.rgba();

        uint res = (((a * (src & 0xFF00FF)) +
                    ((0xFF - a) * (dest & 0xFF00FF))) >> 8) & 0xFF00FF;
        res |= (((a * ((src >> 8) & 0xFF00FF)) +
                ((0xFF - a) * ((dest >> 8) & 0xFF00FF)))) & 0xFF00FF00;
        if ((src & 0xFF000000) == 0xFF000000)
            res |= 0xFF000000;

        finalColor = QColor::fromRgba(res);
    }

    return qScriptValueFromValue(engine, qVariantFromValue(finalColor));
}

QmlScriptClass::QmlScriptClass(QmlEngine *bindengine)
: QScriptClass(QmlEnginePrivate::getScriptEngine(bindengine)),
  engine(bindengine)
{
}

QVariant QmlScriptClass::toVariant(QmlEngine *engine, const QScriptValue &val)
{
    QmlEnginePrivate *ep =
        static_cast<QmlEnginePrivate *>(QObjectPrivate::get(engine));

    QScriptClass *sc = val.scriptClass();
    if (!sc) {
        return val.toVariant();
    } else if (sc == ep->contextClass) {
        return QVariant();
    } else if (sc == ep->objectClass) {
        return QVariant::fromValue(val.data().toQObject());
    } else if (sc == ep->valueTypeClass) {
        QmlValueTypeReference ref =
            qvariant_cast<QmlValueTypeReference>(val.data().toVariant());

        if (!ref.object)
            return QVariant();

        QMetaProperty p = ref.object->metaObject()->property(ref.property);
        return p.read(ref.object);
    }

    return QVariant();
}

/////////////////////////////////////////////////////////////
/*
    The QmlContextScriptClass handles property access for a QmlContext
    via QtScript.
 */
QmlContextScriptClass::QmlContextScriptClass(QmlEngine *bindEngine)
    : QmlScriptClass(bindEngine)
{
}

QmlContextScriptClass::~QmlContextScriptClass()
{
}

QScriptClass::QueryFlags
QmlContextScriptClass::queryProperty(const QScriptValue &object,
                                         const QScriptString &name,
                                         QueryFlags flags, uint *id)
{
    Q_UNUSED(flags);
    QmlContext *bindContext =
        static_cast<QmlContext*>(object.data().toQObject());

    QString propName = name.toString();

    QmlEnginePrivate *ep = QmlEnginePrivate::get(engine);
    return ep->queryContext(propName, id, bindContext);
}

QScriptValue QmlContextScriptClass::property(const QScriptValue &object,
                                             const QScriptString &name,
                                             uint id)
{
    Q_UNUSED(object);

    QmlEnginePrivate *ep = QmlEnginePrivate::get(engine);
    return ep->propertyContext(name, id);
}

void QmlContextScriptClass::setProperty(QScriptValue &object,
                                            const QScriptString &name,
                                            uint id,
                                            const QScriptValue &value)
{
    Q_UNUSED(object);
    Q_UNUSED(name);

    QmlEnginePrivate::get(engine)->setPropertyContext(value, id);
}

/////////////////////////////////////////////////////////////
QmlTypeNameScriptClass::QmlTypeNameScriptClass(QmlEngine *engine)
: QmlScriptClass(engine), object(0), type(0)
{
}

QmlTypeNameScriptClass::~QmlTypeNameScriptClass()
{
}

QmlTypeNameScriptClass::QueryFlags 
QmlTypeNameScriptClass::queryProperty(const QScriptValue &scriptObject,
                                      const QScriptString &name,
                                      QueryFlags flags, uint *id)
{
    Q_UNUSED(flags);

    QmlTypeNameBridge bridge = 
        qvariant_cast<QmlTypeNameBridge>(scriptObject.data().toVariant());

    object = 0;
    type = 0;
    QmlEnginePrivate *ep = QmlEnginePrivate::get(engine);

    if (bridge.ns) {
        QmlType *type = 0;
        ep->resolveTypeInNamespace(bridge.ns, name.toString().toUtf8(),
                                   &type, 0, 0, 0);
        if (type) {
            object = bridge.object;
            this->type = type;
            return HandlesReadAccess;
        } else {
            return 0;
        }

    } else {
        Q_ASSERT(bridge.type);
        QString strName = name.toString();
        if (strName.at(0).isUpper()) {
            // Must be an enum
            // ### Optimize
            const char *enumName = strName.toUtf8().constData();
            const QMetaObject *metaObject = bridge.type->baseMetaObject();
            for (int ii = metaObject->enumeratorCount() - 1; ii >= 0; --ii) {
                QMetaEnum e = metaObject->enumerator(ii);
                int value = e.keyToValue(enumName);
                if (value != -1) {
                    enumValue = value;
                    return HandlesReadAccess;
                }
            }
            return 0;
        } else {
            // Must be an attached property
            this->object = qmlAttachedPropertiesObjectById(bridge.type->index(), bridge.object);
            Q_ASSERT(this->object);
            return ep->queryObject(strName, id, this->object);
        }
    }
}

QScriptValue QmlTypeNameScriptClass::property(const QScriptValue &,
                                              const QScriptString &propName, 
                                              uint id)
{
    QmlEnginePrivate *ep = QmlEnginePrivate::get(engine);
    if (type) {
        QmlTypeNameBridge tnb = { object, type, 0 };
        return ep->scriptEngine.newObject(ep->typeNameClass, ep->scriptEngine.newVariant(qVariantFromValue(tnb)));
    } else if (object) {
        return ep->propertyObject(propName, object, id);
    } else {
        return QScriptValue(enumValue);
    }
}

/////////////////////////////////////////////////////////////
QmlValueTypeScriptClass::QmlValueTypeScriptClass(QmlEngine *bindEngine)
: QmlScriptClass(bindEngine)
{
}

QmlValueTypeScriptClass::~QmlValueTypeScriptClass()
{
}

QmlValueTypeScriptClass::QueryFlags
QmlValueTypeScriptClass::queryProperty(const QScriptValue &object,
                                       const QScriptString &name,
                                       QueryFlags flags, uint *id)
{
    Q_UNUSED(flags);
    QmlValueTypeReference ref =
        qvariant_cast<QmlValueTypeReference>(object.data().toVariant());

    if (!ref.object)
        return 0;

    QByteArray propName = name.toString().toUtf8();

    int idx = ref.type->metaObject()->indexOfProperty(propName.constData());
    if (idx == -1)
        return 0;
    *id = idx;

    QMetaProperty prop = ref.object->metaObject()->property(idx);

    QmlValueTypeScriptClass::QueryFlags rv =
        QmlValueTypeScriptClass::HandlesReadAccess;
    if (prop.isWritable())
        rv |= QmlValueTypeScriptClass::HandlesWriteAccess;

    return rv;
}

QScriptValue QmlValueTypeScriptClass::property(const QScriptValue &object,
                                               const QScriptString &name,
                                               uint id)
{
    Q_UNUSED(name);
    QmlValueTypeReference ref =
        qvariant_cast<QmlValueTypeReference>(object.data().toVariant());

    if (!ref.object)
        return QScriptValue();

    ref.type->read(ref.object, ref.property);

    QMetaProperty p = ref.type->metaObject()->property(id);
    QVariant rv = p.read(ref.type);

    return static_cast<QmlEnginePrivate *>(QObjectPrivate::get(engine))->scriptEngine.newVariant(rv);
}

void QmlValueTypeScriptClass::setProperty(QScriptValue &object,
                                          const QScriptString &name,
                                          uint id,
                                          const QScriptValue &value)
{
    Q_UNUSED(name);
    QmlValueTypeReference ref =
        qvariant_cast<QmlValueTypeReference>(object.data().toVariant());

    if (!ref.object)
        return;

    QVariant v = QmlScriptClass::toVariant(engine, value);

    ref.type->read(ref.object, ref.property);
    QMetaProperty p = ref.type->metaObject()->property(id);
    p.write(ref.type, v);
    ref.type->write(ref.object, ref.property);
}

/////////////////////////////////////////////////////////////
/*
    The QmlObjectScriptClass handles property access for QObjects
    via QtScript. It is also used to provide a more useful API in
    QtScript for QML.
 */

QScriptValue QmlObjectToString(QScriptContext *context, QScriptEngine *engine)
{
    QObject* obj = context->thisObject().data().toQObject();
    QString ret = QLatin1String("Qml Object, ");
    if(obj){
        //###Should this be designer or developer details? Dev for now.
        //TODO: Can we print the id too?
        ret += QLatin1String("\"");
        ret += obj->objectName();
        ret += QLatin1String("\" ");
        ret += QLatin1String(obj->metaObject()->className());
        ret += QLatin1String("(0x");
        ret += QString::number((quintptr)obj,16);
        ret += QLatin1String(")");
    }else{
        ret += QLatin1String("null");
    }
    return engine->newVariant(ret);
}

QScriptValue QmlObjectDestroy(QScriptContext *context, QScriptEngine *engine)
{
    QObject* obj = context->thisObject().data().toQObject();
    if(obj){
        int delay = 0;
        if(context->argumentCount() > 0)
            delay = context->argument(0).toInt32();
        QTimer::singleShot(delay, obj, SLOT(deleteLater()));
        //### Should this be delayed as well?
        context->thisObject().setData(QScriptValue(engine, 0));
    }
    return engine->nullValue();
}

QmlObjectScriptClass::QmlObjectScriptClass(QmlEngine *bindEngine)
    : QmlScriptClass(bindEngine)
{
    engine = bindEngine;
    QScriptEngine *scriptEngine = QmlEnginePrivate::getScriptEngine(bindEngine);
    prototypeObject = scriptEngine->newObject();
    prototypeObject.setProperty(QLatin1String("toStr"),//TODO: Why won't toString work?
                                scriptEngine->newFunction(QmlObjectToString));
    prototypeObject.setProperty(QLatin1String("destroy"),
                                scriptEngine->newFunction(QmlObjectDestroy));
}

QmlObjectScriptClass::~QmlObjectScriptClass()
{
}

QScriptValue QmlObjectScriptClass::prototype() const
{
    return prototypeObject;
}

QScriptClass::QueryFlags QmlObjectScriptClass::queryProperty(const QScriptValue &object,
                                    const QScriptString &name,
                                    QueryFlags flags, uint *id)
{
    Q_UNUSED(flags);
    QObject *obj = object.data().toQObject();
    QueryFlags rv = 0;
    QString propName = name.toString();

    if (obj)
        rv = QmlEnginePrivate::get(engine)->queryObject(propName, id, obj);

    return rv;
}

QScriptValue QmlObjectScriptClass::property(const QScriptValue &object,
                                const QScriptString &name,
                                uint id)
{
    QObject *obj = object.data().toQObject();

    QScriptValue rv =
        QmlEnginePrivate::get(engine)->propertyObject(name, obj, id);
    if (rv.isValid()) 
        return rv;

    return QScriptValue();
}

void QmlObjectScriptClass::setProperty(QScriptValue &object,
                                       const QScriptString &name,
                                       uint id,
                                       const QScriptValue &value)
{
    Q_UNUSED(name);
    Q_UNUSED(object);
    QmlEnginePrivate::get(engine)->setPropertyObject(value, id);
}


struct QmlEnginePrivate::ImportedNamespace {
    QStringList urls;
    QList<int> majversions;
    QList<int> minversions;
    QList<bool> isLibrary;
    QList<bool> isBuiltin;

    bool find(const QByteArray& type, int *vmajor, int *vminor, QmlType** type_return, QUrl* url_return) const
    {
        for (int i=0; i<urls.count(); ++i) {
            int vmaj = majversions.at(i);
            int vmin = minversions.at(i);

            if (isBuiltin.at(i)) {
                QByteArray qt = urls.at(i).toLatin1();
                qt += "/";
                qt += type;
                QmlType *t = QmlMetaType::qmlType(qt,vmaj,vmin);
                if (vmajor) *vmajor = vmaj;
                if (vminor) *vminor = vmin;
                if (t) {
                    if (type_return)
                        *type_return = t;
                    return true;
                }
            } else {
                QUrl url = QUrl(urls.at(i) + QLatin1String("/" + type + ".qml"));
                if (vmaj || vmin) {
                    // Check version file - XXX cache these in QmlEngine!
                    QFile qmldir(QUrl(urls.at(i)+QLatin1String("/qmldir")).toLocalFile());
                    if (qmldir.open(QIODevice::ReadOnly)) {
                        do {
                            QByteArray lineba = qmldir.readLine();
                            if (lineba.at(0) == '#')
                                continue;
                            int space1 = lineba.indexOf(' ');
                            if (qstrncmp(lineba,type,space1)==0) {
                                // eg. 1.2-5
                                QString line = QString::fromUtf8(lineba);
                                space1 = line.indexOf(QLatin1Char(' ')); // refind in Unicode
                                int space2 = space1 >=0 ? line.indexOf(QLatin1Char(' '),space1+1) : -1;
                                QString mapversions = line.mid(space1+1,space2<0?line.length()-space1-2:space2-space1-1);
                                int dot = mapversions.indexOf(QLatin1Char('.'));
                                int dash = mapversions.indexOf(QLatin1Char('-'));
                                int mapvmaj = mapversions.left(dot).toInt();
                                if (mapvmaj==vmaj) {
                                    int mapvmin_from = (dash <= 0 ? mapversions.mid(dot+1) : mapversions.mid(dot+1,dash-dot-1)).toInt();
                                    int mapvmin_to = dash <= 0 ? mapvmin_from : mapversions.mid(dash+1).toInt();
                                    if (vmin >= mapvmin_from && vmin <= mapvmin_to) {
                                        QStringRef mapfile = space2<0 ? QStringRef() : line.midRef(space2+1,line.length()-space2-2);
                                        if (url_return)
                                            *url_return = url.resolved(mapfile.toString());
                                        return true;
                                    }
                                }
                            }
                        } while (!qmldir.atEnd());
                    }
                } else {
                    // XXX search non-files too! (eg. zip files, see QT-524)
                    QFileInfo f(url.toLocalFile());
                    if (f.exists()) {
                        if (url_return)
                            *url_return = url;
                        return true;
                    }
                }
            }
        }
        return false;
    }
};

class QmlImportsPrivate {
public:
    QmlImportsPrivate() : ref(1)
    {
    }

    ~QmlImportsPrivate()
    {
        foreach (QmlEnginePrivate::ImportedNamespace* s, set.values())
            delete s;
    }

    bool add(const QUrl& base, const QString& uri, const QString& prefix, int vmaj, int vmin, QmlScriptParser::Import::Type importType, const QStringList& importPath)
    {
        QmlEnginePrivate::ImportedNamespace *s;
        if (prefix.isEmpty()) {
            s = &unqualifiedset;
        } else {
            s = set.value(prefix);
            if (!s)
                set.insert(prefix,(s=new QmlEnginePrivate::ImportedNamespace));
        }
        QString url = uri;
        bool isbuiltin = false;
        if (importType == QmlScriptParser::Import::Library) {
            url.replace(QLatin1Char('.'),QLatin1Char('/'));
            bool found = false;
            foreach (QString p, importPath) {
                QString dir = p+QLatin1Char('/')+url;
                QFileInfo fi(dir+QLatin1String("/qmldir"));
                if (fi.isFile()) {
                    url = QUrl::fromLocalFile(fi.absolutePath()).toString();
                    found = true;
                    break;
                }
            }
            if (!found) {
                // XXX assume it is a built-in type qualifier
                isbuiltin = true;
            }
        } else {
            url = base.resolved(QUrl(url)).toString();
        }
        s->urls.prepend(url);
        s->majversions.prepend(vmaj);
        s->minversions.prepend(vmin);
        s->isLibrary.prepend(importType == QmlScriptParser::Import::Library);
        s->isBuiltin.prepend(isbuiltin);
        return true;
    }

    bool find(const QByteArray& type, int *vmajor, int *vminor, QmlType** type_return, QUrl* url_return)
    {
        QmlEnginePrivate::ImportedNamespace *s = 0;
        int slash = type.indexOf('/');
        if (slash >= 0) {
            while (!s) {
                s = set.value(QString::fromLatin1(type.left(slash)));
                int nslash = type.indexOf('/',slash+1);
                if (nslash > 0)
                    slash = nslash;
                else
                    break;
            }
        } else {
            s = &unqualifiedset;
        }
        QByteArray unqualifiedtype = slash < 0 ? type : type.mid(slash+1); // common-case opt (QString::mid works fine, but slower)
        if (s) {
            if (s->find(unqualifiedtype,vmajor,vminor,type_return,url_return))
                return true;
            if (s->urls.count() == 1 && !s->isBuiltin[0] && !s->isLibrary[0] && url_return) {
                *url_return = QUrl(s->urls[0]+"/").resolved(QUrl(QLatin1String(unqualifiedtype + ".qml")));
                return true;
            }
        }
        if (url_return) {
            *url_return = base.resolved(QUrl(QLatin1String(type + ".qml")));
            return true;
        } else {
            return false;
        }
    }

    QmlEnginePrivate::ImportedNamespace *findNamespace(const QString& type)
    {
        return set.value(type);
    }

    QUrl base;
    int ref;

private:
    QmlEnginePrivate::ImportedNamespace unqualifiedset;
    QHash<QString,QmlEnginePrivate::ImportedNamespace* > set;
};

QmlEnginePrivate::Imports::Imports(const Imports &copy) :
    d(copy.d)
{
    ++d->ref;
}

QmlEnginePrivate::Imports &QmlEnginePrivate::Imports::operator =(const Imports &copy)
{
    ++copy.d->ref;
    if (--d->ref == 0)
        delete d;
    d = copy.d;
    return *this;
}

QmlEnginePrivate::Imports::Imports() :
    d(new QmlImportsPrivate)
{
}

QmlEnginePrivate::Imports::~Imports()
{
    if (--d->ref == 0)
        delete d;
}

/*!
  Sets the base URL to be used for all relative file imports added.
*/
void QmlEnginePrivate::Imports::setBaseUrl(const QUrl& url)
{
    d->base = url;
}

/*!
  Returns the base URL to be used for all relative file imports added.
*/
QUrl QmlEnginePrivate::Imports::baseUrl() const
{
    return d->base;
}

/*!
  Adds \a path as a directory where installed QML components are
  defined in a URL-based directory structure.

  For example, if you add \c /opt/MyApp/lib/qml and then load QML
  that imports \c com.mycompany.Feature, then QmlEngine will look
  in \c /opt/MyApp/lib/qml/com/mycompany/Feature/ for the components
  provided by that module (and in the case of versioned imports,
  for the \c qmldir file definiting the type version mapping.
*/
void QmlEngine::addImportPath(const QString& path)
{
    if (qmlImportTrace())
        qDebug() << "QmlEngine::addImportPath" << path;
    Q_D(QmlEngine);
    d->fileImportPath.prepend(path);
}

/*!
  \property QmlEngine::offlineStoragePath
  \brief the directory for storing offline user data

  Returns the directory where SQL and other offline
  storage is placed.

  QFxWebView and the SQL databases created with openDatabase()
  are stored here.

  The default is Nokia/Qt/QML/Databases/ in the platform-standard
  user application data directory.
*/
void QmlEngine::setOfflineStoragePath(const QString& dir)
{
    Q_D(QmlEngine);
    d->offlineStoragePath = dir;
}

QString QmlEngine::offlineStoragePath() const
{
    Q_D(const QmlEngine);
    return d->offlineStoragePath;
}


/*!
  \internal

  Adds information to \a imports such that subsequent calls to resolveType()
  will resolve types qualified by \a prefix by considering types found at the given \a uri.

  The uri is either a directory (if importType is FileImport), or a URI resolved using paths
  added via addImportPath() (if importType is LibraryImport).

  The \a prefix may be empty, in which case the import location is considered for
  unqualified types.

  The base URL must already have been set with Import::setBaseUrl().
*/
bool QmlEnginePrivate::addToImport(Imports* imports, const QString& uri, const QString& prefix, int vmaj, int vmin, QmlScriptParser::Import::Type importType) const
{
    bool ok = imports->d->add(imports->d->base,uri,prefix,vmaj,vmin,importType,fileImportPath);
    if (qmlImportTrace())
        qDebug() << "QmlEngine::addToImport(" << imports << uri << prefix << vmaj << "." << vmin << (importType==QmlScriptParser::Import::Library? "Library" : "File") << ": " << ok;
    return ok;
}

/*!
  \internal

  Using the given \a imports, the given (namespace qualified) \a type is resolved to either
  an ImportedNamespace stored at \a ns_return,
  a QmlType stored at \a type_return, or
  a component located at \a url_return.

  If any return pointer is 0, the corresponding search is not done.

  \sa addToImport()
*/
bool QmlEnginePrivate::resolveType(const Imports& imports, const QByteArray& type, QmlType** type_return, QUrl* url_return, int *vmaj, int *vmin, ImportedNamespace** ns_return) const
{
    ImportedNamespace* ns = imports.d->findNamespace(QLatin1String(type));
    if (ns) {
        if (qmlImportTrace())
            qDebug() << "QmlEngine::resolveType" << type << "is namespace for" << ns->urls;
        if (ns_return)
            *ns_return = ns;
        return true;
    }
    if (type_return || url_return) {
        if (imports.d->find(type,vmaj,vmin,type_return,url_return)) {
            if (qmlImportTrace()) {
                if (type_return && *type_return)
                    qDebug() << "QmlEngine::resolveType" << type << "=" << (*type_return)->typeName();
                if (url_return)
                    qDebug() << "QmlEngine::resolveType" << type << "=" << *url_return;
            }
            return true;
        }
        if (qmlImportTrace())
            qDebug() << "QmlEngine::resolveType" << type << "not found";
    }
    return false;
}

/*!
  \internal

  Searching \e only in the namespace \a ns (previously returned in a call to
  resolveType(), \a type is found and returned to either
  a QmlType stored at \a type_return, or
  a component located at \a url_return.

  If either return pointer is 0, the corresponding search is not done.
*/
void QmlEnginePrivate::resolveTypeInNamespace(ImportedNamespace* ns, const QByteArray& type, QmlType** type_return, QUrl* url_return, int *vmaj, int *vmin ) const
{
    ns->find(type,vmaj,vmin,type_return,url_return);
}

static void voidptr_destructor(void *v)
{
    void **ptr = (void **)v;
    delete ptr;
}

static void *voidptr_constructor(const void *v)
{
    if (!v) {
        return new void*;
    } else {
        return new void*(*(void **)v);
    }
}

void QmlEnginePrivate::registerCompositeType(QmlCompiledData *data)
{
    QByteArray name = data->root.className();

    QByteArray ptr = name + "*";
    QByteArray lst = "QmlList<" + ptr + ">*";

    int ptr_type = QMetaType::registerType(ptr.constData(), voidptr_destructor,
                                           voidptr_constructor);
    int lst_type = QMetaType::registerType(lst.constData(), voidptr_destructor,
                                           voidptr_constructor);

    m_qmlLists.insert(lst_type, ptr_type);
    m_compositeTypes.insert(ptr_type, data);
    data->addref();
}

bool QmlEnginePrivate::isQmlList(int t) const
{
    return m_qmlLists.contains(t) || QmlMetaType::isQmlList(t);
}

bool QmlEnginePrivate::isObject(int t)
{
    return m_compositeTypes.contains(t) || QmlMetaType::isObject(t);
}

int QmlEnginePrivate::qmlListType(int t) const
{
    QHash<int, int>::ConstIterator iter = m_qmlLists.find(t);
    if (iter != m_qmlLists.end())
        return *iter;
    else
        return QmlMetaType::qmlListType(t);
}

const QMetaObject *QmlEnginePrivate::rawMetaObjectForType(int t) const
{
    QHash<int, QmlCompiledData*>::ConstIterator iter = m_compositeTypes.find(t);
    if (iter != m_compositeTypes.end()) {
        return &(*iter)->root;
    } else {
        return QmlMetaType::rawMetaObjectForType(t);
    }
}

const QMetaObject *QmlEnginePrivate::metaObjectForType(int t) const
{
    QHash<int, QmlCompiledData*>::ConstIterator iter = m_compositeTypes.find(t);
    if (iter != m_compositeTypes.end()) {
        return &(*iter)->root;
    } else {
        return QmlMetaType::metaObjectForType(t);
    }
}

QT_END_NAMESPACE
