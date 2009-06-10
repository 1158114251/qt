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

#include "qmlvme_p.h"
#include <qfxperf.h>
#include <private/qmlboundsignal_p.h>
#include <private/qmlstringconverters_p.h>
#include "private/qmetaobjectbuilder_p.h"
#include <qml.h>
#include <private/qmlcustomparser_p.h>
#include <qperformancelog.h>
#include <QStack>
#include <QWidget>
#include <private/qmlcompiledcomponent_p.h>
#include <QColor>
#include <QPointF>
#include <QSizeF>
#include <QRectF>
#include <qmlengine.h>
#include <qmlcontext.h>
#include <qmlcomponent.h>
#include <qmlbindablevalue.h>
#include <private/qmlengine_p.h>
#include <private/qmlcomponent_p.h>
#include "private/qmlvmemetaobject_p.h"
#include <QtCore/qdebug.h>
#include <QtCore/qvarlengtharray.h>
#include <private/qmlbindablevalue_p.h>

QT_BEGIN_NAMESPACE

QmlVME::QmlVME()
{
}

#define VME_EXCEPTION(desc) \
    { \
        QString str; \
        QDebug d(&str); \
        d << desc; \
        str = str.trimmed(); \
        QmlError error; \
        error.setDescription(str); \
        error.setLine(instr.line); \
        error.setUrl(comp->url); \
        vmeErrors << error; \
        break; \
    }

struct ListInstance
{
    ListInstance() {}
    /*
    ListInstance(const QVariant &l, int t)
        : list(l), type(t), qmlListInterface(0) {}
        */
    ListInstance(QList<void *> *q, int t)
        : type(t), qListInterface(q) {}
    ListInstance(QmlPrivate::ListInterface *q, int t)
        : type(t), qmlListInterface(q) {}

    //QVariant list;
    int type;
    QList<void *> *qListInterface;
    QmlPrivate::ListInterface *qmlListInterface;
};

QObject *QmlVME::run(QmlContext *ctxt, QmlCompiledComponent *comp, int start, int count)
{
    // XXX - All instances of QmlContext::activeContext() here should be 
    // replaced with the use of ctxt.  However, this cannot be done until 
    // behaviours stop modifying the active context and expecting the 
    // instantiation to notice.  Instead, QmlParserStatus::beginClass() should 
    // be able to return a QmlContext that is used for expressions and 
    // sub-instances on that type.
    Q_ASSERT(comp);
    Q_ASSERT(ctxt);
    const QList<QmlCompiledComponent::TypeReference> &types = comp->types;
    const QList<QString> &primitives = comp->primitives;
    const QList<QByteArray> &datas = comp->datas;
    const QList<QMetaObject *> &synthesizedMetaObjects = comp->synthesizedMetaObjects;;
    const QList<QmlCompiledData::CustomTypeData> &customTypeData = comp->customTypeData;
    const QList<int> &intData = comp->intData;
    const QList<float> &floatData = comp->floatData;


    QmlEnginePrivate::SimpleList<QmlBindableValue> bindValues;
    QmlEnginePrivate::SimpleList<QmlParserStatus> parserStatus;

    QStack<QObject *> stack;
    QStack<ListInstance> qliststack;

    QStack<QmlMetaProperty> pushedProperties;
    QObject **savedObjects = 0;

    vmeErrors.clear();

    if (start == -1) start = 0;
    if (count == -1) count = comp->bytecode.count();

    for (int ii = start; !isError() && ii < (start + count); ++ii) {
        QmlInstruction &instr = comp->bytecode[ii];

        switch(instr.type) {
        case QmlInstruction::Init:
            {
                if (instr.init.dataSize) {
                    savedObjects = new QObject*[instr.init.dataSize];
                    ::memset(savedObjects, 0,
                            sizeof(QObject *)*instr.init.dataSize);
                }

                if (instr.init.bindingsSize) 
                    bindValues = QmlEnginePrivate::SimpleList<QmlBindableValue>(instr.init.bindingsSize);
                if (instr.init.parserStatusSize)
                    parserStatus = QmlEnginePrivate::SimpleList<QmlParserStatus>(instr.init.parserStatusSize);
            }
            break;

        case QmlInstruction::CreateObject:
            {
                QObject *o = types.at(instr.create.type).createInstance(QmlContext::activeContext());
                if (!o) {
                    if(types.at(instr.create.type).component)
                        vmeErrors << types.at(instr.create.type).component->errors();

                    VME_EXCEPTION("Unable to create object of type" << types.at(instr.create.type).className);
                }

                if (instr.create.data != -1) {
                    QmlCustomParser *customParser =
                        types.at(instr.create.type).type->customParser();
                    customParser->setCustomData(o, datas.at(instr.create.data));
                }
                if (!stack.isEmpty()) {
                    QObject *parent = stack.top();
                    o->setParent(parent);
                }
                stack.push(o);
            }
            break;

        case QmlInstruction::SetId:
            {
                QObject *target = stack.top();
                QmlContext *ctxt = 
                    QmlContext::activeContext();
                ctxt->setContextProperty(primitives.at(instr.setId.value), target);

                if (instr.setId.save != -1)
                    savedObjects[instr.setId.save] = target;
            }
            break;


        case QmlInstruction::SetDefault:
            {
                QObject *target = stack.top();
                QmlContext::activeContext()->addDefaultObject(target);
            }
            break;

        case QmlInstruction::CreateComponent:
            {
                QObject *qcomp = new QmlComponent(ctxt->engine(), comp, ii + 1, instr.createComponent.count, stack.isEmpty() ? 0 : stack.top());
                stack.push(qcomp);
                ii += instr.createComponent.count;
            }
            break;

        case QmlInstruction::StoreMetaObject:
            {
                QObject *target = stack.top();
                new QmlVMEMetaObject(target, synthesizedMetaObjects.at(instr.storeMeta.data), &comp->primitives, instr.storeMeta.slotData, comp);
            }
            break;

        case QmlInstruction::StoreVariant:
            {
                QObject *target = stack.top();
                void *a[1];
                // XXX - can be more efficient
                QVariant v = QmlStringConverters::variantFromString(primitives.at(instr.storeString.value));
                a[0] = (void *)&v;
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeString.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreString:
            {
                QObject *target = stack.top();
                void *a[1];
                a[0] = (void *)&primitives.at(instr.storeString.value);
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeString.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreFloat:
            {
                QObject *target = stack.top();
                float f = instr.storeFloat.value;
                void *a[1];
                a[0] = &f;
                QMetaObject::metacall(target, QMetaObject::WriteProperty,
                                      instr.storeFloat.propertyIndex, a);
            }
            break;

case QmlInstruction::StoreDouble:
            {
                QObject *target = stack.top();
                double d = instr.storeDouble.value;
                void *a[1];
                a[0] = &d;
                QMetaObject::metacall(target, QMetaObject::WriteProperty,
                                      instr.storeDouble.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreBool:
            {
                QObject *target = stack.top();
                void *a[1];
                a[0] = (void *)&instr.storeBool.value;
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeBool.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreInteger:
            {
                QObject *target = stack.top();
                void *a[1];
                a[0] = (void *)&instr.storeInteger.value;
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeInteger.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreColor:
            {
                QObject *target = stack.top();
                void *a[1];
                QColor c = QColor::fromRgba(instr.storeColor.value);
                a[0] = (void *)&c;
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeColor.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreDate:
            {
                QObject *target = stack.top();
                void *a[1];
                QDate d = QDate::fromJulianDay(instr.storeDate.value);
                a[0] = (void *)&d;
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeDate.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreTime:
            {
                QObject *target = stack.top();
                void *a[1];
                QTime t;
                t.setHMS(intData.at(instr.storeTime.valueIndex),
                         intData.at(instr.storeTime.valueIndex+1),
                         intData.at(instr.storeTime.valueIndex+2),
                         intData.at(instr.storeTime.valueIndex+3));
                a[0] = (void *)&t;
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeTime.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreDateTime:
            {
                QObject *target = stack.top();
                void *a[1];
                QTime t;
                t.setHMS(intData.at(instr.storeDateTime.valueIndex+1),
                         intData.at(instr.storeDateTime.valueIndex+2),
                         intData.at(instr.storeDateTime.valueIndex+3),
                         intData.at(instr.storeDateTime.valueIndex+4));
                QDateTime dt(QDate::fromJulianDay(intData.at(instr.storeDateTime.valueIndex)), t);
                a[0] = (void *)&dt;
                QMetaObject::metacall(target, QMetaObject::WriteProperty,
                                      instr.storeDateTime.propertyIndex, a);
            }
            break;

        case QmlInstruction::StorePoint:
            {
                QObject *target = stack.top();
                void *a[1];
                QPoint p = QPointF(floatData.at(instr.storeRealPair.valueIndex),
                                   floatData.at(instr.storeRealPair.valueIndex+1)).toPoint();
                a[0] = (void *)&p;
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeRealPair.propertyIndex, a);
            }
            break;

        case QmlInstruction::StorePointF:
            {
                QObject *target = stack.top();
                void *a[1];
                QPointF p(floatData.at(instr.storeRealPair.valueIndex),
                          floatData.at(instr.storeRealPair.valueIndex+1));
                a[0] = (void *)&p;
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeRealPair.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreSize:
            {
                QObject *target = stack.top();
                void *a[1];
                QSize p = QSizeF(floatData.at(instr.storeRealPair.valueIndex),
                                 floatData.at(instr.storeRealPair.valueIndex+1)).toSize();
                a[0] = (void *)&p;
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeRealPair.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreSizeF:
            {
                QObject *target = stack.top();
                void *a[1];
                QSizeF s(floatData.at(instr.storeRealPair.valueIndex),
                         floatData.at(instr.storeRealPair.valueIndex+1));
                a[0] = (void *)&s;
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeRealPair.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreRect:
            {
                QObject *target = stack.top();
                void *a[1];
                QRect r = QRectF(floatData.at(instr.storeRect.valueIndex),
                                 floatData.at(instr.storeRect.valueIndex+1),
                                 floatData.at(instr.storeRect.valueIndex+2),
                                 floatData.at(instr.storeRect.valueIndex+3)).toRect();
                a[0] = (void *)&r;
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeRect.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreRectF:
            {
                QObject *target = stack.top();
                void *a[1];
                QRectF r(floatData.at(instr.storeRect.valueIndex),
                         floatData.at(instr.storeRect.valueIndex+1),
                         floatData.at(instr.storeRect.valueIndex+2),
                         floatData.at(instr.storeRect.valueIndex+3));
                a[0] = (void *)&r;
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeRect.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreObject:
            {
                QObject *assignObj = stack.pop();
                QObject *target = stack.top();

                void *a[1];
                a[0] = (void *)&assignObj;

                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeObject.propertyIndex, a);
            }
            break;


        case QmlInstruction::AssignCustomType:
            {
                QObject *target = stack.top();
                void *a[1];
                QmlCompiledComponent::CustomTypeData data = customTypeData.at(instr.assignCustomType.valueIndex);
                const QString &primitive = primitives.at(data.index);
                QmlMetaType::StringConverter converter = 
                    QmlMetaType::customStringConverter(data.type);
                QVariant v = (*converter)(primitive);

                QMetaProperty prop = 
                        target->metaObject()->property(instr.assignCustomType.propertyIndex);
                if (v.isNull() || ((int)prop.type() != data.type && prop.userType() != data.type)) 
                    VME_EXCEPTION("Cannot assign value" << primitive << "to property" << prop.name());

                a[0] = (void *)v.data();
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.assignCustomType.propertyIndex, a);
            }
            break;

        case QmlInstruction::AssignSignalObject:
            {
                // XXX optimize

                QObject *assign = stack.pop();
                QObject *target = stack.top();
                int sigIdx = instr.assignSignalObject.signal;
                const QByteArray &pr = datas.at(sigIdx);

                QmlMetaProperty prop(target, QLatin1String(pr));
                if (prop.type() & QmlMetaProperty::SignalProperty) {

                    QMetaMethod method = QmlMetaType::defaultMethod(assign);
                    if (method.signature() == 0)
                        VME_EXCEPTION("Cannot assign object type" << assign->metaObject()->className() << "with no default method");

                    if (!QMetaObject::checkConnectArgs(prop.method().signature(), method.signature()))
                        VME_EXCEPTION("Cannot connect mismatched signal/slot" << method.signature() << prop.method().signature());

                    QMetaObject::connect(target, prop.coreIndex(), assign, method.methodIndex());

                } else {
                    VME_EXCEPTION("Cannot assign an object to signal property" << pr);
                }


            }
            break;

        case QmlInstruction::StoreSignal:
            {
                QObject *target = stack.top();
                // XXX scope
                QMetaMethod signal = 
                    target->metaObject()->method(instr.storeSignal.signalIndex);

                if (signal.parameterTypes().isEmpty()) {
                    (void *)new QmlBoundSignal(QmlContext::activeContext(), primitives.at(instr.storeSignal.value), target, instr.storeSignal.signalIndex, target);
                } else {
                    (void *)new QmlBoundSignalProxy(new QmlContext(QmlContext::activeContext(), target), primitives.at(instr.storeSignal.value), target, instr.storeSignal.signalIndex, target);
                }
            }
            break;

        case QmlInstruction::BeginObject:
            {
                QObject *target = stack.top();
                QmlParserStatus *status = reinterpret_cast<QmlParserStatus *>(reinterpret_cast<char *>(target) + instr.begin.castValue);
                parserStatus.append(status);
                status->d = &parserStatus.values[parserStatus.count - 1];

                status->classBegin();
            }
            break;

        case QmlInstruction::CompleteObject:
            {
                QObject *target = stack.top();
                QmlParserStatus *status = reinterpret_cast<QmlParserStatus *>(reinterpret_cast<char *>(target) + instr.complete.castValue);
                status->classComplete();
            }
            break;

        case QmlInstruction::PushProperty:
            {
                QObject *target = stack.top();
                QmlMetaProperty mp(target, instr.pushProperty.property,
                                   QmlMetaProperty::Object);
                pushedProperties.push(mp);
            }
            break;

        case QmlInstruction::StoreCompiledBinding:
            {
                QObject *target = stack.top();
                QObject *context = 
                    stack.at(stack.count() - 1 - instr.assignBinding.context);

                QmlMetaProperty mp(target, instr.assignBinding.property,
                                   (QmlMetaProperty::PropertyCategory)instr.assignBinding.category);

                QmlBindableValue *bind = new QmlBindableValue((void *)datas.at(instr.assignBinding.value).constData(), comp, context, 0);
                bindValues.append(bind);
                QmlBindableValuePrivate *p = 
                    static_cast<QmlBindableValuePrivate *>(QObjectPrivate::get(bind));
                p->mePtr = &bindValues.values[bindValues.count - 1];
                QFx_setParent_noEvent(bind, target);

                bind->setTarget(mp);
                bind->setSourceLocation(comp->url, instr.line);
            }
            break;

        case QmlInstruction::StoreBinding:
            {
                QObject *target = stack.top();
                QObject *context = 
                    stack.at(stack.count() - 1 - instr.assignBinding.context);

                QmlMetaProperty mp(target, instr.assignBinding.property, 
                                   (QmlMetaProperty::PropertyCategory)instr.assignBinding.category);

                QmlBindableValue *bind = new QmlBindableValue(primitives.at(instr.assignBinding.value), context, false);
                bindValues.append(bind);
                QmlBindableValuePrivate *p = 
                    static_cast<QmlBindableValuePrivate *>(QObjectPrivate::get(bind));
                p->mePtr = &bindValues.values[bindValues.count - 1];
                QFx_setParent_noEvent(bind, target);

                bind->setTarget(mp);
                bind->setSourceLocation(comp->url, instr.line);
            }
            break;

        case QmlInstruction::StoreValueSource:
            {
                QObject *assign = stack.pop();
                QmlPropertyValueSource *vs = 
                    static_cast<QmlPropertyValueSource *>(assign);
                QObject *target = stack.top();
                vs->setParent(target);
                vs->setTarget(QmlMetaProperty(target, instr.assignValueSource.property));
            }
            break;

        case QmlInstruction::StoreObjectQmlList:
            {
                QObject *assign = stack.pop();
                const ListInstance &list = qliststack.top();

                void *d = (void *)&assign;
                list.qmlListInterface->append(d);
            }
            break;

        case QmlInstruction::StoreObjectQList:
            {
                QObject *assign = stack.pop();

                const ListInstance &list = qliststack.top();
                list.qListInterface->append((void *)assign);
            }
            break;

        case QmlInstruction::AssignObjectList:
            {
                // This is only used for assigning interfaces
                QObject *assign = stack.pop();
                const ListInstance &list = qliststack.top();

                int type = list.type;

                void *ptr = 0;

                const char *iid = QmlMetaType::interfaceIId(type);
                if (iid) 
                    ptr = assign->qt_metacast(iid);
                if (!ptr) 
                    VME_EXCEPTION("Cannot assign object to list");


                if (list.qmlListInterface) {
                    void *d = (void *)&ptr;
                    list.qmlListInterface->append(d);
                } else {
                    list.qListInterface->append(ptr);
                }
            }
            break;

        case QmlInstruction::StoreVariantObject:
            {
                QObject *assign = stack.pop();
                QObject *target = stack.top();

                QVariant v = QVariant::fromValue(assign);
                void *a[1];
                a[0] = (void *)&v;
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeObject.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreInterface:
            {
                QObject *assign = stack.pop();
                QObject *target = stack.top();

                int coreIdx = instr.storeObject.propertyIndex;
                QMetaProperty prop = target->metaObject()->property(coreIdx);
                int t = prop.userType();
                const char *iid = QmlMetaType::interfaceIId(t);
                bool ok = false;
                if (iid) {
                    void *ptr = assign->qt_metacast(iid);
                    if (ptr) {
                        void *a[1];
                        a[0] = &ptr;
                        QMetaObject::metacall(target, 
                                              QMetaObject::WriteProperty,
                                              coreIdx, a);
                        ok = true;
                    }
                } 

                if (!ok) 
                    VME_EXCEPTION("Cannot assign object to interface property");
            }
            break;
            
        case QmlInstruction::FetchAttached:
            {
                QObject *target = stack.top();

                QObject *qmlObject = qmlAttachedPropertiesObjectById(instr.fetchAttached.id, target);

                if (!qmlObject)
                    VME_EXCEPTION("Unable to create attached object");

                stack.push(qmlObject);
            }
            break;

        case QmlInstruction::FetchQmlList:
            {
                QObject *target = stack.top();

                void *a[1];
                // We know that QmlList<*> can be converted to 
                // QmlPrivate::ListInterface
                QmlPrivate::ListInterface *list = 0;
                a[0] = &list;
                QMetaObject::metacall(target, QMetaObject::ReadProperty, 
                                      instr.fetchQmlList.property, a);
                if (!list) 
                    VME_EXCEPTION("Cannot assign to null list");

                qliststack.push(ListInstance(list, instr.fetchQmlList.type));
            }
            break;

        case QmlInstruction::FetchQList:
            {
                QObject *target = stack.top();

                void *a[1];
                // We know that QList<T *>* can be converted to 
                // QList<void *>*
                QList<void *> *list = 0;
                a[0] = &list;
                QMetaObject::metacall(target, QMetaObject::ReadProperty, 
                                      instr.fetchQmlList.property, a);
                if (!list) 
                    VME_EXCEPTION("Cannot assign to null list");

                qliststack.push(ListInstance(list, instr.fetchQmlList.type));
            }
            break;

        case QmlInstruction::FetchObject:
            {
                QObject *target = stack.top();

                QObject *obj = 0;
                if (instr.fetch.isObject) {
                    // NOTE: This assumes a cast to QObject does not alter the 
                    // object pointer
                    void *a[1];
                    a[0] = &obj;
                    QMetaObject::metacall(target, QMetaObject::ReadProperty, 
                                          instr.fetch.property, a);
                } else {
                    void *a[1];
                    QVariant var;
                    a[0] = &var;
                    QMetaObject::metacall(target, QMetaObject::ReadProperty, 
                                          instr.fetch.property, a);
                    obj = QmlMetaType::toQObject(var);

                }

                if (!obj)
                    VME_EXCEPTION("Cannot set properties on" << target->metaObject()->property(instr.fetch.property).name() << "as it is null");

                stack.push(obj);
            }
            break;

        case QmlInstruction::PopQList:
            {
                qliststack.pop();
            }
            break;

        case QmlInstruction::PopFetchedObject:
            {
                stack.pop();
            }
            break;

        case QmlInstruction::StoreStackObject:
            {
                const QmlMetaProperty &prop = 
                    pushedProperties.at(instr.assignStackObject.property);
                QObject *obj = savedObjects[instr.assignStackObject.object];

                // NOTE: This assumes a cast to QObject does not alter the 
                // object pointer
                void *a[1];
                a[0] = (void *)&obj;
                QMetaObject::metacall(prop.object(), QMetaObject::WriteProperty,
                                      prop.coreIndex(), a);
            }
            break;

        default:
            qFatal("QmlCompiledComponent: Internal error - unknown instruction %d", instr.type);
            break;
        }
    }

    if (isError()) {
        if (!stack.isEmpty()) {
            delete stack.at(0);
        }

        QmlEnginePrivate::clear(bindValues);
        QmlEnginePrivate::clear(parserStatus);
        return 0;
    }

    QmlEnginePrivate *ep = ctxt->engine()->d_func();
    if (bindValues.count)
        ep->bindValues << bindValues;
    if (parserStatus.count)
        ep->parserStatus << parserStatus;

    comp->dumpPost();

    if (savedObjects)
        delete [] savedObjects;

    if (stack.isEmpty())
        return 0;
    else
        return stack.top();
    return 0;
}

bool QmlVME::isError() const
{
    return !vmeErrors.isEmpty();
}

QList<QmlError> QmlVME::errors() const
{
    return vmeErrors;
}

QT_END_NAMESPACE
