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

#include "qmlmetatype.h"

#include "qmlproxymetaobject_p.h"
#include "qmlcustomparser_p.h"

#include <QtCore/qdebug.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qbitarray.h>
#include <QtCore/qreadwritelock.h>
#include <qmetatype.h>
#include <qobjectdefs.h>
#include <qdatetime.h>
#include <qbytearray.h>
#include <qreadwritelock.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvector.h>
#include <qlocale.h>
#include <QtCore/qcryptographichash.h>
#include <QtScript/qscriptvalue.h>

#include <ctype.h>

#ifdef QT_BOOTSTRAPPED
# ifndef QT_NO_GEOM_VARIANT
#  define QT_NO_GEOM_VARIANT
# endif
#else
#  include <qbitarray.h>
#  include <qurl.h>
#  include <qvariant.h>
#endif

#ifndef QT_NO_GEOM_VARIANT
# include <qsize.h>
# include <qpoint.h>
# include <qrect.h>
# include <qline.h>
# include <qvector3d.h>
#endif
#define NS(x) QT_PREPEND_NAMESPACE(x)

QT_BEGIN_NAMESPACE

struct QmlMetaTypeData
{
    ~QmlMetaTypeData();
    QList<QmlType *> types;
    typedef QHash<int, QmlType *> Ids;
    Ids idToType;
    typedef QHash<QByteArray, QmlType *> Names;
    Names nameToType;
    typedef QHash<const QMetaObject *, QmlType *> MetaObjects;
    MetaObjects metaObjectToType;
    typedef QHash<int, QmlMetaType::StringConverter> StringConverters;
    StringConverters stringConverters;

    QBitArray objects;
    QBitArray interfaces;
    QBitArray qmllists;
    QBitArray lists;
};
Q_GLOBAL_STATIC(QmlMetaTypeData, metaTypeData)
Q_GLOBAL_STATIC(QReadWriteLock, metaTypeDataLock)

QmlMetaTypeData::~QmlMetaTypeData()
{
    for (int i = 0; i < types.count(); ++i)
        delete types.at(i);
}

class QmlTypePrivate
{
public:
    QmlTypePrivate();

    void init() const;

    bool m_isInterface : 1;
    const char *m_iid;
    QByteArray m_name;
    int m_version_maj;
    int m_version_min;
    int m_typeId; int m_listId; int m_qmlListId;
    QmlPrivate::Func m_opFunc;
    const QMetaObject *m_baseMetaObject;
    QmlAttachedPropertiesFunc m_attachedPropertiesFunc;
    const QMetaObject *m_attachedPropertiesType;
    int m_parserStatusCast;
    int m_propertyValueSourceCast;
    int m_propertyValueInterceptorCast;
    QmlPrivate::CreateFunc m_extFunc;
    const QMetaObject *m_extMetaObject;
    int m_index;
    QmlCustomParser *m_customParser;
    mutable volatile bool m_isSetup:1;
    mutable QList<QmlProxyMetaObject::ProxyData> m_metaObjects;
};

QmlTypePrivate::QmlTypePrivate()
: m_isInterface(false), m_iid(0), m_typeId(0), m_listId(0), m_qmlListId(0),
  m_opFunc(0), m_baseMetaObject(0), m_attachedPropertiesFunc(0), m_attachedPropertiesType(0),
  m_parserStatusCast(-1), m_propertyValueSourceCast(-1), m_propertyValueInterceptorCast(-1),
  m_extFunc(0), m_extMetaObject(0), m_index(-1), m_customParser(0), m_isSetup(false)
{
}


QmlType::QmlType(int type, int listType, int qmlListType,
                 QmlPrivate::Func opFunc, const char *iid, int index)
: d(new QmlTypePrivate)
{
    d->m_isInterface = true;
    d->m_iid = iid;
    d->m_typeId = type;
    d->m_listId = listType;
    d->m_qmlListId = qmlListType;
    d->m_opFunc = opFunc;
    d->m_index = index;
    d->m_isSetup = true;
    d->m_version_maj = 0;
    d->m_version_min = 0;
}

QmlType::QmlType(int type, int listType, int qmlListType,
                 QmlPrivate::Func opFunc, const char *qmlName,
                 int version_maj, int version_min,
                 const QMetaObject *metaObject,
                 QmlAttachedPropertiesFunc attachedPropertiesFunc,
                 const QMetaObject *attachedType,
                 int parserStatusCast, int propertyValueSourceCast, int propertyValueInterceptorCast,
                 QmlPrivate::CreateFunc extFunc,
                 const QMetaObject *extMetaObject, int index,
                 QmlCustomParser *customParser)
: d(new QmlTypePrivate)
{
    d->m_name = qmlName;
    d->m_version_maj = version_maj;
    d->m_version_min = version_min;
    d->m_typeId = type;
    d->m_listId = listType;
    d->m_qmlListId = qmlListType;
    d->m_opFunc = opFunc;
    d->m_baseMetaObject = metaObject;
    d->m_attachedPropertiesFunc = attachedPropertiesFunc;
    d->m_attachedPropertiesType = attachedType;
    d->m_parserStatusCast = parserStatusCast;
    d->m_propertyValueSourceCast = propertyValueSourceCast;
    d->m_propertyValueInterceptorCast = propertyValueInterceptorCast;
    d->m_extFunc = extFunc;
    d->m_index = index;
    d->m_customParser = customParser;

    if (extMetaObject)
        d->m_extMetaObject = extMetaObject;
}

QmlType::~QmlType()
{
    delete d->m_customParser;
    delete d;
}

int QmlType::majorVersion() const
{
    return d->m_version_maj;
}

int QmlType::minorVersion() const
{
    return d->m_version_min;
}

bool QmlType::availableInVersion(int vmajor, int vminor) const
{
    return vmajor > d->m_version_maj || (vmajor == d->m_version_maj && vminor >= d->m_version_min);
}

void QmlTypePrivate::init() const
{
    if (m_isSetup) return;

    QWriteLocker lock(metaTypeDataLock());
    if (m_isSetup)
        return;

    // Setup extended meta object
    // XXX - very inefficient
    const QMetaObject *mo = m_baseMetaObject;
    if (m_extFunc) {
        QMetaObject *mmo = new QMetaObject;
        *mmo = *m_extMetaObject;
        mmo->d.superdata = mo;
        QmlProxyMetaObject::ProxyData data = { mmo, m_extFunc, 0, 0 };
        m_metaObjects << data;
    }

    mo = mo->d.superdata;
    while(mo) {
        QmlType *t = metaTypeData()->metaObjectToType.value(mo);
        if (t) {
            if (t->d->m_extFunc) {
                QMetaObject *mmo = new QMetaObject;
                *mmo = *t->d->m_extMetaObject;
                mmo->d.superdata = m_baseMetaObject;
                if (!m_metaObjects.isEmpty())
                    m_metaObjects.last().metaObject->d.superdata = mmo;
                QmlProxyMetaObject::ProxyData data = { mmo, t->d->m_extFunc, 0, 0 };
                m_metaObjects << data;
            }
        }
        mo = mo->d.superdata;
    }

    for (int ii = 0; ii < m_metaObjects.count(); ++ii) {
        m_metaObjects[ii].propertyOffset =
            m_metaObjects.at(ii).metaObject->propertyOffset();
        m_metaObjects[ii].methodOffset =
            m_metaObjects.at(ii).metaObject->methodOffset();
    }

    m_isSetup = true;
    lock.unlock();
}

QByteArray QmlType::typeName() const
{
    if (d->m_baseMetaObject)
        return d->m_baseMetaObject->className();
    else
        return QByteArray();
}

QByteArray QmlType::qmlTypeName() const
{
    return d->m_name;
}

QObject *QmlType::create() const
{
    d->init();

    QVariant v;
    QObject *rv = 0;
    d->m_opFunc(QmlPrivate::Create, 0, v, v, (void **)&rv);

    if (rv && !d->m_metaObjects.isEmpty())
        (void *)new QmlProxyMetaObject(rv, &d->m_metaObjects);

    return rv;
}

QmlCustomParser *QmlType::customParser() const
{
    return d->m_customParser;
}

bool QmlType::isInterface() const
{
    return d->m_isInterface;
}

int QmlType::typeId() const
{
    return d->m_typeId;
}

int QmlType::qListTypeId() const
{
    return d->m_listId;
}

int QmlType::qmlListTypeId() const
{
    return d->m_qmlListId;
}

void QmlType::listClear(const QVariant &list)
{
    Q_ASSERT(list.userType() == qListTypeId());
    QVariant arg;
    d->m_opFunc(QmlPrivate::Clear, 0, list, arg, 0);
}

void QmlType::listAppend(const QVariant &list, const QVariant &item)
{
    Q_ASSERT(list.userType() == qListTypeId());
    d->m_opFunc(QmlPrivate::Append, 0, list, item, 0);
}

QVariant QmlType::listAt(const QVariant &list, int idx)
{
    Q_ASSERT(list.userType() == qListTypeId());
    QVariant rv;
    void *ptr = (void *)&rv;
    d->m_opFunc(QmlPrivate::Value, idx, list, QVariant(), &ptr);
    return rv;
}

int QmlType::listCount(const QVariant &list)
{
    Q_ASSERT(list.userType() == qListTypeId());
    return d->m_opFunc(QmlPrivate::Length, 0, list, QVariant(), 0);
}

const QMetaObject *QmlType::metaObject() const
{
    d->init();

    if (d->m_metaObjects.isEmpty())
        return d->m_baseMetaObject;
    else
        return d->m_metaObjects.first().metaObject;

}

const QMetaObject *QmlType::baseMetaObject() const
{
    return d->m_baseMetaObject;
}

QmlAttachedPropertiesFunc QmlType::attachedPropertiesFunction() const
{
    return d->m_attachedPropertiesFunc;
}

const QMetaObject *QmlType::attachedPropertiesType() const
{
    return d->m_attachedPropertiesType;
}

int QmlType::parserStatusCast() const
{
    return d->m_parserStatusCast;
}

int QmlType::propertyValueSourceCast() const
{
    return d->m_propertyValueSourceCast;
}

int QmlType::propertyValueInterceptorCast() const
{
    return d->m_propertyValueInterceptorCast;
}

QVariant QmlType::fromObject(QObject *obj) const
{
    QVariant rv;
    QVariant *v_ptr = &rv;
    QVariant vobj = QVariant::fromValue(obj);
    d->m_opFunc(QmlPrivate::FromObject, 0, QVariant(), vobj, (void **)&v_ptr);
    return rv;
}

const char *QmlType::interfaceIId() const
{
    return d->m_iid;
}

int QmlType::index() const
{
    return d->m_index;
}

int QmlMetaType::registerInterface(const QmlPrivate::MetaTypeIds &id,
                                    QmlPrivate::Func listFunction,
                                    const char *iid)
{
    QWriteLocker lock(metaTypeDataLock());
    QmlMetaTypeData *data = metaTypeData();

    int index = data->types.count();

    QmlType *type = new QmlType(id.typeId, id.listId, id.qmlListId,
                                listFunction, iid, index);

    data->types.append(type);
    data->idToType.insert(type->typeId(), type);
    data->idToType.insert(type->qListTypeId(), type);
    data->idToType.insert(type->qmlListTypeId(), type);
    // XXX No insertMulti, so no multi-version interfaces?
    if (!type->qmlTypeName().isEmpty())
        data->nameToType.insert(type->qmlTypeName(), type);

    if (data->interfaces.size() < id.typeId)
        data->interfaces.resize(id.typeId + 16);
    if (data->qmllists.size() < id.qmlListId)
        data->qmllists.resize(id.qmlListId + 16);
    if (data->lists.size() < id.listId)
        data->lists.resize(id.listId + 16);
    data->interfaces.setBit(id.typeId, true);
    data->qmllists.setBit(id.qmlListId, true);
    data->lists.setBit(id.listId, true);

    return index;
}

int QmlMetaType::registerType(const QmlPrivate::MetaTypeIds &id, QmlPrivate::Func func,
        const char *uri, int version_maj, int version_min, const char *cname,
        const QMetaObject *mo, QmlAttachedPropertiesFunc attach, const QMetaObject *attachMo,
        int pStatus, int object, int valueSource, int valueInterceptor, QmlPrivate::CreateFunc extFunc, const QMetaObject *extmo, QmlCustomParser *parser)
{
    Q_UNUSED(object);

    if (cname) {
        for (int ii = 0; cname[ii]; ++ii) {
            if (!isalnum(cname[ii])) {
                qWarning("QmlMetaType: Invalid QML name %s", cname);
                return -1;
            }
        }
    }

    QWriteLocker lock(metaTypeDataLock());
    QmlMetaTypeData *data = metaTypeData();
    int index = data->types.count();

    QByteArray name = uri;
    if (uri)
        name += '/';
    name += cname;

    QmlType *type = new QmlType(id.typeId, id.listId, id.qmlListId,
                                func, name, version_maj, version_min, mo, attach, attachMo, pStatus,
                                valueSource, valueInterceptor, extFunc, extmo, index, parser);

    data->types.append(type);
    data->idToType.insert(type->typeId(), type);
    data->idToType.insert(type->qListTypeId(), type);
    data->idToType.insert(type->qmlListTypeId(), type);

    if (!type->qmlTypeName().isEmpty())
        data->nameToType.insertMulti(type->qmlTypeName(), type);

    data->metaObjectToType.insert(type->baseMetaObject(), type);

    if (data->objects.size() <= id.typeId)
        data->objects.resize(id.typeId + 16);
    if (data->qmllists.size() <= id.qmlListId)
        data->qmllists.resize(id.qmlListId + 16);
    if (data->lists.size() <= id.listId)
        data->lists.resize(id.listId + 16);
    data->objects.setBit(id.typeId, true);
    data->qmllists.setBit(id.qmlListId, true);
    data->lists.setBit(id.listId, true);

    return index;
}

int QmlMetaType::qmlParserStatusCast(int userType)
{
    QReadLocker lock(metaTypeDataLock());
    QmlMetaTypeData *data = metaTypeData();
    QmlType *type = data->idToType.value(userType);
    if (type && type->typeId() == userType)
        return type->parserStatusCast();
    else
        return -1;
}

int QmlMetaType::qmlPropertyValueSourceCast(int userType)
{
    QReadLocker lock(metaTypeDataLock());
    QmlMetaTypeData *data = metaTypeData();
    QmlType *type = data->idToType.value(userType);
    if (type && type->typeId() == userType)
        return type->propertyValueSourceCast();
    else
        return -1;
}

int QmlMetaType::qmlPropertyValueInterceptorCast(int userType)
{
    QReadLocker lock(metaTypeDataLock());
    QmlMetaTypeData *data = metaTypeData();
    QmlType *type = data->idToType.value(userType);
    if (type && type->typeId() == userType)
        return type->propertyValueInterceptorCast();
    else
        return -1;
}

QObject *QmlMetaType::toQObject(const QVariant &v)
{
    if (!isObject(v.userType()))
        return 0;

    // NOTE: This assumes a cast to QObject does not alter the
    // object pointer
    QObject *rv = *(QObject **)v.constData();
    return rv;
}

/*
    Returns the item type for a list of type \a id.
 */
int QmlMetaType::listType(int id)
{
    QReadLocker lock(metaTypeDataLock());
    QmlMetaTypeData *data = metaTypeData();
    QmlType *type = data->idToType.value(id);
    if (type && type->qListTypeId() == id)
        return type->typeId();
    else
        return 0;
}

/*
    Returns the item type for a qml list of type \a id.
 */
int QmlMetaType::qmlListType(int id)
{
    QReadLocker lock(metaTypeDataLock());
    QmlMetaTypeData *data = metaTypeData();
    QmlType *type = data->idToType.value(id);
    if (type && type->qmlListTypeId() == id)
        return type->typeId();
    else
        return 0;
}

bool QmlMetaType::clear(const QVariant &list)
{
    int userType = list.userType();
    QReadLocker lock(metaTypeDataLock());
    QmlMetaTypeData *data = metaTypeData();
    QmlType *type = data->idToType.value(userType);
    lock.unlock();
    if (type && type->qListTypeId() == userType) {
        type->listClear(list);
        return true;
    } else {
        return false;
    }
}

bool QmlMetaType::append(const QVariant &list, const QVariant &item)
{
    int userType = list.userType();
    QReadLocker lock(metaTypeDataLock());
    QmlMetaTypeData *data = metaTypeData();
    QmlType *type = data->idToType.value(userType);
    lock.unlock();
    if (type && type->qListTypeId() == userType &&
       item.userType() == type->typeId()) {
        type->listAppend(list, item);
        return true;
    } else {
        return false;
    }
}

QVariant QmlMetaType::fromObject(QObject *obj, int typeId)
{
    QReadLocker lock(metaTypeDataLock());
    QmlMetaTypeData *data = metaTypeData();

    QmlType *type = data->idToType.value(typeId);
    if (type && type->typeId() == typeId)
        return type->fromObject(obj);
    else
        return QVariant();
}

const QMetaObject *QmlMetaType::rawMetaObjectForType(int id)
{
    if (id == QMetaType::QObjectStar)
        return &QObject::staticMetaObject;

    QReadLocker lock(metaTypeDataLock());
    QmlMetaTypeData *data = metaTypeData();

    QmlType *type = data->idToType.value(id);
    if (type && type->typeId() == id)
        return type->baseMetaObject();
    else
        return 0;
}

const QMetaObject *QmlMetaType::metaObjectForType(int id)
{
    QReadLocker lock(metaTypeDataLock());
    QmlMetaTypeData *data = metaTypeData();
    QmlType *type = data->idToType.value(id);
    lock.unlock();

    if (type && type->typeId() == id)
        return type->metaObject();
    else
        return 0;
}

int QmlMetaType::attachedPropertiesFuncId(const QMetaObject *mo)
{
    QReadLocker lock(metaTypeDataLock());
    QmlMetaTypeData *data = metaTypeData();

    QmlType *type = data->metaObjectToType.value(mo);
    if (type && type->attachedPropertiesFunction())
        return type->index();
    else
        return -1;
}

QmlAttachedPropertiesFunc QmlMetaType::attachedPropertiesFuncById(int id)
{
    if (id < 0)
        return 0;
    QReadLocker lock(metaTypeDataLock());
    QmlMetaTypeData *data = metaTypeData();
    return data->types.at(id)->attachedPropertiesFunction();
}

QMetaProperty QmlMetaType::defaultProperty(const QMetaObject *metaObject)
{
    int idx = metaObject->indexOfClassInfo("DefaultProperty");
    if (-1 == idx)
        return QMetaProperty();

    QMetaClassInfo info = metaObject->classInfo(idx);
    if (!info.value())
        return QMetaProperty();

    idx = metaObject->indexOfProperty(info.value());
    if (-1 == idx)
        return QMetaProperty();

    return metaObject->property(idx);
}

QMetaProperty QmlMetaType::defaultProperty(QObject *obj)
{
    if (!obj)
        return QMetaProperty();

    const QMetaObject *metaObject = obj->metaObject();
    return defaultProperty(metaObject);
}

QMetaMethod QmlMetaType::defaultMethod(const QMetaObject *metaObject)
{
    int idx = metaObject->indexOfClassInfo("DefaultMethod");
    if (-1 == idx)
        return QMetaMethod();

    QMetaClassInfo info = metaObject->classInfo(idx);
    if (!info.value())
        return QMetaMethod();

    idx = metaObject->indexOfMethod(info.value());
    if (-1 == idx)
        return QMetaMethod();

    return metaObject->method(idx);
}

QMetaMethod QmlMetaType::defaultMethod(QObject *obj)
{
    if (!obj)
        return QMetaMethod();

    const QMetaObject *metaObject = obj->metaObject();
    return defaultMethod(metaObject);
}

/*!
 */
QMetaProperty QmlMetaType::property(QObject *obj, const QByteArray &bname)
{
    return property(obj, bname.constData());
}

/*!
 */
QMetaProperty QmlMetaType::property(QObject *obj, const char *name)
{
    if (!obj)
        return QMetaProperty();

    const QMetaObject *metaObject = obj->metaObject();

    int idx = metaObject->indexOfProperty(name);
    if (-1 == idx)
        return QMetaProperty();

    return metaObject->property(idx);
}

QmlMetaType::TypeCategory QmlMetaType::typeCategory(int userType)
{
    if (userType < 0)
        return Unknown;
    if (userType == QMetaType::QObjectStar)
        return Object;

    QReadLocker lock(metaTypeDataLock());
    QmlMetaTypeData *data = metaTypeData();
    if (userType < data->objects.size() && data->objects.testBit(userType))
        return Object;
    else if (userType < data->qmllists.size() && data->qmllists.testBit(userType))
        return QmlList;
    else if (userType < data->lists.size() && data->lists.testBit(userType))
        return List;
    else
        return Unknown;
}

bool QmlMetaType::isObject(int userType)
{
    if (userType == QMetaType::QObjectStar)
        return true;

    QReadLocker lock(metaTypeDataLock());
    QmlMetaTypeData *data = metaTypeData();
    return userType >= 0 && userType < data->objects.size() && data->objects.testBit(userType);
}

bool QmlMetaType::isInterface(int userType)
{
    QReadLocker lock(metaTypeDataLock());
    QmlMetaTypeData *data = metaTypeData();
    return userType >= 0 && userType < data->interfaces.size() && data->interfaces.testBit(userType);
}

const char *QmlMetaType::interfaceIId(int userType)
{
    QReadLocker lock(metaTypeDataLock());
    QmlMetaTypeData *data = metaTypeData();
    QmlType *type = data->idToType.value(userType);
    lock.unlock();
    if (type && type->isInterface() && type->typeId() == userType)
        return type->interfaceIId();
    else
        return 0;
}

bool QmlMetaType::isQmlList(int userType)
{
    QReadLocker lock(metaTypeDataLock());
    QmlMetaTypeData *data = metaTypeData();
    return userType >= 0 && userType < data->qmllists.size() && data->qmllists.testBit(userType);
}

bool QmlMetaType::isList(int userType)
{
    QReadLocker lock(metaTypeDataLock());
    QmlMetaTypeData *data = metaTypeData();
    return userType >= 0 && userType < data->lists.size() && data->lists.testBit(userType);
}

bool QmlMetaType::isList(const QVariant &v)
{
    return (v.type() == QVariant::UserType && isList(v.userType()));
}

int QmlMetaType::listCount(const QVariant &v)
{
    int userType = v.userType();

    QReadLocker lock(metaTypeDataLock());
    QmlMetaTypeData *data = metaTypeData();
    QmlType *type = data->idToType.value(userType);
    lock.unlock();

    if (type && type->qListTypeId() == userType)
        return type->listCount(v);
    else
        return 0;
}

QVariant QmlMetaType::listAt(const QVariant &v, int idx)
{
    if (idx < 0)
        return QVariant();

    int userType = v.userType();

    QReadLocker lock(metaTypeDataLock());
    QmlMetaTypeData *data = metaTypeData();
    QmlType *type = data->idToType.value(userType);
    lock.unlock();

    if (type && type->qListTypeId() == userType)
        return type->listAt(v, idx);
    else
        return QVariant();
}

/*!
    A custom string convertor allows you to specify a function pointer that
    returns a variant of \a type. For example, if you have written your own icon
    class that you want to support as an object property assignable in QML:

    \code
    int type = qRegisterMetaType<SuperIcon>("SuperIcon");
    QML::addCustomStringConvertor(type, &SuperIcon::pixmapFromString);
    \endcode

    The function pointer must be of the form:
    \code
    QVariant (*StringConverter)(const QString &);
    \endcode
 */
void QmlMetaType::registerCustomStringConverter(int type, StringConverter converter)
{
    QWriteLocker lock(metaTypeDataLock());

    QmlMetaTypeData *data = metaTypeData();
    if (data->stringConverters.contains(type))
        return;
    data->stringConverters.insert(type, converter);
}

/*!
    Return the custom string converter for \a type, previously installed through
    registerCustomStringConverter()
 */
QmlMetaType::StringConverter QmlMetaType::customStringConverter(int type)
{
    QReadLocker lock(metaTypeDataLock());

    QmlMetaTypeData *data = metaTypeData();
    return data->stringConverters.value(type);
}

/*!
    Returns the type (if any) of URI-qualified named \a name in version specified
    by \a version_major and \a version_minor.
*/
QmlType *QmlMetaType::qmlType(const QByteArray &name, int version_major, int version_minor)
{
    QReadLocker lock(metaTypeDataLock());
    QmlMetaTypeData *data = metaTypeData();

    QList<QmlType*> types = data->nameToType.values(name);
    foreach (QmlType *t, types) {
        // XXX version_major<0 just a kludge for QmlMetaPropertyPrivate::initProperty
        if (version_major<0 || t->availableInVersion(version_major,version_minor))
            return t;
    }
    return 0;
}

QmlType *QmlMetaType::qmlType(const QMetaObject *metaObject)
{
    QReadLocker lock(metaTypeDataLock());
    QmlMetaTypeData *data = metaTypeData();

    return data->metaObjectToType.value(metaObject);
}

QList<QByteArray> QmlMetaType::qmlTypeNames()
{
    QReadLocker lock(metaTypeDataLock());
    QmlMetaTypeData *data = metaTypeData();

    return data->nameToType.keys();
}

QList<QmlType*> QmlMetaType::qmlTypes()
{
    QReadLocker lock(metaTypeDataLock());
    QmlMetaTypeData *data = metaTypeData();

    return data->nameToType.values();
}

#include <QtGui/qfont.h>
#include <QtGui/qpixmap.h>
#include <QtGui/qbrush.h>
#include <QtGui/qcolor.h>
#include <QtGui/qpalette.h>
#include <QtGui/qicon.h>
#include <QtGui/qimage.h>
#include <QtGui/qpolygon.h>
#include <QtGui/qregion.h>
#include <QtGui/qbitmap.h>
#include <QtGui/qcursor.h>
#include <QtGui/qsizepolicy.h>
#include <QtGui/qkeysequence.h>
#include <QtGui/qpen.h>

//#include <QtGui/qtextlength.h>
#include <QtGui/qtextformat.h>
#include <QtGui/qmatrix.h>
#include <QtGui/qtransform.h>
#include <QtGui/qmatrix4x4.h>
#include <QtGui/qvector2d.h>
#include <QtGui/qvector3d.h>
#include <QtGui/qvector4d.h>
#include <QtGui/qquaternion.h>

Q_DECLARE_METATYPE(QScriptValue);

/*!
    Copies \a copy into \a data, assuming they both are of type \a type.  If
    \a copy is zero, a default type is copied.  Returns true if the copy was
    successful and false if not.

    \note This should move into QMetaType once complete

*/
bool QmlMetaType::copy(int type, void *data, const void *copy)
{
    if (copy) {
        switch(type) {
        case QMetaType::VoidStar:
        case QMetaType::QObjectStar:
        case QMetaType::QWidgetStar:
            *static_cast<void **>(data) = *static_cast<void* const *>(copy);
            return true;
        case QMetaType::Long:
            *static_cast<long *>(data) = *static_cast<const long*>(copy);
            return true;
        case QMetaType::Int:
            *static_cast<int *>(data) = *static_cast<const int*>(copy);
            return true;
        case QMetaType::Short:
            *static_cast<short *>(data) = *static_cast<const short*>(copy);
            return true;
        case QMetaType::Char:
            *static_cast<char *>(data) = *static_cast<const char*>(copy);
            return true;
        case QMetaType::ULong:
            *static_cast<ulong *>(data) = *static_cast<const ulong*>(copy);
            return true;
        case QMetaType::UInt:
            *static_cast<uint *>(data) = *static_cast<const uint*>(copy);
            return true;
        case QMetaType::LongLong:
            *static_cast<qlonglong *>(data) = *static_cast<const qlonglong*>(copy);
            return true;
        case QMetaType::ULongLong:
            *static_cast<qulonglong *>(data) = *static_cast<const qulonglong*>(copy);
            return true;
        case QMetaType::UShort:
            *static_cast<ushort *>(data) = *static_cast<const ushort*>(copy);
            return true;
        case QMetaType::UChar:
            *static_cast<uchar *>(data) = *static_cast<const uchar*>(copy);
            return true;
        case QMetaType::Bool:
            *static_cast<bool *>(data) = *static_cast<const bool*>(copy);
            return true;
        case QMetaType::Float:
            *static_cast<float *>(data) = *static_cast<const float*>(copy);
            return true;
        case QMetaType::Double:
            *static_cast<double *>(data) = *static_cast<const double*>(copy);
            return true;
        case QMetaType::QChar:
            *static_cast<NS(QChar) *>(data) = *static_cast<const NS(QChar)*>(copy);
            return true;
        case QMetaType::QVariantMap:
            *static_cast<NS(QVariantMap) *>(data) = *static_cast<const NS(QVariantMap)*>(copy);
            return true;
        case QMetaType::QVariantHash:
            *static_cast<NS(QVariantHash) *>(data) = *static_cast<const NS(QVariantHash)*>(copy);
            return true;
        case QMetaType::QVariantList:
            *static_cast<NS(QVariantList) *>(data) = *static_cast<const NS(QVariantList)*>(copy);
            return true;
        case QMetaType::QByteArray:
            *static_cast<NS(QByteArray) *>(data) = *static_cast<const NS(QByteArray)*>(copy);
            return true;
        case QMetaType::QString:
            *static_cast<NS(QString) *>(data) = *static_cast<const NS(QString)*>(copy);
            return true;
        case QMetaType::QStringList:
            *static_cast<NS(QStringList) *>(data) = *static_cast<const NS(QStringList)*>(copy);
            return true;
        case QMetaType::QBitArray:
            *static_cast<NS(QBitArray) *>(data) = *static_cast<const NS(QBitArray)*>(copy);
            return true;
        case QMetaType::QDate:
            *static_cast<NS(QDate) *>(data) = *static_cast<const NS(QDate)*>(copy);
            return true;
        case QMetaType::QTime:
            *static_cast<NS(QTime) *>(data) = *static_cast<const NS(QTime)*>(copy);
            return true;
        case QMetaType::QDateTime:
            *static_cast<NS(QDateTime) *>(data) = *static_cast<const NS(QDateTime)*>(copy);
            return true;
        case QMetaType::QUrl:
            *static_cast<NS(QUrl) *>(data) = *static_cast<const NS(QUrl)*>(copy);
            return true;
        case QMetaType::QLocale:
            *static_cast<NS(QLocale) *>(data) = *static_cast<const NS(QLocale)*>(copy);
            return true;
        case QMetaType::QRect:
            *static_cast<NS(QRect) *>(data) = *static_cast<const NS(QRect)*>(copy);
            return true;
        case QMetaType::QRectF:
            *static_cast<NS(QRectF) *>(data) = *static_cast<const NS(QRectF)*>(copy);
            return true;
        case QMetaType::QSize:
            *static_cast<NS(QSize) *>(data) = *static_cast<const NS(QSize)*>(copy);
            return true;
        case QMetaType::QSizeF:
            *static_cast<NS(QSizeF) *>(data) = *static_cast<const NS(QSizeF)*>(copy);
            return true;
        case QMetaType::QLine:
            *static_cast<NS(QLine) *>(data) = *static_cast<const NS(QLine)*>(copy);
            return true;
        case QMetaType::QLineF:
            *static_cast<NS(QLineF) *>(data) = *static_cast<const NS(QLineF)*>(copy);
            return true;
        case QMetaType::QPoint:
            *static_cast<NS(QPoint) *>(data) = *static_cast<const NS(QPoint)*>(copy);
            return true;
        case QMetaType::QPointF:
            *static_cast<NS(QPointF) *>(data) = *static_cast<const NS(QPointF)*>(copy);
            return true;
        case QMetaType::QVector3D:
            *static_cast<NS(QVector3D) *>(data) = *static_cast<const NS(QVector3D)*>(copy);
            return true;
#ifndef QT_NO_REGEXP
        case QMetaType::QRegExp:
            *static_cast<NS(QRegExp) *>(data) = *static_cast<const NS(QRegExp)*>(copy);
            return true;
#endif
        case QMetaType::Void:
            return true;


#ifdef QT3_SUPPORT
        case QMetaType::QColorGroup:
            *static_cast<NS(QColorGroup) *>(data) = *static_cast<const NS(QColorGroup)*>(copy);
            return true;
#endif

        case QMetaType::QFont:
            *static_cast<NS(QFont) *>(data) = *static_cast<const NS(QFont)*>(copy);
            return true;
        case QMetaType::QPixmap:
            *static_cast<NS(QPixmap) *>(data) = *static_cast<const NS(QPixmap)*>(copy);
            return true;
        case QMetaType::QBrush:
            *static_cast<NS(QBrush) *>(data) = *static_cast<const NS(QBrush)*>(copy);
            return true;
        case QMetaType::QColor:
            *static_cast<NS(QColor) *>(data) = *static_cast<const NS(QColor)*>(copy);
            return true;
        case QMetaType::QPalette:
            *static_cast<NS(QPalette) *>(data) = *static_cast<const NS(QPalette)*>(copy);
            return true;
        case QMetaType::QIcon:
            *static_cast<NS(QIcon) *>(data) = *static_cast<const NS(QIcon)*>(copy);
            return true;
        case QMetaType::QImage:
            *static_cast<NS(QImage) *>(data) = *static_cast<const NS(QImage)*>(copy);
            return true;
        case QMetaType::QPolygon:
            *static_cast<NS(QPolygon) *>(data) = *static_cast<const NS(QPolygon)*>(copy);
            return true;
        case QMetaType::QRegion:
            *static_cast<NS(QRegion) *>(data) = *static_cast<const NS(QRegion)*>(copy);
            return true;
        case QMetaType::QBitmap:
            *static_cast<NS(QBitmap) *>(data) = *static_cast<const NS(QBitmap)*>(copy);
            return true;
        case QMetaType::QCursor:
            *static_cast<NS(QCursor) *>(data) = *static_cast<const NS(QCursor)*>(copy);
            return true;
        case QMetaType::QSizePolicy:
            *static_cast<NS(QSizePolicy) *>(data) = *static_cast<const NS(QSizePolicy)*>(copy);
            return true;
        case QMetaType::QKeySequence:
            *static_cast<NS(QKeySequence) *>(data) = *static_cast<const NS(QKeySequence)*>(copy);
            return true;
        case QMetaType::QPen:
            *static_cast<NS(QPen) *>(data) = *static_cast<const NS(QPen)*>(copy);
            return true;
        case QMetaType::QTextLength:
            *static_cast<NS(QTextLength) *>(data) = *static_cast<const NS(QTextLength)*>(copy);
            return true;
        case QMetaType::QTextFormat:
            *static_cast<NS(QTextFormat) *>(data) = *static_cast<const NS(QTextFormat)*>(copy);
            return true;
        case QMetaType::QMatrix:
            *static_cast<NS(QMatrix) *>(data) = *static_cast<const NS(QMatrix)*>(copy);
            return true;
        case QMetaType::QTransform:
            *static_cast<NS(QTransform) *>(data) = *static_cast<const NS(QTransform)*>(copy);
            return true;
        case QMetaType::QMatrix4x4:
            *static_cast<NS(QMatrix4x4) *>(data) = *static_cast<const NS(QMatrix4x4)*>(copy);
            return true;
        case QMetaType::QVector2D:
            *static_cast<NS(QVector2D) *>(data) = *static_cast<const NS(QVector2D)*>(copy);
            return true;
        case QMetaType::QVector4D:
            *static_cast<NS(QVector4D) *>(data) = *static_cast<const NS(QVector4D)*>(copy);
            return true;
        case QMetaType::QQuaternion:
            *static_cast<NS(QQuaternion) *>(data) = *static_cast<const NS(QQuaternion)*>(copy);
            return true;

        default:
            if (type == qMetaTypeId<QVariant>()) {
                *static_cast<NS(QVariant) *>(data) = *static_cast<const NS(QVariant)*>(copy);
                return true;
            } else if (type == qMetaTypeId<QScriptValue>()) {
                *static_cast<NS(QScriptValue) *>(data) = *static_cast<const NS(QScriptValue)*>(copy);
                return true;
            } else if (typeCategory(type) != Unknown) {
                *static_cast<void **>(data) = *static_cast<void* const *>(copy);
                return true;
            }
            break;
        }
    } else {
        switch(type) {
        case QMetaType::VoidStar:
        case QMetaType::QObjectStar:
        case QMetaType::QWidgetStar:
            *static_cast<void **>(data) = 0;
            return true;
        case QMetaType::Long:
            *static_cast<long *>(data) = long(0);
            return true;
        case QMetaType::Int:
            *static_cast<int *>(data) = int(0);
            return true;
        case QMetaType::Short:
            *static_cast<short *>(data) = short(0);
            return true;
        case QMetaType::Char:
            *static_cast<char *>(data) = char(0);
            return true;
        case QMetaType::ULong:
            *static_cast<ulong *>(data) = ulong(0);
            return true;
        case QMetaType::UInt:
            *static_cast<uint *>(data) = uint(0);
            return true;
        case QMetaType::LongLong:
            *static_cast<qlonglong *>(data) = qlonglong(0);
            return true;
        case QMetaType::ULongLong:
            *static_cast<qulonglong *>(data) = qulonglong(0);
            return true;
        case QMetaType::UShort:
            *static_cast<ushort *>(data) = ushort(0);
            return true;
        case QMetaType::UChar:
            *static_cast<uchar *>(data) = uchar(0);
            return true;
        case QMetaType::Bool:
            *static_cast<bool *>(data) = bool(false);
            return true;
        case QMetaType::Float:
            *static_cast<float *>(data) = float(0);
            return true;
        case QMetaType::Double:
            *static_cast<double *>(data) = double();
            return true;
        case QMetaType::QChar:
            *static_cast<NS(QChar) *>(data) = NS(QChar)();
            return true;
        case QMetaType::QVariantMap:
            *static_cast<NS(QVariantMap) *>(data) = NS(QVariantMap)();
            return true;
        case QMetaType::QVariantHash:
            *static_cast<NS(QVariantHash) *>(data) = NS(QVariantHash)();
            return true;
        case QMetaType::QVariantList:
            *static_cast<NS(QVariantList) *>(data) = NS(QVariantList)();
            return true;
        case QMetaType::QByteArray:
            *static_cast<NS(QByteArray) *>(data) = NS(QByteArray)();
            return true;
        case QMetaType::QString:
            *static_cast<NS(QString) *>(data) = NS(QString)();
            return true;
        case QMetaType::QStringList:
            *static_cast<NS(QStringList) *>(data) = NS(QStringList)();
            return true;
        case QMetaType::QBitArray:
            *static_cast<NS(QBitArray) *>(data) = NS(QBitArray)();
            return true;
        case QMetaType::QDate:
            *static_cast<NS(QDate) *>(data) = NS(QDate)();
            return true;
        case QMetaType::QTime:
            *static_cast<NS(QTime) *>(data) = NS(QTime)();
            return true;
        case QMetaType::QDateTime:
            *static_cast<NS(QDateTime) *>(data) = NS(QDateTime)();
            return true;
        case QMetaType::QUrl:
            *static_cast<NS(QUrl) *>(data) = NS(QUrl)();
            return true;
        case QMetaType::QLocale:
            *static_cast<NS(QLocale) *>(data) = NS(QLocale)();
            return true;
        case QMetaType::QRect:
            *static_cast<NS(QRect) *>(data) = NS(QRect)();
            return true;
        case QMetaType::QRectF:
            *static_cast<NS(QRectF) *>(data) = NS(QRectF)();
            return true;
        case QMetaType::QSize:
            *static_cast<NS(QSize) *>(data) = NS(QSize)();
            return true;
        case QMetaType::QSizeF:
            *static_cast<NS(QSizeF) *>(data) = NS(QSizeF)();
            return true;
        case QMetaType::QLine:
            *static_cast<NS(QLine) *>(data) = NS(QLine)();
            return true;
        case QMetaType::QLineF:
            *static_cast<NS(QLineF) *>(data) = NS(QLineF)();
            return true;
        case QMetaType::QPoint:
            *static_cast<NS(QPoint) *>(data) = NS(QPoint)();
            return true;
        case QMetaType::QPointF:
            *static_cast<NS(QPointF) *>(data) = NS(QPointF)();
            return true;
        case QMetaType::QVector3D:
            *static_cast<NS(QVector3D) *>(data) = NS(QVector3D)();
            return true;
#ifndef QT_NO_REGEXP
        case QMetaType::QRegExp:
            *static_cast<NS(QRegExp) *>(data) = NS(QRegExp)();
            return true;
#endif
        case QMetaType::Void:
            return true;

#ifdef QT3_SUPPORT
        case QMetaType::QColorGroup:
            *static_cast<NS(QColorGroup) *>(data) = NS(QColorGroup)();
            return true;
#endif

        case QMetaType::QFont:
            *static_cast<NS(QFont) *>(data) = NS(QFont)();
            return true;
        case QMetaType::QPixmap:
            *static_cast<NS(QPixmap) *>(data) = NS(QPixmap)();
            return true;
        case QMetaType::QBrush:
            *static_cast<NS(QBrush) *>(data) = NS(QBrush)();
            return true;
        case QMetaType::QColor:
            *static_cast<NS(QColor) *>(data) = NS(QColor)();
            return true;
        case QMetaType::QPalette:
            *static_cast<NS(QPalette) *>(data) = NS(QPalette)();
            return true;
        case QMetaType::QIcon:
            *static_cast<NS(QIcon) *>(data) = NS(QIcon)();
            return true;
        case QMetaType::QImage:
            *static_cast<NS(QImage) *>(data) = NS(QImage)();
            return true;
        case QMetaType::QPolygon:
            *static_cast<NS(QPolygon) *>(data) = NS(QPolygon)();
            return true;
        case QMetaType::QRegion:
            *static_cast<NS(QRegion) *>(data) = NS(QRegion)();
            return true;
        case QMetaType::QBitmap:
            *static_cast<NS(QBitmap) *>(data) = NS(QBitmap)();
            return true;
        case QMetaType::QCursor:
            *static_cast<NS(QCursor) *>(data) = NS(QCursor)();
            return true;
        case QMetaType::QSizePolicy:
            *static_cast<NS(QSizePolicy) *>(data) = NS(QSizePolicy)();
            return true;
        case QMetaType::QKeySequence:
            *static_cast<NS(QKeySequence) *>(data) = NS(QKeySequence)();
            return true;
        case QMetaType::QPen:
            *static_cast<NS(QPen) *>(data) = NS(QPen)();
            return true;
        case QMetaType::QTextLength:
            *static_cast<NS(QTextLength) *>(data) = NS(QTextLength)();
            return true;
        case QMetaType::QTextFormat:
            *static_cast<NS(QTextFormat) *>(data) = NS(QTextFormat)();
            return true;
        case QMetaType::QMatrix:
            *static_cast<NS(QMatrix) *>(data) = NS(QMatrix)();
            return true;
        case QMetaType::QTransform:
            *static_cast<NS(QTransform) *>(data) = NS(QTransform)();
            return true;
        case QMetaType::QMatrix4x4:
            *static_cast<NS(QMatrix4x4) *>(data) = NS(QMatrix4x4)();
            return true;
        case QMetaType::QVector2D:
            *static_cast<NS(QVector2D) *>(data) = NS(QVector2D)();
            return true;
        case QMetaType::QVector4D:
            *static_cast<NS(QVector4D) *>(data) = NS(QVector4D)();
            return true;
        case QMetaType::QQuaternion:
            *static_cast<NS(QQuaternion) *>(data) = NS(QQuaternion)();
            return true;
        default:
            if (type == qMetaTypeId<QVariant>()) {
                *static_cast<NS(QVariant) *>(data) = NS(QVariant)();
                return true;
            } else if (type == qMetaTypeId<QScriptValue>()) {
                *static_cast<NS(QScriptValue) *>(data) = NS(QScriptValue)();
                return true;
            } else if (typeCategory(type) != Unknown) {
                *static_cast<void **>(data) = 0;
                return true;
            }
            break;
        }
    }

    return false;
}

QT_END_NAMESPACE
