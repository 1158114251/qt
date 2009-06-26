/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
****************************************************************************/

#ifndef QSCRIPTENGINE_P_H
#define QSCRIPTENGINE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "private/qobject_p.h"

#ifndef QT_NO_SCRIPT

#include <QtCore/qhash.h>

#include "RefPtr.h"
#include "Structure.h"
#include "JSGlobalObject.h"
#include "JSValue.h"

QT_BEGIN_NAMESPACE

namespace JSC
{
    class JSCell;
    class JSGlobalObject;
    class UString;
}

namespace QScript
{
    class QObjectPrototype;
    class QMetaObjectPrototype;
    class QVariantPrototype;
#ifndef QT_NO_QOBJECT
    class QObjectData;
#endif
}

class QString;
class QStringList;
class QScriptContext;
class QScriptValue;
class QScriptTypeInfo;
class QScriptEngineAgent;

class QScriptEngine;
class QScriptEnginePrivate
#ifndef QT_NO_QOBJECT
    : public QObjectPrivate
#endif
{
    Q_DECLARE_PUBLIC(QScriptEngine)
public:
    QScriptEnginePrivate();
    virtual ~QScriptEnginePrivate();

    static QScriptEnginePrivate *get(QScriptEngine*);
    static QScriptEngine *get(QScriptEnginePrivate*);

    static bool convert(const QScriptValue &value,
                        int type, void *ptr,
                        QScriptEnginePrivate *eng);
    QScriptValue create(int type, const void *ptr);

    QScriptValue scriptValueFromJSCValue(JSC::JSValue value);
    JSC::JSValue scriptValueToJSCValue(const QScriptValue &value);
    void releaseJSCValue(JSC::JSValue value);

    QScriptValue scriptValueFromVariant(const QVariant &value);
    QVariant scriptValueToVariant(const QScriptValue &value, int targetType);

    JSC::JSValue jscValueFromVariant(const QVariant &value);
    QVariant jscValueToVariant(JSC::JSValue value, int targetType);

    QScriptValue arrayFromStringList(const QStringList &lst);
    static QStringList stringListFromArray(const QScriptValue &arr);

    QScriptValue arrayFromVariantList(const QVariantList &lst);
    static QVariantList variantListFromArray(const QScriptValue &arr);

    QScriptValue objectFromVariantMap(const QVariantMap &vmap);
    static QVariantMap variantMapFromObject(const QScriptValue &obj);

#ifndef QT_NO_QOBJECT
    JSC::JSValue newQObject(QObject *object,
        QScriptEngine::ValueOwnership ownership = QScriptEngine::QtOwnership,
        const QScriptEngine:: QObjectWrapOptions &options = 0);
    JSC::JSValue newQMetaObject(const QMetaObject *metaObject,
                                JSC::JSValue ctor);
    
    static bool convertToNativeQObject(const QScriptValue &value,
                                       const QByteArray &targetType,
                                       void **result);

    QScript::QObjectData *qobjectData(QObject *object);
    void disposeQObject(QObject *object);
    void emitSignalHandlerException();

    bool scriptConnect(QObject *sender, const char *signal,
                       JSC::JSValue receiver, JSC::JSValue function);
    bool scriptDisconnect(QObject *sender, const char *signal,
                          JSC::JSValue receiver, JSC::JSValue function);

    bool scriptConnect(QObject *sender, int index,
                       JSC::JSValue receiver, JSC::JSValue function,
                       JSC::JSValue senderWrapper = 0);
    bool scriptDisconnect(QObject *sender, int index,
                          JSC::JSValue receiver, JSC::JSValue function);

    bool scriptConnect(JSC::JSValue signal, JSC::JSValue receiver,
                       JSC::JSValue function);
    bool scriptDisconnect(JSC::JSValue signal, JSC::JSValue receiver,
                          JSC::JSValue function);

    // private slots
    void _q_objectDestroyed(QObject *);
#endif

    JSC::JSGlobalObject *globalObject;
    QScriptContext *currentContext;

    QScript::QObjectPrototype *qobjectPrototype;
    WTF::RefPtr<JSC::Structure> qobjectWrapperObjectStructure;
    QScript::QMetaObjectPrototype *qmetaobjectPrototype;
    WTF::RefPtr<JSC::Structure> qmetaobjectWrapperObjectStructure;
    QScript::QVariantPrototype *variantPrototype;
    WTF::RefPtr<JSC::Structure> variantWrapperObjectStructure;

    QScriptEngineAgent *agent;
    QHash<JSC::JSCell*, QBasicAtomicInt> keepAliveValues;
    QHash<int, QScriptTypeInfo*> m_typeInfos;
    int processEventsInterval;

#ifndef QT_NO_QOBJECT
    QHash<QObject*, QScript::QObjectData*> m_qobjectData;
#endif

#ifdef QT_NO_QOBJECT
    QScriptEngine *q_ptr;
#endif
};

namespace QScript
{

class GlobalObject : public JSC::JSGlobalObject
{
public:
    GlobalObject(QScriptEnginePrivate*);
    ~GlobalObject();
    virtual JSC::UString className() const { return "global"; }
    virtual void mark();

public:
    QScriptEnginePrivate *engine;
};

}

QT_END_NAMESPACE

#endif // QT_NO_SCRIPT

#endif
