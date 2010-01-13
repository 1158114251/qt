/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qmlobjectscriptclass_p.h"

#include "qmlengine_p.h"
#include "qmlcontext_p.h"
#include "qmldeclarativedata_p.h"
#include "qmltypenamescriptclass_p.h"
#include "qmllistscriptclass_p.h"
#include "qmlbinding.h"
#include "qmlguard_p.h"
#include "qmlvmemetaobject_p.h"

#include <QtCore/qtimer.h>

QT_BEGIN_NAMESPACE

struct ObjectData : public QScriptDeclarativeClass::Object {
    ObjectData(QObject *o, int t) : object(o), type(t) {}
    QmlGuard<QObject> object;
    int type;
};

/*
    The QmlObjectScriptClass handles property access for QObjects
    via QtScript. It is also used to provide a more useful API in
    QtScript for QML.
 */
QmlObjectScriptClass::QmlObjectScriptClass(QmlEngine *bindEngine)
: QScriptDeclarativeClass(QmlEnginePrivate::getScriptEngine(bindEngine)), lastData(0),
  engine(bindEngine)
{
    QScriptEngine *scriptEngine = QmlEnginePrivate::getScriptEngine(engine);

    m_destroy = scriptEngine->newFunction(destroy);
    m_destroyId = createPersistentIdentifier(QLatin1String("destroy"));
    m_toString = scriptEngine->newFunction(tostring);
    m_toStringId = createPersistentIdentifier(QLatin1String("toString"));
}

QmlObjectScriptClass::~QmlObjectScriptClass()
{
}

QScriptValue QmlObjectScriptClass::newQObject(QObject *object, int type) 
{
    QScriptEngine *scriptEngine = QmlEnginePrivate::getScriptEngine(engine);

    if (!object)
        return newObject(scriptEngine, this, new ObjectData(object, type));

    QmlDeclarativeData *ddata = QmlDeclarativeData::get(object, true);

    if (!ddata->scriptValue.isValid()) {
        ddata->scriptValue = newObject(scriptEngine, this, new ObjectData(object, type));
        return ddata->scriptValue;
    } else if (ddata->scriptValue.engine() == QmlEnginePrivate::getScriptEngine(engine)) {
        return ddata->scriptValue;
    } else {
        return newObject(scriptEngine, this, new ObjectData(object, type));
    }
}

QObject *QmlObjectScriptClass::toQObject(const QScriptValue &value) const
{
    return value.toQObject();
}

int QmlObjectScriptClass::objectType(const QScriptValue &value) const
{
    if (scriptClass(value) != this)
        return QVariant::Invalid;

    Object *o = object(value);
    return ((ObjectData*)(o))->type;
}

QScriptClass::QueryFlags 
QmlObjectScriptClass::queryProperty(Object *object, const Identifier &name, 
                                    QScriptClass::QueryFlags flags)
{
    return queryProperty(toQObject(object), name, flags, 0);
}

QScriptClass::QueryFlags 
QmlObjectScriptClass::queryProperty(QObject *obj, const Identifier &name, 
                                    QScriptClass::QueryFlags flags, QmlContext *evalContext,
                                    QueryHints hints)
{
    Q_UNUSED(flags);
    lastData = 0;
    lastTNData = 0;

    if (name == m_destroyId.identifier ||
        name == m_toStringId.identifier)
        return QScriptClass::HandlesReadAccess;

    if (!obj)
        return 0;

    QmlEnginePrivate *enginePrivate = QmlEnginePrivate::get(engine);

    QmlPropertyCache *cache = 0;
    QmlDeclarativeData *ddata = QmlDeclarativeData::get(obj);
    if (ddata)
        cache = ddata->propertyCache;
    if (!cache) {
        cache = enginePrivate->cache(obj);
        if (cache && ddata) { cache->addref(); ddata->propertyCache = cache; }
    }

    if (cache) {
        lastData = cache->property(name);
    } else {
        local = QmlPropertyCache::create(obj->metaObject(), toString(name));
        if (local.isValid())
            lastData = &local;
    }

    if (lastData)
        return QScriptClass::HandlesReadAccess | QScriptClass::HandlesWriteAccess; 

    if (!evalContext && context()) {
        // Global object, QScriptContext activation object, QmlContext object
        QScriptValue scopeNode = scopeChainValue(context(), -3);         
        Q_ASSERT(scopeNode.isValid());
        Q_ASSERT(scriptClass(scopeNode) == enginePrivate->contextClass);

        evalContext = enginePrivate->contextClass->contextFromValue(scopeNode);
    }

    if (evalContext) {
        QmlContextPrivate *cp = QmlContextPrivate::get(evalContext);

        if (cp->imports) {
            QmlTypeNameCache::Data *data = cp->imports->data(name);
            if (data) {
                lastTNData = data;
                return QScriptClass::HandlesReadAccess;
            }
        }
    }

    if (!(hints & ImplicitObject)) {
        local.coreIndex = -1;
        lastData = &local;
        return QScriptClass::HandlesReadAccess | QScriptClass::HandlesWriteAccess;
    }

    return 0;
}

QmlObjectScriptClass::Value
QmlObjectScriptClass::property(Object *object, const Identifier &name)
{
    return property(toQObject(object), name);
}

QmlObjectScriptClass::Value
QmlObjectScriptClass::property(QObject *obj, const Identifier &name)
{
    if (name == m_destroyId.identifier)
        return m_destroy;
    else if (name == m_toStringId.identifier)
        return m_toString;

    if (lastData && !lastData->isValid())
        return QmlEnginePrivate::getScriptEngine(engine)->undefinedValue();

    Q_ASSERT(obj);

    QScriptEngine *scriptEngine = QmlEnginePrivate::getScriptEngine(engine);
    QmlEnginePrivate *enginePriv = QmlEnginePrivate::get(engine);

    if (lastTNData) {

        if (lastTNData->type)
            return enginePriv->typeNameClass->newObject(obj, lastTNData->type);
        else
            return enginePriv->typeNameClass->newObject(obj, lastTNData->typeNamespace);

    } else if (lastData->flags & QmlPropertyCache::Data::IsFunction) {
        if (lastData->flags & QmlPropertyCache::Data::IsVMEFunction) {
            return ((QmlVMEMetaObject *)(obj->metaObject()))->vmeMethod(lastData->coreIndex);
        } else {
            // ### Optimize
            QScriptValue sobj = scriptEngine->newQObject(obj);
            return sobj.property(toString(name));
        }
    } else {
        if (enginePriv->captureProperties && !(lastData->flags & QmlPropertyCache::Data::IsConstant)) {
            enginePriv->capturedProperties << 
                QmlEnginePrivate::CapturedProperty(obj, lastData->coreIndex, lastData->notifyIndex);
        }

        if ((uint)lastData->propType < QVariant::UserType) {
            QmlValueType *valueType = enginePriv->valueTypes[lastData->propType];
            if (valueType)
                return enginePriv->valueTypeClass->newObject(obj, lastData->coreIndex, valueType);
        }

        if (lastData->flags & QmlPropertyCache::Data::IsQList) {
            return enginePriv->listClass->newList(obj, lastData->coreIndex, 
                                                  QmlListScriptClass::QListPtr);
        } else if (lastData->flags & QmlPropertyCache::Data::IsQmlList) {
            return enginePriv->listClass->newList(obj, lastData->coreIndex, 
                                                  QmlListScriptClass::QmlListPtr);
        } else if (lastData->flags & QmlPropertyCache::Data::IsQObjectDerived) {
            QObject *rv = 0;
            void *args[] = { &rv, 0 };
            QMetaObject::metacall(obj, QMetaObject::ReadProperty, lastData->coreIndex, args);
            return newQObject(rv, lastData->propType);
        } else if (lastData->flags & QmlPropertyCache::Data::IsQScriptValue) {
            QScriptValue rv = scriptEngine->nullValue();
            void *args[] = { &rv, 0 };
            QMetaObject::metacall(obj, QMetaObject::ReadProperty, lastData->coreIndex, args);
            return rv;
        } else if (lastData->propType == QMetaType::QReal) {
            qreal rv = 0;
            void *args[] = { &rv, 0 };
            QMetaObject::metacall(obj, QMetaObject::ReadProperty, lastData->coreIndex, args);
            return Value(scriptEngine, rv);
        } else if (lastData->propType == QMetaType::Int) {
            int rv = 0;
            void *args[] = { &rv, 0 };
            QMetaObject::metacall(obj, QMetaObject::ReadProperty, lastData->coreIndex, args);
            return Value(scriptEngine, rv);
        } else if (lastData->propType == QMetaType::Bool) {
            bool rv = false;
            void *args[] = { &rv, 0 };
            QMetaObject::metacall(obj, QMetaObject::ReadProperty, lastData->coreIndex, args);
            return Value(scriptEngine, rv);
        } else if (lastData->propType == QMetaType::QString) {
            QString rv;
            void *args[] = { &rv, 0 };
            QMetaObject::metacall(obj, QMetaObject::ReadProperty, lastData->coreIndex, args);
            return Value(scriptEngine, rv);
        } else if (lastData->propType == QMetaType::UInt) {
            uint rv = 0;
            void *args[] = { &rv, 0 };
            QMetaObject::metacall(obj, QMetaObject::ReadProperty, lastData->coreIndex, args);
            return Value(scriptEngine, rv);
        } else if (lastData->propType == QMetaType::Float) {
            float rv = 0;
            void *args[] = { &rv, 0 };
            QMetaObject::metacall(obj, QMetaObject::ReadProperty, lastData->coreIndex, args);
            return Value(scriptEngine, rv);
        } else if (lastData->propType == QMetaType::Double) {
            double rv = 0;
            void *args[] = { &rv, 0 };
            QMetaObject::metacall(obj, QMetaObject::ReadProperty, lastData->coreIndex, args);
            return Value(scriptEngine, rv);
        } else {
            QVariant var = obj->metaObject()->property(lastData->coreIndex).read(obj);
            return enginePriv->scriptValueFromVariant(var);
        }

    }
}

void QmlObjectScriptClass::setProperty(Object *object, 
                                       const Identifier &name, 
                                       const QScriptValue &value)
{
    return setProperty(toQObject(object), name, value);
}

void QmlObjectScriptClass::setProperty(QObject *obj, 
                                       const Identifier &name, 
                                       const QScriptValue &value,
                                       QmlContext *evalContext)
{
    Q_UNUSED(name);

    Q_ASSERT(obj);
    Q_ASSERT(lastData);

    if (!lastData->isValid()) {
        QString error = QLatin1String("Cannot assign to non-existant property \"") +
                        toString(name) + QLatin1Char('\"');
        if (context())
            context()->throwError(error);
        return;
    }

    if (!(lastData->flags & QmlPropertyCache::Data::IsWritable)) {
        QString error = QLatin1String("Cannot assign to read-only property \"") +
                        toString(name) + QLatin1Char('\"');
        if (context())
            context()->throwError(error);
        return;
    }

    QmlEnginePrivate *enginePriv = QmlEnginePrivate::get(engine);

    if (!evalContext && context()) {
        // Global object, QScriptContext activation object, QmlContext object
        QScriptValue scopeNode = scopeChainValue(context(), -3);         
        Q_ASSERT(scopeNode.isValid());
        Q_ASSERT(scriptClass(scopeNode) == enginePriv->contextClass);

        evalContext = enginePriv->contextClass->contextFromValue(scopeNode);
    }

    // ### Can well known types be optimized?
    QVariant v = QmlScriptClass::toVariant(engine, value);
    QmlAbstractBinding *delBinding = QmlMetaPropertyPrivate::setBinding(obj, *lastData, 0);
    if (delBinding)
        delBinding->destroy();
    QmlMetaPropertyPrivate::write(obj, *lastData, v, evalContext);
}

bool QmlObjectScriptClass::isQObject() const
{
    return true;
}

QObject *QmlObjectScriptClass::toQObject(Object *object, bool *ok)
{
    if (ok) *ok = true;

    ObjectData *data = (ObjectData*)object;
    return data->object.data();
}

QScriptValue QmlObjectScriptClass::tostring(QScriptContext *context, QScriptEngine *)
{
    QObject* obj = context->thisObject().toQObject();

    QString ret;
    if(obj){
        QString objectName = obj->objectName();

        ret += QString::fromUtf8(obj->metaObject()->className());
        ret += QLatin1String("(0x");
        ret += QString::number((quintptr)obj,16);

        if (!objectName.isEmpty()) {
            ret += QLatin1String(", \"");
            ret += objectName;
            ret += QLatin1Char('\"');
        }

        ret += QLatin1Char(')');
    }else{
        ret += QLatin1String("null");
    }
    return QScriptValue(ret);
}

QScriptValue QmlObjectScriptClass::destroy(QScriptContext *context, QScriptEngine *engine)
{
    QObject* obj = context->thisObject().toQObject();
    if(obj){
        int delay = 0;
        if(context->argumentCount() > 0)
            delay = context->argument(0).toInt32();
        if (delay > 0)
            QTimer::singleShot(delay, obj, SLOT(deleteLater()));
        else
            obj->deleteLater();
    }
    return engine->nullValue();
}

QStringList QmlObjectScriptClass::propertyNames(Object *object)
{
    QObject *obj = toQObject(object);
    if (!obj)
        return QStringList();

    QmlEnginePrivate *enginePrivate = QmlEnginePrivate::get(engine);

    QmlPropertyCache *cache = 0;
    QmlDeclarativeData *ddata = QmlDeclarativeData::get(obj);
    if (ddata)
        cache = ddata->propertyCache;
    if (!cache) {
        cache = enginePrivate->cache(obj);
        if (cache && ddata) { cache->addref(); ddata->propertyCache = cache; }
    }

    if (!cache)
        return QStringList();

    return cache->propertyNames();
}

QT_END_NAMESPACE

