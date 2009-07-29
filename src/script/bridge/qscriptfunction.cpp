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
** contact the sales department at http://www.qtsoftware.com/contact.
** $QT_END_LICENSE$
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


class QScriptActivation : public JSC::JSVariableObject {
public:
    QScriptActivation(JSC::ExecState *callFrame)
        : JSC::JSVariableObject(callFrame->globalData().activationStructure, new QScriptActivationData(callFrame->registers()))
        {}
    virtual ~QScriptActivation() { delete d; }
    virtual bool isDynamicScope() const {return true; }
//            virtual bool isActivationObject() const { return true; }

    virtual void putWithAttributes(JSC::ExecState *exec, const JSC::Identifier &propertyName, JSC::JSValue value, unsigned attributes)
    {
        if (symbolTablePutWithAttributes(propertyName, value, attributes))
            return;

        JSC::PutPropertySlot slot;
        JSObject::putWithAttributes(exec, propertyName, value, attributes, true, slot); 
    }


private:
    struct QScriptActivationData : public JSVariableObjectData {
        QScriptActivationData(JSC::Register* registers)
            : JSVariableObjectData(&symbolTable, registers)
            { }
            JSC::SymbolTable symbolTable;
        };
};



FunctionWrapper::FunctionWrapper(QScriptEngine *engine, int length, const JSC::Identifier &name,
                                   QScriptEngine::FunctionSignature function)
    : JSC::PrototypeFunction(QScriptEnginePrivate::get(engine)->globalExec(),
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

    //We might have nested eval inside our function so we should create another scope
    JSC::JSObject* scope = new (exec) QScriptActivation(exec);
    exec->setScopeChain(exec->scopeChain()->copy()->push(scope));

    QScriptValue result = self->data->function(ctx, self->data->engine);
    if (!result.isValid())
        result = QScriptValue(QScriptValue::UndefinedValue);

    exec->setScopeChain(exec->scopeChain()->pop());
    exec->scopeChain()->deref();

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
    QScriptContextPrivate::get(ctx)->calledAsConstructor = true;
    eng_p->currentFrame = exec;

    //We might have nested eval inside our function so we should create another scope
    JSC::JSObject* scope = new (exec) QScriptActivation(exec);
    exec->setScopeChain(exec->scopeChain()->copy()->push(scope));

    QScriptValue defaultObject = ctx->thisObject();
    QScriptValue result = self->data->function(ctx, self->data->engine);
    if (!result.isObject())
        result = defaultObject;

    exec->setScopeChain(exec->scopeChain()->pop());
    exec->scopeChain()->deref();

    eng_p->currentFrame = previousFrame;
    eng_p->releaseContextForFrame(exec);
    return JSC::asObject(eng_p->scriptValueToJSCValue(result));
}

FunctionWithArgWrapper::FunctionWithArgWrapper(QScriptEngine *engine, int length, const JSC::Identifier &name,
                                                 QScriptEngine::FunctionWithArgSignature function, void *arg)
    : JSC::PrototypeFunction(QScriptEnginePrivate::get(engine)->globalExec(),
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

    //We might have nested eval inside our function so we should create another scope
    JSC::JSObject* scope = new (exec) QScriptActivation(exec);
    exec->setScopeChain(exec->scopeChain()->copy()->push(scope));

    QScriptValue result = self->data->function(ctx, self->data->engine, self->data->arg);

    exec->setScopeChain(exec->scopeChain()->pop());
    exec->scopeChain()->deref();

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
    QScriptContextPrivate::get(ctx)->calledAsConstructor = true;
    eng_p->currentFrame = exec;

    //We might have nested eval inside our function so we should create another scope
    JSC::JSObject* scope = new (exec) QScriptActivation(exec);
    exec->setScopeChain(exec->scopeChain()->copy()->push(scope));

    QScriptValue defaultObject = ctx->thisObject();
    QScriptValue result = self->data->function(ctx, self->data->engine, self->data->arg);
    if (!result.isObject())
        result = defaultObject;

    exec->setScopeChain(exec->scopeChain()->pop());
    exec->scopeChain()->deref();

    eng_p->currentFrame = previousFrame;
    eng_p->releaseContextForFrame(exec);
    return JSC::asObject(eng_p->scriptValueToJSCValue(result));
}

} // namespace QScript

QT_END_NAMESPACE

#endif // QT_NO_SCRIPT
