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

#include <private/qmlpropertycache_p.h>
#include <private/qmlengine_p.h>
#include <qmlbinding.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_METATYPE(QScriptValue);

void QmlPropertyCache::Data::load(const QMetaProperty &p)
{
    propType = p.userType();
    if (QVariant::Type(propType) == QVariant::LastType)
        propType = qMetaTypeId<QVariant>();
    coreIndex = p.propertyIndex();
    notifyIndex = p.notifySignalIndex();

    if (p.isConstant())
        flags |= Data::IsConstant;
    if (p.isWritable())
        flags |= Data::IsWritable;

    if (propType == qMetaTypeId<QmlBinding *>()) {
        flags |= Data::IsQmlBinding;
    } else if (propType == qMetaTypeId<QScriptValue>()) {
        flags |= Data::IsQScriptValue;
    } else if (p.isEnumType()) {
        flags |= Data::IsEnumType;
    } else {
        QmlMetaType::TypeCategory cat = QmlMetaType::typeCategory(propType);
        if (cat == QmlMetaType::Object)
            flags |= Data::IsQObjectDerived;
        else if (cat == QmlMetaType::List)
            flags |= Data::IsQList;
        else if (cat == QmlMetaType::QmlList)
            flags |= Data::IsQmlList;
    }
}

void QmlPropertyCache::Data::load(const QMetaMethod &m)
{
    coreIndex = m.methodIndex();
    flags |= Data::IsFunction;
}


QmlPropertyCache::QmlPropertyCache(QmlEngine *engine)
: QmlCleanup(engine)
{
}

QmlPropertyCache::~QmlPropertyCache()
{
    clear();
}

void QmlPropertyCache::clear()
{
    for (int ii = 0; ii < indexCache.count(); ++ii) 
        indexCache.at(ii)->release();

    for (StringCache::ConstIterator iter = stringCache.begin(); 
            iter != stringCache.end(); ++iter)
        (*iter)->release();

    for (IdentifierCache::ConstIterator iter = identifierCache.begin(); 
            iter != identifierCache.end(); ++iter)
        (*iter)->release();

    indexCache.clear();
    stringCache.clear();
    identifierCache.clear();
}

QmlPropertyCache::Data QmlPropertyCache::create(const QMetaObject *metaObject, 
                                                const QString &property)
{
    Q_ASSERT(metaObject);

    QmlPropertyCache::Data rv;

    int idx = metaObject->indexOfProperty(property.toUtf8());
    if (idx != -1) {
        rv.load(metaObject->property(idx));
        return rv;
    }

    int methodCount = metaObject->methodCount();
    for (int ii = methodCount - 1; ii >= 0; --ii) {
        QMetaMethod m = metaObject->method(ii);
        QString methodName = QString::fromUtf8(m.signature());

        int parenIdx = methodName.indexOf(QLatin1Char('('));
        Q_ASSERT(parenIdx != -1);
        methodName = methodName.left(parenIdx);

        if (methodName == property) {
            rv.load(m);
            return rv;
        }
    }

    return rv;
}

// ### Optimize - check engine for the parent meta object etc.
QmlPropertyCache *QmlPropertyCache::create(QmlEngine *engine, const QMetaObject *metaObject)
{
    Q_ASSERT(engine);
    Q_ASSERT(metaObject);

    QmlPropertyCache *cache = new QmlPropertyCache(engine);

    QmlEnginePrivate *enginePriv = QmlEnginePrivate::get(engine);

    // ### The properties/methods should probably be spliced on a per-metaobject basis
    int propCount = metaObject->propertyCount();

    cache->indexCache.resize(propCount);
    for (int ii = propCount - 1; ii >= 0; --ii) {
        QMetaProperty p = metaObject->property(ii);
        QString propName = QString::fromUtf8(p.name());

        RData *data = new RData;
        data->identifier = enginePriv->objectClass->createPersistentIdentifier(propName);

        data->load(p);

        cache->indexCache[ii] = data;

        if (cache->stringCache.contains(propName))
            continue;

        cache->stringCache.insert(propName, data);
        cache->identifierCache.insert(data->identifier.identifier, data);
        data->addref();
        data->addref();
    }

    int methodCount = metaObject->methodCount();
    for (int ii = methodCount - 1; ii >= 0; --ii) {
        QMetaMethod m = metaObject->method(ii);
        QString methodName = QString::fromUtf8(m.signature());

        int parenIdx = methodName.indexOf(QLatin1Char('('));
        Q_ASSERT(parenIdx != -1);
        methodName = methodName.left(parenIdx);

        if (cache->stringCache.contains(methodName))
            continue;

        RData *data = new RData;
        data->identifier = enginePriv->objectClass->createPersistentIdentifier(methodName);

        data->load(m);

        cache->stringCache.insert(methodName, data);
        cache->identifierCache.insert(data->identifier.identifier, data);
        data->addref();
        data->addref();
    }

    return cache;
}

QmlPropertyCache::Data *
QmlPropertyCache::property(int index) const
{
    if (index < 0 || index >= indexCache.count())
        return 0;

    return indexCache.at(index);
}

QmlPropertyCache::Data *
QmlPropertyCache::property(const QString &str) const
{
    return stringCache.value(str);
}

QString QmlPropertyCache::Data::name(QObject *object)
{
    if (!object)
        return QString();

    return name(object->metaObject());
}

QString QmlPropertyCache::Data::name(const QMetaObject *metaObject)
{
    if (!metaObject || coreIndex == -1)
        return QString();

    if (flags & IsFunction) {
        QMetaMethod m = metaObject->method(coreIndex);

        QString name = QString::fromUtf8(m.signature());
        int parenIdx = name.indexOf(QLatin1Char('('));
        if (parenIdx != -1)
            name = name.left(parenIdx);
        return name;
    } else {
        QMetaProperty p = metaObject->property(coreIndex);
        return QString::fromUtf8(p.name());
    }
}

QT_END_NAMESPACE
