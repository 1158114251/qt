/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtScript module of the Qt Toolkit.
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
** contact the sales department at http://qt.nokia.com/contact.
** $QT_END_LICENSE$
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

#include <QtCore/qhash.h>
#include <QtCore/qset.h>

#include "RefPtr.h"
#include "Structure.h"
#include "JSGlobalObject.h"
#include "JSValue.h"

namespace JSC
{
    class ExecState;
    typedef ExecState CallFrame;
    class JSCell;
    class JSGlobalObject;
    class UString;
}


QT_BEGIN_NAMESPACE

class QString;
class QStringList;
class QScriptContext;
class QScriptValue;
class QScriptValuePrivate;
class QScriptTypeInfo;
class QScriptEngineAgent;
class QScriptEnginePrivate;
class QScriptSyntaxCheckResult;
class QScriptEngine;

namespace QScript
{
    class QObjectPrototype;
    class QMetaObjectPrototype;
    class QVariantPrototype;
#ifndef QT_NO_QOBJECT
    class QObjectData;
#endif
    class TimeoutCheckerProxy;

    //some conversion helper functions
    QScriptEnginePrivate *scriptEngineFromExec(const JSC::ExecState *exec);
    bool isFunction(JSC::JSValue value);
}

class QScriptEnginePrivate
#ifndef QT_NO_QOBJECT
    : public QObjectPrivate
#endif
{
    Q_DECLARE_PUBLIC(QScriptEngine)
public:
    QScriptEnginePrivate();
    virtual ~QScriptEnginePrivate();

    static QScriptEnginePrivate *get(QScriptEngine *q) { return q ? q->d_func() : 0; }
    static QScriptEngine *get(QScriptEnginePrivate *d) { return d ? d->q_func() : 0; }

    static bool convert(const QScriptValue &value,
                        int type, void *ptr,
                        QScriptEnginePrivate *eng);
    QScriptValue create(int type, const void *ptr);
    bool hasDemarshalFunction(int type) const;

    QScriptValue scriptValueFromJSCValue(JSC::JSValue value);
    JSC::JSValue scriptValueToJSCValue(const QScriptValue &value);

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

    JSC::JSValue defaultPrototype(int metaTypeId) const;
    void setDefaultPrototype(int metaTypeId, JSC::JSValue prototype);

    static QScriptContext *contextForFrame(JSC::ExecState *frame);
    static JSC::ExecState *frameForContext(QScriptContext *context);
    static const JSC::ExecState *frameForContext(const QScriptContext *context);

    JSC::JSGlobalObject *originalGlobalObject() const;
    JSC::JSObject *getOriginalGlobalObjectProxy();
    JSC::JSObject *customGlobalObject() const;
    JSC::JSObject *globalObject() const;
    void setGlobalObject(JSC::JSObject *object);
    JSC::ExecState *globalExec() const;
    JSC::JSValue toUsableValue(JSC::JSValue value);
    static JSC::JSValue thisForContext(JSC::ExecState *frame);

    JSC::CallFrame *pushContext(JSC::CallFrame *exec, JSC::JSValue thisObject, const JSC::ArgList& args,
                                JSC::JSObject *callee, bool calledAsConstructor = false);
    void popContext();

    void mark();
    bool isCollecting() const;
    void collectGarbage();

    //flags that we set on the return value register for native function. (ie when codeBlock is 0)
    enum ContextFlags {
        NativeContext = 1,
        CalledAsConstructorContext = 2,
        HasScopeContext = 4
    };
    static uint contextFlags(JSC::ExecState *);
    static void setContextFlags(JSC::ExecState *, uint);

    QScript::TimeoutCheckerProxy *timeoutChecker() const;

    void agentDeleted(QScriptEngineAgent *agent);

#ifndef QT_NO_QOBJECT
    JSC::JSValue newQObject(QObject *object,
        QScriptEngine::ValueOwnership ownership = QScriptEngine::QtOwnership,
        const QScriptEngine:: QObjectWrapOptions &options = 0);
    JSC::JSValue newQMetaObject(const QMetaObject *metaObject,
                                JSC::JSValue ctor);

    static QScriptSyntaxCheckResult checkSyntax(const QString &program);
    static bool canEvaluate(const QString &program);
    static bool convertToNativeQObject(const QScriptValue &value,
                                       const QByteArray &targetType,
                                       void **result);

    QScript::QObjectData *qobjectData(QObject *object);
    void disposeQObject(QObject *object);
    void emitSignalHandlerException();

    bool scriptConnect(QObject *sender, const char *signal,
                       JSC::JSValue receiver, JSC::JSValue function,
                       Qt::ConnectionType type);
    bool scriptDisconnect(QObject *sender, const char *signal,
                          JSC::JSValue receiver, JSC::JSValue function);

    bool scriptConnect(QObject *sender, int index,
                       JSC::JSValue receiver, JSC::JSValue function,
                       JSC::JSValue senderWrapper,
                       Qt::ConnectionType type);
    bool scriptDisconnect(QObject *sender, int index,
                          JSC::JSValue receiver, JSC::JSValue function);

    bool scriptConnect(JSC::JSValue signal, JSC::JSValue receiver,
                       JSC::JSValue function, Qt::ConnectionType type);
    bool scriptDisconnect(JSC::JSValue signal, JSC::JSValue receiver,
                          JSC::JSValue function);

    void registerScriptValue(QScriptValuePrivate *value);
    void unregisterScriptValue(QScriptValuePrivate *value);
    void detachAllRegisteredScriptValues();

    // private slots
    void _q_objectDestroyed(QObject *);
#endif

    JSC::JSGlobalData *globalData;
    JSC::JSObject *originalGlobalObjectProxy;
    JSC::ExecState *currentFrame;

    WTF::RefPtr<JSC::Structure> scriptObjectStructure;

    QScript::QObjectPrototype *qobjectPrototype;
    WTF::RefPtr<JSC::Structure> qobjectWrapperObjectStructure;

    QScript::QMetaObjectPrototype *qmetaobjectPrototype;
    WTF::RefPtr<JSC::Structure> qmetaobjectWrapperObjectStructure;

    QScript::QVariantPrototype *variantPrototype;
    WTF::RefPtr<JSC::Structure> variantWrapperObjectStructure;

    QList<QScriptEngineAgent*> ownedAgents;
    QScriptEngineAgent *activeAgent;
    int agentLineNumber;
    QScriptValuePrivate *registeredScriptValues;
    QHash<int, QScriptTypeInfo*> m_typeInfos;
    int processEventsInterval;
    QScriptValue abortResult;
    bool inEval;

    QSet<QString> importedExtensions;
    QSet<QString> extensionsBeingImported;

#ifndef QT_NO_QOBJECT
    QHash<QObject*, QScript::QObjectData*> m_qobjectData;
#endif

#ifdef QT_NO_QOBJECT
    QScriptEngine *q_ptr;
#endif
};

QT_END_NAMESPACE

#endif
