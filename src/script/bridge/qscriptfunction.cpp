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

#include "qscriptfunction_p.h"

#ifndef QT_NO_SCRIPT

#include "private/qscriptengine_p.h"
#include "qscriptcontext.h"
#include "private/qscriptcontext_p.h"

#include "JSGlobalObject.h"

QT_BEGIN_NAMESPACE

namespace JSC
{
ASSERT_CLASS_FITS_IN_CELL(QScript::FunctionWrapper);
ASSERT_CLASS_FITS_IN_CELL(QScript::FunctionWithArgWrapper);
}

namespace QScript
{

FunctionWrapper::FunctionWrapper(QScriptEngine *engine, int length, const JSC::Identifier &name,
                                   QScriptEngine::FunctionSignature function)
    : JSC::PrototypeFunction(QScriptEnginePrivate::get(engine)->globalObject->globalExec(),
                             length, name, proxyCall), data(new Data())
{
    data->engine = engine;
    data->function = function;
}

FunctionWrapper::~FunctionWrapper()
{
    delete data;
}

JSC::ConstructType FunctionWrapper::getConstructData(JSC::ConstructData& consData)
{
    consData.native.function = proxyConstruct;
    return JSC::ConstructTypeHost;
}

JSC::JSValue FunctionWrapper::proxyCall(JSC::ExecState *exec, JSC::JSObject *callee,
                                        JSC::JSValue thisObject, const JSC::ArgList &args)
{
    FunctionWrapper *self = static_cast<FunctionWrapper*>(callee);
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(self->data->engine);
    JSC::ExecState *previousFrame = eng_p->currentFrame;
    eng_p->currentFrame = exec;
    QScriptContext *ctx = eng_p->contextForFrame(exec);
    QScriptValue result = self->data->function(ctx, self->data->engine);
    if (!result.isValid())
        result = QScriptValue(QScriptValue::UndefinedValue);
    eng_p->currentFrame = previousFrame;
    eng_p->releaseContextForFrame(exec);
    return eng_p->scriptValueToJSCValue(result);
}

JSC::JSObject* FunctionWrapper::proxyConstruct(JSC::ExecState *exec, JSC::JSObject *callee,
                                               const JSC::ArgList &args)
{
    FunctionWrapper *self = static_cast<FunctionWrapper*>(callee);
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(self->data->engine);
    JSC::ExecState *previousFrame = eng_p->currentFrame;
    QScriptContext *ctx = eng_p->contextForFrame(exec);
    eng_p->currentFrame = exec;
    QScriptValue defaultObject = ctx->thisObject();
    QScriptValue result = self->data->function(ctx, self->data->engine);
    if (!result.isObject())
        result = defaultObject;
    eng_p->currentFrame = previousFrame;
    eng_p->releaseContextForFrame(exec);
    return JSC::asObject(eng_p->scriptValueToJSCValue(result));
}

FunctionWithArgWrapper::FunctionWithArgWrapper(QScriptEngine *engine, int length, const JSC::Identifier &name,
                                                 QScriptEngine::FunctionWithArgSignature function, void *arg)
    : JSC::PrototypeFunction(QScriptEnginePrivate::get(engine)->globalObject->globalExec(),
                             length, name, proxyCall), data(new Data())
{
    data->engine = engine;
    data->function = function;
    data->arg = arg;
}

FunctionWithArgWrapper::~FunctionWithArgWrapper()
{
    delete data;
}

JSC::ConstructType FunctionWithArgWrapper::getConstructData(JSC::ConstructData& consData)
{
    consData.native.function = proxyConstruct;
    return JSC::ConstructTypeHost;
}

JSC::JSValue FunctionWithArgWrapper::proxyCall(JSC::ExecState *exec, JSC::JSObject *callee,
                                               JSC::JSValue thisObject, const JSC::ArgList &args)
{
    FunctionWithArgWrapper *self = static_cast<FunctionWithArgWrapper*>(callee);
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(self->data->engine);
    JSC::ExecState *previousFrame = eng_p->currentFrame;
    QScriptContext *ctx = eng_p->contextForFrame(exec);
    eng_p->currentFrame = exec;
    QScriptValue result = self->data->function(ctx, self->data->engine, self->data->arg);
    eng_p->currentFrame = previousFrame;
    eng_p->releaseContextForFrame(exec);
    return eng_p->scriptValueToJSCValue(result);
}

JSC::JSObject* FunctionWithArgWrapper::proxyConstruct(JSC::ExecState *exec, JSC::JSObject *callee,
                                                      const JSC::ArgList &args)
{
    FunctionWithArgWrapper *self = static_cast<FunctionWithArgWrapper*>(callee);
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(self->data->engine);
    JSC::ExecState *previousFrame = eng_p->currentFrame;
    QScriptContext *ctx = eng_p->contextForFrame(exec);
    eng_p->currentFrame = exec;
    QScriptValue defaultObject = ctx->thisObject();
    QScriptValue result = self->data->function(ctx, self->data->engine, self->data->arg);
    if (!result.isObject())
        result = defaultObject;
    eng_p->currentFrame = previousFrame;
    eng_p->releaseContextForFrame(exec);
    return JSC::asObject(eng_p->scriptValueToJSCValue(result));
}

} // namespace QScript

QT_END_NAMESPACE

#endif // QT_NO_SCRIPT
