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
#include "qmlcompiler_p.h"
#include <private/qfxperf_p.h>
#include <private/qmlboundsignal_p.h>
#include <private/qmlstringconverters_p.h>
#include "private/qmetaobjectbuilder_p.h"
#include "private/qmldeclarativedata_p.h"
#include <qml.h>
#include <private/qmlcustomparser_p.h>
#include <QStack>
#include <QWidget>
#include <QColor>
#include <QPointF>
#include <QSizeF>
#include <QRectF>
#include <qmlengine.h>
#include <qmlcontext.h>
#include <qmlcomponent.h>
#include <qmlbinding.h>
#include <private/qmlengine_p.h>
#include <private/qmlcomponent_p.h>
#include "private/qmlvmemetaobject_p.h"
#include <QtCore/qdebug.h>
#include <QtCore/qvarlengtharray.h>
#include <private/qmlbinding_p.h>
#include <private/qmlcontext_p.h>
#include <private/qmlbindingoptimizations_p.h>

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
    ListInstance(QList<void *> *q, int t)
        : type(t), qListInterface(q), qmlListInterface(0) {}
    ListInstance(QmlPrivate::ListInterface *q, int t)
        : type(t), qListInterface(0), qmlListInterface(q) {}

    int type;
    QList<void *> *qListInterface;
    QmlPrivate::ListInterface *qmlListInterface;
};

QObject *QmlVME::run(QmlContext *ctxt, QmlCompiledData *comp, 
                     int start, int count, 
                     const QBitField &bindingSkipList)
{
    QStack<QObject *> stack;

    if (start == -1) start = 0;
    if (count == -1) count = comp->bytecode.count();

    return run(stack, ctxt, comp, start, count, bindingSkipList);
}

void QmlVME::runDeferred(QObject *object)
{
    QmlDeclarativeData *data = QmlDeclarativeData::get(object);

    if (!data || !data->context || !data->deferredComponent)
        return;

    QmlContext *ctxt = data->context;
    QmlCompiledData *comp = data->deferredComponent;
    int start = data->deferredIdx + 1;
    int count = data->deferredComponent->bytecode.at(data->deferredIdx).defer.deferCount;
    QStack<QObject *> stack;
    stack.push(object);

    run(stack, ctxt, comp, start, count, QBitField());
}

QBitField bindingSkipList;
QObject *QmlVME::run(QStack<QObject *> &stack, QmlContext *ctxt, 
                     QmlCompiledData *comp, 
                     int start, int count, 
                     const QBitField &bindingSkipList)
{
    Q_ASSERT(comp);
    Q_ASSERT(ctxt);
    const QList<QmlCompiledData::TypeReference> &types = comp->types;
    const QList<QString> &primitives = comp->primitives;
    const QList<QByteArray> &datas = comp->datas;
    const QList<QmlCompiledData::CustomTypeData> &customTypeData = comp->customTypeData;
    const QList<int> &intData = comp->intData;
    const QList<float> &floatData = comp->floatData;
    const QList<QmlPropertyCache *> &propertyCaches = comp->propertyCaches;


    QmlEnginePrivate::SimpleList<QmlAbstractBinding> bindValues;
    QmlEnginePrivate::SimpleList<QmlParserStatus> parserStatus;

    QStack<ListInstance> qliststack;

    vmeErrors.clear();
    QmlEnginePrivate *ep = QmlEnginePrivate::get(ctxt->engine());
    QmlContextPrivate *cp = (QmlContextPrivate *)QObjectPrivate::get(ctxt);

    int status = -1;    //for dbus
    QmlMetaProperty::WriteFlags flags = QmlMetaProperty::BypassInterceptor;

    for (int ii = start; !isError() && ii < (start + count); ++ii) {
        QmlInstruction &instr = comp->bytecode[ii];

        switch(instr.type) {
        case QmlInstruction::Init:
            {
                if (instr.init.bindingsSize) 
                    bindValues = QmlEnginePrivate::SimpleList<QmlAbstractBinding>(instr.init.bindingsSize);
                if (instr.init.parserStatusSize)
                    parserStatus = QmlEnginePrivate::SimpleList<QmlParserStatus>(instr.init.parserStatusSize);

                if (instr.init.contextCache != -1) 
                    cp->setIdPropertyData(comp->contextCaches.at(instr.init.contextCache));
            }
            break;

        case QmlInstruction::CreateObject:
            {
                QBitField bindings;
                if (instr.create.bindingBits != -1) {
                    const QByteArray &bits = datas.at(instr.create.bindingBits);
                    bindings = QBitField((const quint32*)bits.constData(),
                                         bits.size() * 8);
                }
                if (stack.isEmpty())
                    bindings = bindings.united(bindingSkipList);

                QObject *o = 
                    types.at(instr.create.type).createInstance(ctxt, bindings);
                if (!o) {
                    if(types.at(instr.create.type).component)
                        vmeErrors << types.at(instr.create.type).component->errors();

                    VME_EXCEPTION("Unable to create object of type" << types.at(instr.create.type).className);
                }

                QmlDeclarativeData *ddata = QmlDeclarativeData::get(o);
                Q_ASSERT(ddata);
                ddata->outerContext = ctxt;
                ddata->lineNumber = instr.line;
                ddata->columnNumber = instr.create.column;

                if (instr.create.data != -1) {
                    QmlCustomParser *customParser =
                        types.at(instr.create.type).type->customParser();
                    customParser->setCustomData(o, datas.at(instr.create.data));
                }
                if (!stack.isEmpty()) {
                    QObject *parent = stack.top();
                    if (o->isWidgetType()) { 
                        QWidget *widget = static_cast<QWidget*>(o); 
                        if (parent->isWidgetType()) { 
                            QWidget *parentWidget = static_cast<QWidget*>(parent); 
                            widget->setParent(parentWidget); 
                        } else { 
                            // TODO: parent might be a layout 
                        } 
                    } else { 
                        o->setParent(parent); 
                    } 
                }
                stack.push(o);
            }
            break;

        case QmlInstruction::SetId:
            {
                QObject *target = stack.top();
//                ctxt->setContextProperty(primitives.at(instr.setId.value), target);
                cp->setIdProperty(primitives.at(instr.setId.value), instr.setId.index, target);
            }
            break;


        case QmlInstruction::SetDefault:
            {
                QObject *target = stack.top();
                ctxt->addDefaultObject(target);
            }
            break;

        case QmlInstruction::CreateComponent:
            {
                QObject *qcomp = new QmlComponent(ctxt->engine(), comp, ii + 1, instr.createComponent.count, stack.isEmpty() ? 0 : stack.top());

                QmlEngine::setContextForObject(qcomp, ctxt);
                QmlDeclarativeData *ddata = QmlDeclarativeData::get(qcomp);
                Q_ASSERT(ddata);
                ddata->outerContext = ctxt;
                ddata->lineNumber = instr.line;
                ddata->columnNumber = instr.create.column;

                stack.push(qcomp);
                ii += instr.createComponent.count;
            }
            break;

        case QmlInstruction::StoreMetaObject:
            {
                QObject *target = stack.top();

                QMetaObject mo;
                const QByteArray &metadata = datas.at(instr.storeMeta.data);
                QMetaObjectBuilder::fromRelocatableData(&mo, 0, metadata);

                const QmlVMEMetaData *data = 
                    (const QmlVMEMetaData *)datas.at(instr.storeMeta.aliasData).constData();

                (void)new QmlVMEMetaObject(target, &mo, data, comp);

                QmlDeclarativeData *ddata = QmlDeclarativeData::get(target, true);
                if (ddata->propertyCache) ddata->propertyCache->release();
                ddata->propertyCache = propertyCaches.at(instr.storeMeta.propertyCache);
                ddata->propertyCache->addref();
            }
            break;

        case QmlInstruction::StoreVariant:
            {
                QObject *target = stack.top();
                // XXX - can be more efficient
                QVariant v = QmlStringConverters::variantFromString(primitives.at(instr.storeString.value));
                void *a[] = { &v, 0, &status, &flags };
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeString.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreString:
            {
                QObject *target = stack.top();
                void *a[] = { (void *)&primitives.at(instr.storeString.value), 0, &status, &flags };
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeString.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreUrl:
            {
                QObject *target = stack.top();
                QUrl u(primitives.at(instr.storeUrl.value));
                void *a[] = { &u, 0, &status, &flags };
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeUrl.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreFloat:
            {
                QObject *target = stack.top();
                float f = instr.storeFloat.value;
                void *a[] = { &f, 0, &status, &flags };
                QMetaObject::metacall(target, QMetaObject::WriteProperty,
                                      instr.storeFloat.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreDouble:
            {
                QObject *target = stack.top();
                double d = instr.storeDouble.value;
                void *a[] = { &d, 0, &status, &flags };
                QMetaObject::metacall(target, QMetaObject::WriteProperty,
                                      instr.storeDouble.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreBool:
            {
                QObject *target = stack.top();
                void *a[] = { (void *)&instr.storeBool.value, 0, &status, &flags };
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeBool.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreInteger:
            {
                QObject *target = stack.top();
                void *a[] = { (void *)&instr.storeInteger.value, 0, &status, &flags };
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeInteger.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreColor:
            {
                QObject *target = stack.top();
                QColor c = QColor::fromRgba(instr.storeColor.value);
                void *a[] = { &c, 0, &status, &flags };
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeColor.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreDate:
            {
                QObject *target = stack.top();
                QDate d = QDate::fromJulianDay(instr.storeDate.value);
                void *a[] = { &d, 0, &status, &flags };
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeDate.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreTime:
            {
                QObject *target = stack.top();
                QTime t;
                t.setHMS(intData.at(instr.storeTime.valueIndex),
                         intData.at(instr.storeTime.valueIndex+1),
                         intData.at(instr.storeTime.valueIndex+2),
                         intData.at(instr.storeTime.valueIndex+3));
                void *a[] = { &t, 0, &status, &flags };
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeTime.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreDateTime:
            {
                QObject *target = stack.top();
                QTime t;
                t.setHMS(intData.at(instr.storeDateTime.valueIndex+1),
                         intData.at(instr.storeDateTime.valueIndex+2),
                         intData.at(instr.storeDateTime.valueIndex+3),
                         intData.at(instr.storeDateTime.valueIndex+4));
                QDateTime dt(QDate::fromJulianDay(intData.at(instr.storeDateTime.valueIndex)), t);
                void *a[] = { &dt, 0, &status, &flags };
                QMetaObject::metacall(target, QMetaObject::WriteProperty,
                                      instr.storeDateTime.propertyIndex, a);
            }
            break;

        case QmlInstruction::StorePoint:
            {
                QObject *target = stack.top();
                QPoint p = QPointF(floatData.at(instr.storeRealPair.valueIndex),
                                   floatData.at(instr.storeRealPair.valueIndex+1)).toPoint();
                void *a[] = { &p, 0, &status, &flags };
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeRealPair.propertyIndex, a);
            }
            break;

        case QmlInstruction::StorePointF:
            {
                QObject *target = stack.top();
                QPointF p(floatData.at(instr.storeRealPair.valueIndex),
                          floatData.at(instr.storeRealPair.valueIndex+1));
                void *a[] = { &p, 0, &status, &flags };
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeRealPair.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreSize:
            {
                QObject *target = stack.top();
                QSize p = QSizeF(floatData.at(instr.storeRealPair.valueIndex),
                                 floatData.at(instr.storeRealPair.valueIndex+1)).toSize();
                void *a[] = { &p, 0, &status, &flags };
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeRealPair.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreSizeF:
            {
                QObject *target = stack.top();
                QSizeF s(floatData.at(instr.storeRealPair.valueIndex),
                         floatData.at(instr.storeRealPair.valueIndex+1));
                void *a[] = { &s, 0, &status, &flags };
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeRealPair.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreRect:
            {
                QObject *target = stack.top();
                QRect r = QRectF(floatData.at(instr.storeRect.valueIndex),
                                 floatData.at(instr.storeRect.valueIndex+1),
                                 floatData.at(instr.storeRect.valueIndex+2),
                                 floatData.at(instr.storeRect.valueIndex+3)).toRect();
                void *a[] = { &r, 0, &status, &flags };
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeRect.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreRectF:
            {
                QObject *target = stack.top();
                QRectF r(floatData.at(instr.storeRect.valueIndex),
                         floatData.at(instr.storeRect.valueIndex+1),
                         floatData.at(instr.storeRect.valueIndex+2),
                         floatData.at(instr.storeRect.valueIndex+3));
                void *a[] = { &r, 0, &status, &flags };
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeRect.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreVector3D:
            {
                QObject *target = stack.top();
                QVector3D p(floatData.at(instr.storeVector3D.valueIndex),
                            floatData.at(instr.storeVector3D.valueIndex+1),
                            floatData.at(instr.storeVector3D.valueIndex+2));
                void *a[] = { &p, 0, &status, &flags };
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeVector3D.propertyIndex, a);
            }
            break;

        case QmlInstruction::StoreObject:
            {
                QObject *assignObj = stack.pop();
                QObject *target = stack.top();

                void *a[] = { (void *)&assignObj, 0, &status, &flags };
                QMetaObject::metacall(target, QMetaObject::WriteProperty, 
                                      instr.storeObject.propertyIndex, a);
            }
            break;


        case QmlInstruction::AssignCustomType:
            {
                QObject *target = stack.top();
                QmlCompiledData::CustomTypeData data = customTypeData.at(instr.assignCustomType.valueIndex);
                const QString &primitive = primitives.at(data.index);
                QmlMetaType::StringConverter converter = 
                    QmlMetaType::customStringConverter(data.type);
                QVariant v = (*converter)(primitive);

                QMetaProperty prop = 
                        target->metaObject()->property(instr.assignCustomType.propertyIndex);
                if (v.isNull() || ((int)prop.type() != data.type && prop.userType() != data.type)) 
                    VME_EXCEPTION("Cannot assign value" << primitive << "to property" << prop.name());

                void *a[] = { (void *)v.data(), 0, &status, &flags };
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

                (void *)new QmlBoundSignal(ctxt, primitives.at(instr.storeSignal.value), target, signal, target);
            }
            break;

        case QmlInstruction::StoreScript:
            {
                QObject *target = stack.top();
                cp->addScript(primitives.at(instr.storeScript.value), target);
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

        case QmlInstruction::StoreBinding:
            {
                QObject *target = 
                    stack.at(stack.count() - 1 - instr.assignBinding.owner);
                QObject *context = 
                    stack.at(stack.count() - 1 - instr.assignBinding.context);

                QmlMetaProperty mp;
                mp.restore(instr.assignBinding.property, target, ctxt);

                int coreIndex = mp.coreIndex();

                if (stack.count() == 1 && bindingSkipList.testBit(coreIndex))  
                    break;

                QmlBinding *bind = new QmlBinding((void *)datas.at(instr.assignBinding.value).constData(), comp, context, ctxt, 0);
                bindValues.append(bind);
                bind->m_mePtr = &bindValues.values[bindValues.count - 1];
                bind->setTarget(mp);
                bind->addToObject(target);
                bind->setSourceLocation(comp->url, instr.line);
            }
            break;

        case QmlInstruction::StoreIdOptBinding:
            {
                int coreIndex = instr.assignIdOptBinding.property;
                if (stack.count() == 1 && bindingSkipList.testBit(coreIndex)) 
                    break;

                QObject *target = stack.top();

                QmlBinding_Id *bind = 
                    new QmlBinding_Id(target, instr.assignIdOptBinding.property,
                                      ctxt, instr.assignIdOptBinding.id);
                bindValues.append(bind);
                bind->m_mePtr = &bindValues.values[bindValues.count - 1];
                bind->addToObject(target);
            } 
            break;

        case QmlInstruction::StoreObjPropBinding:
            {
                int coreIndex = instr.assignObjPropBinding.property;
                if (stack.count() == 1 && bindingSkipList.testBit(coreIndex)) 
                    break;

                QObject *target = stack.top();
                QObject *context = 
                    stack.at(stack.count() - 1 - instr.assignObjPropBinding.context);

                QmlBinding_ObjProperty *bind =
                    new QmlBinding_ObjProperty(target, instr.assignObjPropBinding.property, context, instr.assignObjPropBinding.contextIdx, instr.assignObjPropBinding.notifyIdx);

                bindValues.append(bind);
                bind->m_mePtr = &bindValues.values[bindValues.count - 1];
                bind->addToObject(target);
            }
            break;

        case QmlInstruction::StoreValueSource:
            {
                QObject *obj = stack.pop();
                QmlPropertyValueSource *vs = reinterpret_cast<QmlPropertyValueSource *>(reinterpret_cast<char *>(obj) + instr.assignValueSource.castValue);
                QObject *target = stack.at(stack.count() - 1 - instr.assignValueSource.owner);
                QmlMetaProperty prop;
                prop.restore(instr.assignValueSource.property, target, ctxt);
                obj->setParent(target);
                vs->setTarget(prop);
            }
            break;

        case QmlInstruction::StoreValueInterceptor:
            {
                QObject *obj = stack.pop();
                QmlPropertyValueInterceptor *vi = reinterpret_cast<QmlPropertyValueInterceptor *>(reinterpret_cast<char *>(obj) + instr.assignValueInterceptor.castValue);
                QObject *target = stack.at(stack.count() - 1 - instr.assignValueInterceptor.owner);
                QmlMetaProperty prop;
                prop.restore(instr.assignValueInterceptor.property, target, ctxt);
                obj->setParent(target);
                vi->setTarget(prop);
                QmlVMEMetaObject *mo = static_cast<QmlVMEMetaObject *>((QMetaObject*)target->metaObject());
                mo->registerInterceptor(prop.coreIndex(), vi);
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
                void *a[] = { &v, 0, &status, &flags };
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
                        void *a[] = { &ptr, 0, &status, &flags };
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
                // NOTE: This assumes a cast to QObject does not alter the 
                // object pointer
                void *a[1];
                a[0] = &obj;
                QMetaObject::metacall(target, QMetaObject::ReadProperty, 
                                      instr.fetch.property, a);

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

        case QmlInstruction::Defer:
            {
                if (instr.defer.deferCount) {
                    QObject *target = stack.top();
                    QmlDeclarativeData *data = 
                        QmlDeclarativeData::get(target, true);
                    comp->addref();
                    data->deferredComponent = comp;
                    data->deferredIdx = ii;
                    ii += instr.defer.deferCount;
                }
            }
            break;

        case QmlInstruction::PopFetchedObject:
            {
                stack.pop();
            }
            break;

        case QmlInstruction::FetchValueType:
            {
                QObject *target = stack.top();
                QmlValueType *valueHandler = 
                    ep->valueTypes[instr.fetchValue.type];
                valueHandler->read(target, instr.fetchValue.property);
                stack.push(valueHandler);
            }
            break;

        case QmlInstruction::PopValueType:
            {
                QmlValueType *valueHandler = 
                    static_cast<QmlValueType *>(stack.pop());
                QObject *target = stack.top();
                valueHandler->write(target, instr.fetchValue.property, QmlMetaProperty::BypassInterceptor);
            }
            break;

        default:
            qFatal("QmlCompiledData: Internal error - unknown instruction %d", instr.type);
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

    if (bindValues.count)
        ep->bindValues << bindValues;
    if (parserStatus.count)
        ep->parserStatus << parserStatus;

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
