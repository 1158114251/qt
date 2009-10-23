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

#include "qmlvmemetaobject_p.h"
#include <qml.h>
#include <private/qmlrefcount_p.h>
#include <QColor>
#include <QDate>
#include <QtCore/qlist.h>
#include <QtCore/qdebug.h>
#include <qmlexpression.h>
#include <private/qmlcontext_p.h>

QT_BEGIN_NAMESPACE

QmlVMEMetaObject::QmlVMEMetaObject(QObject *obj,
                                   const QMetaObject *other, 
                                   const QmlVMEMetaData *meta,
                                   QmlRefCount *rc)
: object(obj), ref(rc), ctxt(qmlContext(obj)), metaData(meta), parent(0)
{
    if (ref)
        ref->addref();

    *static_cast<QMetaObject *>(this) = *other;
    this->d.superdata = obj->metaObject();

    QObjectPrivate *op = QObjectPrivate::get(obj);
    if (op->metaObject)
        parent = static_cast<QAbstractDynamicMetaObject*>(op->metaObject);
    op->metaObject = this;

    propOffset = QAbstractDynamicMetaObject::propertyOffset();
    methodOffset = QAbstractDynamicMetaObject::methodOffset();

    data = new QVariant[metaData->propertyCount];
    aConnected.resize(metaData->aliasCount);

    int list_type = qMetaTypeId<QmlList<QObject*>* >();
    // ### Optimize
    for (int ii = 0; ii < metaData->propertyCount; ++ii) {
        int t = (metaData->propertyData() + ii)->propertyType;
        if (t == list_type) {
            listProperties.append(new List(this, ii));
            data[ii] = QVariant::fromValue((QmlList<QObject *>*)listProperties.last());
        } else if (t != -1) {
            data[ii] = QVariant((QVariant::Type)t);
        }
    }
}

QmlVMEMetaObject::~QmlVMEMetaObject()
{
    if (ref)
        ref->release();
    if (parent)
        delete parent;
    qDeleteAll(listProperties);
    delete [] data;
}

int QmlVMEMetaObject::metaCall(QMetaObject::Call c, int _id, void **a)
{
    int id = _id;
    if(c == QMetaObject::WriteProperty) {
        int flags = *reinterpret_cast<int*>(a[3]);
        if (!(flags & QmlMetaProperty::BypassInterceptor)
            && !aInterceptors.isEmpty()
            && aInterceptors.testBit(id)) {
            QPair<int, QmlPropertyValueInterceptor*> pair = interceptors.value(id);
            int valueIndex = pair.first;
            QmlPropertyValueInterceptor *vi = pair.second;
            QVariant::Type type = QVariant::Invalid;
            if (id >= propOffset) {
                id -= propOffset;
                if (id < metaData->propertyCount) {
                    type = data[id].type();
                }
            } else {
                type = property(id).type();
            }

            if (type != QVariant::Invalid) {
                if (valueIndex != -1) {
                    QmlEnginePrivate *ep = ctxt?QmlEnginePrivate::get(ctxt->engine()):0;
                    QmlValueType *valueType = 0;
                    if (ep) valueType = ep->valueTypes[type];
                    else valueType = QmlValueTypeFactory::valueType(type);
                    Q_ASSERT(valueType);

                    valueType->setValue(QVariant(type, a[0]));
                    QMetaProperty valueProp = valueType->metaObject()->property(valueIndex);
                    vi->write(valueProp.read(valueType));
                    return -1;
                } else {
                    vi->write(QVariant(type, a[0]));
                    return -1;
                }
            }
        }
    }
    if(c == QMetaObject::ReadProperty || c == QMetaObject::WriteProperty) {
        if (id >= propOffset) {
            id -= propOffset;

            if (id < metaData->propertyCount) {
                int t = (metaData->propertyData() + id)->propertyType;
                bool needActivate = false;

                if (t == -1) {

                    if (c == QMetaObject::ReadProperty) {
                        *reinterpret_cast<QVariant *>(a[0]) = data[id];
                    } else if (c == QMetaObject::WriteProperty) {
                        needActivate = 
                            (data[id] != *reinterpret_cast<QVariant *>(a[0]));
                        data[id] = *reinterpret_cast<QVariant *>(a[0]);
                    }

                } else {

                    if (c == QMetaObject::ReadProperty) {
                        switch(t) {
                        case QVariant::Int:
                            *reinterpret_cast<int *>(a[0]) = data[id].toInt();
                            break;
                        case QVariant::Bool:
                            *reinterpret_cast<bool *>(a[0]) = data[id].toBool();
                            break;
                        case QVariant::Double:
                            *reinterpret_cast<double *>(a[0]) = data[id].toDouble();
                            break;
                        case QVariant::String:
                            *reinterpret_cast<QString *>(a[0]) = data[id].toString();
                            break;
                        case QVariant::Url:
                            *reinterpret_cast<QUrl *>(a[0]) = data[id].toUrl();
                            break;
                        case QVariant::Color:
                            *reinterpret_cast<QColor *>(a[0]) = data[id].value<QColor>();
                            break;
                        case QVariant::Date:
                            *reinterpret_cast<QDate *>(a[0]) = data[id].toDate();
                            break;
                        case QMetaType::QObjectStar:
                            *reinterpret_cast<QObject **>(a[0]) = data[id].value<QObject*>();
                            break;
                        default:
                            break;
                        }
                        if (t == qMetaTypeId<QmlList<QObject*>* >()) {
                            *reinterpret_cast<QmlList<QObject *> **>(a[0]) = data[id].value<QmlList<QObject*>*>();
                        }

                    } else if (c == QMetaObject::WriteProperty) {

                        QVariant value = QVariant((QVariant::Type)data[id].type(), a[0]); 
                        needActivate = (data[id] != value);
                        data[id] = value;
                    }

                }

                if (c == QMetaObject::WriteProperty && needActivate) {
                    activate(object, methodOffset + id, 0);
                }

                return -1;
            }

            id -= metaData->propertyCount;

            if (id < metaData->aliasCount) {

                if (!ctxt) return -1;
                QmlVMEMetaData::AliasData *d = metaData->aliasData() + id;
                QmlContextPrivate *ctxtPriv = 
                    (QmlContextPrivate *)QObjectPrivate::get(ctxt);

                QObject *target = ctxtPriv->idValues[d->contextIdx].data();
                if (!target) {
                    if (d->propertyIdx == -1) 
                        *reinterpret_cast<QObject **>(a[0]) = target;
                    return -1;
                }

                if (c == QMetaObject::ReadProperty && !aConnected.testBit(id)) {
                    int sigIdx = methodOffset + id + metaData->propertyCount;
                    QMetaObject::connect(ctxt, d->contextIdx + ctxtPriv->notifyIndex, object, sigIdx);

                    if (d->propertyIdx != -1) {
                        QMetaProperty prop = 
                            target->metaObject()->property(d->propertyIdx);
                        if (prop.hasNotifySignal())
                            QMetaObject::connect(target, prop.notifySignalIndex(), 
                                                 object, sigIdx);
                    }
                    aConnected.setBit(id);
                }

                if (d->propertyIdx == -1) {
                    *reinterpret_cast<QObject **>(a[0]) = target;
                    return -1;
                } else {
                    return QMetaObject::metacall(target, c, d->propertyIdx, a);
                }

            }
            return -1;

        }

    } else if(c == QMetaObject::InvokeMetaMethod) {

        if (id >= methodOffset) {

            id -= methodOffset;
            int plainSignals = metaData->signalCount + metaData->propertyCount +
                               metaData->aliasCount;
            if (id < plainSignals) {
                QMetaObject::activate(object, _id, a);
                return -1;
            }

            id -= plainSignals;

            if (id < metaData->methodCount) {
                QmlVMEMetaData::MethodData *data = metaData->methodData() + id;
                const QChar *body = 
                    (const QChar *)(((const char*)metaData) + data->bodyOffset);

                QString code = QString::fromRawData(body, data->bodyLength);

                if (0 == (metaData->methodData() + id)->parameterCount) {
                    QmlExpression expr(ctxt, code, object);
                    expr.setTrackChange(false);
                    expr.value();
                } else {
                    QmlContext newCtxt(ctxt);
                    QMetaMethod m = method(_id);
                    QList<QByteArray> names = m.parameterNames(); 
                    for (int ii = 0; ii < names.count(); ++ii) 
                        newCtxt.setContextProperty(names.at(ii), *(QVariant *)a[ii + 1]);
                    QmlExpression expr(&newCtxt, code, object);
                    expr.setTrackChange(false);
                    expr.value();
                }
            }
            return -1;
        }
    }

    if (parent)
        return parent->metaCall(c, _id, a);
    else
        return object->qt_metacall(c, _id, a);
}

void QmlVMEMetaObject::listChanged(int id)
{
    activate(object, methodOffset + id, 0);
}

void QmlVMEMetaObject::registerInterceptor(int index, int valueIndex, QmlPropertyValueInterceptor *interceptor)
{
    if (aInterceptors.isEmpty())
        aInterceptors.resize(propertyCount() + metaData->propertyCount);
    aInterceptors.setBit(index);
    interceptors.insert(index, qMakePair(valueIndex, interceptor));
}


QT_END_NAMESPACE
