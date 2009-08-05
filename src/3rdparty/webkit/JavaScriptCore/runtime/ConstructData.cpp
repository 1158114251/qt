/*
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "ConstructData.h"
#ifdef QT_BUILD_SCRIPT_LIB
#include "ExceptionHelpers.h"
#include "Interpreter.h"
#include "JSGlobalObject.h"
#endif

#include "JSFunction.h"

namespace JSC {

JSObject* construct(ExecState* exec, JSValue callee, ConstructType constructType, const ConstructData& constructData, const ArgList& args)
{
    if (constructType == ConstructTypeHost) {
#ifdef QT_BUILD_SCRIPT_LIB
        Structure* structure;
        JSValue prototype = callee.get(exec, exec->propertyNames().prototype);
        if (prototype.isObject())
            structure = asObject(prototype)->inheritorID();
        else
            structure = exec->lexicalGlobalObject()->emptyObjectStructure();
        JSObject* thisObj = new (exec) JSObject(structure);

        ScopeChainNode* scopeChain = exec->scopeChain();
        Interpreter *interp = exec->interpreter();
        Register *oldEnd = interp->registerFile().end();
        int argc = 1 + args.size(); // implicit "this" parameter
        if (!interp->registerFile().grow(oldEnd + argc + RegisterFile::CallFrameHeaderSize))
            return asObject(createStackOverflowError(exec));
        CallFrame* newCallFrame = CallFrame::create(oldEnd);
        size_t dst = 0;
        newCallFrame[0] = JSValue(thisObj);
        ArgList::const_iterator it;
        for (it = args.begin(); it != args.end(); ++it)
            newCallFrame[++dst] = *it;
        newCallFrame += argc + RegisterFile::CallFrameHeaderSize;
        newCallFrame->init(0, /*vPC=*/0, scopeChain, exec, 0, argc, asObject(callee));
        JSObject *result = constructData.native.function(newCallFrame, asObject(callee), args);
        interp->registerFile().shrink(oldEnd);
        return result;
#else
        return constructData.native.function(exec, asObject(object), args);
#endif
    }
    ASSERT(constructType == ConstructTypeJS);
    // FIXME: Can this be done more efficiently using the constructData?
    return asFunction(callee)->construct(exec, args);
}

} // namespace JSC
