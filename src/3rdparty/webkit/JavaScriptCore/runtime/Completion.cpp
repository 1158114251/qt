/*
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2003, 2007 Apple Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include "config.h"
#include "Completion.h"

#include "CallFrame.h"
#include "JSGlobalObject.h"
#include "JSLock.h"
#include "Interpreter.h"
#include "Parser.h"
#include "Debugger.h"
#include <stdio.h>

#ifdef QT_BUILD_SCRIPT_LIB
#include "DebuggerCallFrame.h"
#include "SourcePoolQt.h"
#endif

#if !PLATFORM(WIN_OS)
#include <unistd.h>
#endif

namespace JSC {

Completion checkSyntax(ExecState* exec, const SourceCode& source)
{
    JSLock lock(exec);

    int errLine;
    UString errMsg;

    RefPtr<ProgramNode> progNode = exec->globalData().parser->parse<ProgramNode>(exec, exec->dynamicGlobalObject()->debugger(), source, &errLine, &errMsg);
    if (!progNode)
        return Completion(Throw, Error::create(exec, SyntaxError, errMsg, errLine, source.provider()->asID(), source.provider()->url()));
    return Completion(Normal);
}

Completion evaluate(ExecState* exec, ScopeChain& scopeChain, const SourceCode& source, JSValue thisValue)
{
    JSLock lock(exec);
    
    intptr_t sourceId = source.provider()->asID();
#ifdef QT_BUILD_SCRIPT_LIB
    Debugger* debugger = exec->lexicalGlobalObject()->debugger();
    exec->globalData().scriptpool->startEvaluating(source);
    if (debugger)
        debugger->evaluateStart(sourceId);
#endif
    int errLine;
    UString errMsg;
    RefPtr<ProgramNode> programNode = exec->globalData().parser->parse<ProgramNode>(exec, exec->dynamicGlobalObject()->debugger(), source, &errLine, &errMsg);

    if (!programNode) {
        JSValue error = Error::create(exec, SyntaxError, errMsg, errLine, sourceId, source.provider()->url());
#ifdef QT_BUILD_SCRIPT_LIB
        if (debugger) {
            debugger->exceptionThrow(DebuggerCallFrame(exec, error), sourceId, false);
            debugger->evaluateStop(error, sourceId);
        }
        exec->globalData().scriptpool->stopEvaluating(source);
#endif
        return Completion(Throw, error);
    }

    JSObject* thisObj = (!thisValue || thisValue.isUndefinedOrNull()) ? exec->dynamicGlobalObject() : thisValue.toObject(exec);

    JSValue exception;
    JSValue result = exec->interpreter()->execute(programNode.get(), exec, scopeChain.node(), thisObj, &exception);

    if (exception) {
#ifdef QT_BUILD_SCRIPT_LIB
        if (debugger)
            debugger->evaluateStop(exception, sourceId);
        exec->globalData().scriptpool->stopEvaluating(source);
#endif
        if (exception.isObject() && asObject(exception)->isWatchdogException())
            return Completion(Interrupted, exception);
        return Completion(Throw, exception);
    }

#ifdef QT_BUILD_SCRIPT_LIB
    if (debugger)
        debugger->evaluateStop(result, sourceId);
    exec->globalData().scriptpool->stopEvaluating(source);
#endif
    return Completion(Normal, result);
}

} // namespace JSC
