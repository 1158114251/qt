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

#include "qscriptcontext.h"

#include "qscriptcontext_p.h"
#include "qscriptcontextinfo.h"
#include "qscriptengine.h"
#include "qscriptengine_p.h"
#include "../bridge/qscriptactivationobject_p.h"

#include "Arguments.h"
#include "CodeBlock.h"
#include "Error.h"
#include "JSFunction.h"
#include "JSObject.h"
#include "JSGlobalObject.h"

#include <QtCore/qstringlist.h>

QT_BEGIN_NAMESPACE

/*!
  \since 4.3
  \class QScriptContext

  \brief The QScriptContext class represents a Qt Script function invocation.

  \ingroup script
  \mainclass

  A QScriptContext provides access to the `this' object and arguments
  passed to a script function. You typically want to access this
  information when you're writing a native (C++) function (see
  QScriptEngine::newFunction()) that will be called from script
  code. For example, when the script code

  \snippet doc/src/snippets/code/src_script_qscriptcontext.cpp 0

  is evaluated, a QScriptContext will be created, and the context will
  carry the arguments as QScriptValues; in this particular case, the
  arguments will be one QScriptValue containing the number 20.5, a second
  QScriptValue containing the string \c{"hello"}, and a third QScriptValue
  containing a Qt Script object.

  Use argumentCount() to get the number of arguments passed to the
  function, and argument() to get an argument at a certain index. The
  argumentsObject() function returns a Qt Script array object
  containing all the arguments; you can use the QScriptValueIterator
  to iterate over its elements, or pass the array on as arguments to
  another script function using QScriptValue::call().

  Use thisObject() to get the `this' object associated with the function call,
  and setThisObject() to set the `this' object. If you are implementing a
  native "instance method", you typically fetch the thisObject() and access
  one or more of its properties:

  \snippet doc/src/snippets/code/src_script_qscriptcontext.cpp 1

  Use isCalledAsConstructor() to determine if the function was called
  as a constructor (e.g. \c{"new foo()"} (as constructor) or just
  \c{"foo()"}).  When a function is called as a constructor, the
  thisObject() contains the newly constructed object that the function
  is expected to initialize.

  Use throwValue() or throwError() to throw an exception.

  Use callee() to obtain the QScriptValue that represents the function being
  called. This can for example be used to call the function recursively.

  Use parentContext() to get a pointer to the context that precedes
  this context in the activation stack. This is mostly useful for
  debugging purposes (e.g. when constructing some form of backtrace).

  The activationObject() function returns the object that is used to
  hold the local variables associated with this function call. You can
  replace the activation object by calling setActivationObject(). A
  typical usage of these functions is when you want script code to be
  evaluated in the context of the parent context, e.g. to implement an
  include() function:

  \snippet doc/src/snippets/code/src_script_qscriptcontext.cpp 2

  Use backtrace() to get a human-readable backtrace associated with
  this context. This can be useful for debugging purposes when
  implementing native functions. The toString() function provides a
  string representation of the context. (QScriptContextInfo provides
  more detailed debugging-related information about the
  QScriptContext.)

  Use engine() to obtain a pointer to the QScriptEngine that this context
  resides in.

  \sa QScriptContextInfo, QScriptEngine::newFunction(), QScriptable
*/

/*!
    \enum QScriptContext::ExecutionState

    This enum specifies the frameution state of the context.

    \value NormalState The context is in a normal state.

    \value ExceptionState The context is in an exceptional state.
*/

/*!
    \enum QScriptContext::Error

    This enum specifies types of error.

    \value ReferenceError A reference error.

    \value SyntaxError A syntax error.

    \value TypeError A type error.

    \value RangeError A range error.

    \value URIError A URI error.

    \value UnknownError An unknown error.
*/

/*!
  \internal
*/
QScriptContext::QScriptContext()
{
    //QScriptContext doesn't exist,  pointer to QScriptContext are just pointer to  JSC::CallFrame
    Q_ASSERT(false);
}

/*!
  Throws an exception with the given \a value.
  Returns the value thrown (the same as the argument).

  \sa throwError(), state()
*/
QScriptValue QScriptContext::throwValue(const QScriptValue &value)
{
    JSC::CallFrame *frame = reinterpret_cast<JSC::CallFrame *>(this);
    JSC::JSValue jscValue = QScript::scriptEngineFromExec(frame)->scriptValueToJSCValue(value);
    frame->setException(jscValue);
    return value;
}

/*!
  Throws an \a error with the given \a text.
  Returns the created error object.

  The \a text will be stored in the \c{message} property of the error
  object.

  The error object will be initialized to contain information about
  the location where the error occurred; specifically, it will have
  properties \c{lineNumber}, \c{fileName} and \c{stack}. These
  properties are described in \l {QtScript Extensions to ECMAScript}.

  \sa throwValue(), state()
*/
QScriptValue QScriptContext::throwError(Error error, const QString &text)
{
    JSC::CallFrame *frame = reinterpret_cast<JSC::CallFrame *>(this);
    JSC::ErrorType jscError = JSC::GeneralError;
    switch (error) {
    case UnknownError:
        break;
    case ReferenceError:
        jscError = JSC::ReferenceError;
        break;
    case SyntaxError:
        jscError = JSC::SyntaxError;
        break;
    case TypeError:
        jscError = JSC::TypeError;
        break;
    case RangeError:
        jscError = JSC::RangeError;
        break;
    case URIError:
        jscError = JSC::URIError;
        break;
    }
    JSC::JSObject *result = JSC::throwError(frame, jscError, QScript::qtStringToJSCUString(text));
    return QScript::scriptEngineFromExec(frame)->scriptValueFromJSCValue(result);
}

/*!
  \overload

  Throws an error with the given \a text.
  Returns the created error object.

  \sa throwValue(), state()
*/
QScriptValue QScriptContext::throwError(const QString &text)
{
    JSC::CallFrame *frame = reinterpret_cast<JSC::CallFrame *>(this);
    JSC::JSObject *result = JSC::throwError(frame, JSC::GeneralError, QScript::qtStringToJSCUString(text));
    return QScript::scriptEngineFromExec(frame)->scriptValueFromJSCValue(result);
}

/*!
  Destroys this QScriptContext.
*/
QScriptContext::~QScriptContext()
{
    //QScriptContext doesn't exist,  pointer to QScriptContext are just pointer to JSC::CallFrame
    Q_ASSERT(false);
}

/*!
  Returns the QScriptEngine that this QScriptContext belongs to.
*/
QScriptEngine *QScriptContext::engine() const
{
    const JSC::CallFrame *frame = reinterpret_cast<const JSC::CallFrame *>(this);
    return QScriptEnginePrivate::get(QScript::scriptEngineFromExec(frame));
}

/*!
  Returns the function argument at the given \a index.

  If \a index >= argumentCount(), a QScriptValue of
  the primitive type Undefined is returned.

  \sa argumentCount()
*/
QScriptValue QScriptContext::argument(int index) const
{
    JSC::CallFrame *frame = const_cast<JSC::CallFrame *>(reinterpret_cast<const JSC::CallFrame *>(this));
    if (index < 0)
        return QScriptValue();
    if (index >= argumentCount())
        return QScriptValue(QScriptValue::UndefinedValue);
    JSC::Register* thisRegister = frame->registers() - JSC::RegisterFile::CallFrameHeaderSize - frame->argumentCount();
    if (frame->codeBlock() == 0)
        ++index; // ### off-by-one issue with native functions
    return QScript::scriptEngineFromExec(frame)->scriptValueFromJSCValue(thisRegister[index].jsValue());
}

/*!
  Returns the callee. The callee is the function object that this
  QScriptContext represents an invocation of.
*/
QScriptValue QScriptContext::callee() const
{
    const JSC::CallFrame *frame = reinterpret_cast<const JSC::CallFrame *>(this);
    return QScript::scriptEngineFromExec(frame)->scriptValueFromJSCValue(frame->callee());
}

/*!
  Returns the arguments object of this QScriptContext.

  The arguments object has properties \c callee (equal to callee())
  and \c length (equal to argumentCount()), and properties \c 0, \c 1,
  ..., argumentCount() - 1 that provide access to the argument
  values. Initially, property \c P (0 <= \c P < argumentCount()) has
  the same value as argument(\c P). In the case when \c P is less
  than the number of formal parameters of the function, \c P shares
  its value with the corresponding property of the activation object
  (activationObject()). This means that changing this property changes
  the corresponding property of the activation object and vice versa.

  \sa argument(), activationObject()
*/
QScriptValue QScriptContext::argumentsObject() const
{
    JSC::CallFrame *frame = const_cast<JSC::CallFrame *>(reinterpret_cast<const JSC::CallFrame *>(this));
    if (frame == frame->lexicalGlobalObject()->globalExec()) {
        //global context doesn't have any argument, return an empty object
        return QScriptEnginePrivate::get(QScript::scriptEngineFromExec(frame))->newObject();
    }
    Q_ASSERT(frame->argumentCount() > 0); //we need at least 'this' otherwise we'll crash later
    if (!frame->optionalCalleeArguments()) {
        JSC::Arguments* arguments = new (&frame->globalData())JSC::Arguments(frame, JSC::Arguments::NoParameters);
        frame->setCalleeArguments(arguments);
        frame[JSC::RegisterFile::ArgumentsRegister] = arguments;
    }
    return QScript::scriptEngineFromExec(frame)->scriptValueFromJSCValue(frame->optionalCalleeArguments());
}

/*!
  Returns true if the function was called as a constructor
  (e.g. \c{"new foo()"}); otherwise returns false.

  When a function is called as constructor, the thisObject()
  contains the newly constructed object to be initialized.
*/
bool QScriptContext::isCalledAsConstructor() const
{
    JSC::CallFrame *frame = reinterpret_cast<JSC::CallFrame *>(const_cast<QScriptContext *>(this));

    //For native functions, look up for the QScriptActivationObject and its calledAsConstructor flag.
    JSC::ScopeChainNode *node = frame->scopeChain();
    JSC::ScopeChainIterator it(node);
    for (it = node->begin(); it != node->end(); ++it) {
        if ((*it)->isVariableObject()) {
            if ((*it)->inherits(&QScript::QScriptActivationObject::info)) {
                return static_cast<QScript::QScriptActivationObject *>(*it)->d_ptr()->calledAsConstructor;
            }
            //not a native function
            break;
        }
    }

    //Not a native function, try to look up in the bytecode if we where called from op_construct
    JSC::Instruction* returnPC = frame->returnPC();

    if (!returnPC)
        return false;

    JSC::CallFrame *callerFrame = reinterpret_cast<JSC::CallFrame *>(parentContext());
    if (!callerFrame)
        return false;

    if (returnPC[-JSC::op_construct_length].u.opcode == frame->interpreter()->getOpcode(JSC::op_construct)) {
        //We are maybe called from the op_construct opcode which has 6 opperands.
        //But we need to check we are not called from op_call with 4 opperands

        //we make sure that the returnPC[-1] (thisRegister) is smaller than the returnPC[-3] (registerOffset)
        //as if it was an op_call, the returnPC[-1] would be the registerOffset, bigger than returnPC[-3] (funcRegister)
        return returnPC[-1].u.operand < returnPC[-3].u.operand;
    }
    return false;
}

/*!
  Returns the parent context of this QScriptContext.
*/
QScriptContext *QScriptContext::parentContext() const
{
   const  JSC::CallFrame *frame = reinterpret_cast<const JSC::CallFrame *>(this);
    JSC::CallFrame *callerFrame = frame->callerFrame();
    if (callerFrame == (JSC::CallFrame*)(1)) // ### CallFrame::noCaller() is private
        return 0;
    return reinterpret_cast<QScriptContext *>(callerFrame);
}

/*!
  Returns the number of arguments passed to the function
  in this invocation.

  Note that the argument count can be different from the
  formal number of arguments (the \c{length} property of
  callee()).

  \sa argument()
*/
int QScriptContext::argumentCount() const
{
    const JSC::CallFrame *frame = reinterpret_cast<const JSC::CallFrame *>(this);
    int argc = frame->argumentCount();
    if (argc != 0)
        --argc; // -1 due to "this"
    return argc;
}

/*!
  \internal
*/
QScriptValue QScriptContext::returnValue() const
{
    qWarning("QScriptContext::returnValue() not implemented");
    return QScriptValue();
}

/*!
  \internal
*/
void QScriptContext::setReturnValue(const QScriptValue &result)
{
    JSC::CallFrame *frame = reinterpret_cast<JSC::CallFrame *>(this);
    JSC::CallFrame *callerFrame = frame->callerFrame();
    if (!callerFrame->codeBlock())
        return;
    Q_ASSERT_X(false, Q_FUNC_INFO, "check me");
    int dst = frame->registers()[JSC::RegisterFile::ReturnValueRegister].i(); // returnValueRegister() is private
    callerFrame[dst] = QScript::scriptEngineFromExec(frame)->scriptValueToJSCValue(result);
}

/*!
  Returns the activation object of this QScriptContext. The activation
  object provides access to the local variables associated with this
  context.

  \sa argument(), argumentsObject()
*/
QScriptValue QScriptContext::activationObject() const
{
    JSC::CallFrame *frame = const_cast<JSC::CallFrame *>(reinterpret_cast<const JSC::CallFrame *>(this));
    // ### this is still a bit shaky
    // if properties of the activation are accessed after this context is
    // popped, we CRASH.
    // Ideally we should be able to store the activation object in the callframe
    // and JSC would clean it up for us.
    JSC::JSObject *result = 0;
    // look in scope chain
    {
        JSC::ScopeChainNode *node = frame->scopeChain();
        JSC::ScopeChainIterator it(node);
        for (it = node->begin(); it != node->end(); ++it) {
            if ((*it)->isVariableObject()) {
                result = *it;
                break;
            }
        }
    }
    if (!result) {
        JSC::CodeBlock *codeBlock = frame->codeBlock();
        if (!codeBlock) {
            // native function
            result = new (frame)QScript::QScriptActivationObject(frame);
        } else {
            JSC::FunctionBodyNode *body = static_cast<JSC::FunctionBodyNode*>(codeBlock->ownerNode());
            result = new (frame)JSC::JSActivation(frame, body);
        }
    }
    return QScript::scriptEngineFromExec(frame)->scriptValueFromJSCValue(result);
}

/*!
  Sets the activation object of this QScriptContext to be the given \a
  activation.

  If \a activation is not an object, this function does nothing.
*/
void QScriptContext::setActivationObject(const QScriptValue &activation)
{
    if (!activation.isObject())
        return;
    JSC::CallFrame *frame = reinterpret_cast<JSC::CallFrame *>(this);
    QScriptEnginePrivate *engine = QScript::scriptEngineFromExec(frame);
    JSC::JSObject *object = JSC::asObject(engine->scriptValueToJSCValue(activation));
    if (!object->isVariableObject()) {
        qWarning("QScriptContext::setActivationObject(): not an activation object");
        return;
    }
// ### look for variableObject in d->frame->scopeChain, replace by object
    qWarning("QScriptContext::setActivationObject() not implemented");
}

/*!
  Returns the `this' object associated with this QScriptContext.
*/
QScriptValue QScriptContext::thisObject() const
{
    JSC::CallFrame *frame = const_cast<JSC::CallFrame *>(reinterpret_cast<const JSC::CallFrame *>(this));
    QScriptEnginePrivate *engine = QScript::scriptEngineFromExec(frame);
    JSC::JSValue result = engine->thisForContext(frame);
    if (!result || result.isNull())
        result = frame->globalThisValue();
    return engine->scriptValueFromJSCValue(result);
}

/*!
  Sets the `this' object associated with this QScriptContext to be
  \a thisObject.

  If \a thisObject is not an object, this function does nothing.
*/
void QScriptContext::setThisObject(const QScriptValue &thisObject)
{
    JSC::CallFrame *frame = reinterpret_cast<JSC::CallFrame *>(this);
    if (!thisObject.isObject())
        return;
    if (thisObject.engine() != engine()) {
        qWarning("QScriptContext::setThisObject() failed: "
                 "cannot set an object created in "
                 "a different engine");
        return;
    }
    if (frame == frame->lexicalGlobalObject()->globalExec()) {
        engine()->setGlobalObject(thisObject);
        return;
    }
    JSC::JSValue jscThisObject = QScript::scriptEngineFromExec(frame)->scriptValueToJSCValue(thisObject);
    JSC::CodeBlock *cb = frame->codeBlock();
    if (cb != 0) {
        frame[cb->thisRegister()] = jscThisObject;
    } else {
        JSC::Register* thisRegister = frame->registers() - JSC::RegisterFile::CallFrameHeaderSize - frame->argumentCount();
        thisRegister[0] = jscThisObject;
    }
}

/*!
  Returns the frameution state of this QScriptContext.
*/
QScriptContext::ExecutionState QScriptContext::state() const
{
    const JSC::CallFrame *frame = reinterpret_cast<const JSC::CallFrame *>(this);
    if (frame->hadException())
        return QScriptContext::ExceptionState;
    return QScriptContext::NormalState;
}

/*!
  Returns a human-readable backtrace of this QScriptContext.

  Each line is of the form \c{<function-name>(<arguments>)@<file-name>:<line-number>}.

  To access individual pieces of debugging-related information (for
  example, to construct your own backtrace representation), use
  QScriptContextInfo.

  \sa QScriptEngine::uncaughtExceptionBacktrace(), QScriptContextInfo, toString()
*/
QStringList QScriptContext::backtrace() const
{
    QStringList result;
    const QScriptContext *ctx = this;
    while (ctx) {
        result.append(ctx->toString());
        ctx = ctx->parentContext();
    }
    return result;
}

/*!
  \since 4.4

  Returns a string representation of this context.
  This is useful for debugging.

  \sa backtrace()
*/
QString QScriptContext::toString() const
{
    QScriptContextInfo info(this);
    QString result;

    QString functionName = info.functionName();
    if (functionName.isEmpty()) {
        if (parentContext()) {
            if (info.functionType() == QScriptContextInfo::ScriptFunction)
                result.append(QLatin1String("<anonymous>"));
            else
                result.append(QLatin1String("<native>"));
        } else {
            result.append(QLatin1String("<global>"));
        }
    } else {
        result.append(functionName);
    }

    QStringList parameterNames = info.functionParameterNames();
    result.append(QLatin1String(" ("));
    for (int i = 0; i < argumentCount(); ++i) {
        if (i > 0)
            result.append(QLatin1String(", "));
        if (i < parameterNames.count()) {
            result.append(parameterNames.at(i));
            result.append(QLatin1Char('='));
        }
        QScriptValue arg = argument(i);
//        result.append(safeValueToString(arg)); ###
        result.append(arg.toString());
    }
    result.append(QLatin1Char(')'));

    QString fileName = info.fileName();
    int lineNumber = info.lineNumber();
    result.append(QLatin1String(" at "));
    if (!fileName.isEmpty()) {
        result.append(fileName);
        result.append(QLatin1Char(':'));
    }
    result.append(QString::number(lineNumber));
    return result;
}

/*!
  \internal
  \since 4.5

  Returns the scope chain of this QScriptContext.
*/
QScriptValueList QScriptContext::scopeChain() const
{
    const JSC::CallFrame *frame = reinterpret_cast<const JSC::CallFrame *>(this);
    QScriptValueList result;
    JSC::ScopeChainNode *node = frame->scopeChain();
    JSC::ScopeChainIterator it(node);
    for (it = node->begin(); it != node->end(); ++it)
        result.append(QScript::scriptEngineFromExec(frame)->scriptValueFromJSCValue(*it));
    return result;
}

/*!
  \internal
  \since 4.5

  Adds the given \a object to the front of this context's scope chain.

  If \a object is not an object, this function does nothing.
*/
void QScriptContext::pushScope(const QScriptValue &object)
{
    if (!object.isObject())
        return;
    JSC::CallFrame *frame = reinterpret_cast<JSC::CallFrame *>(this);
    JSC::JSValue jscObject = QScript::scriptEngineFromExec(frame)->scriptValueToJSCValue(object);
    frame->setScopeChain(frame->scopeChain()->push(JSC::asObject(jscObject)));
}

/*!
  \internal
  \since 4.5

  Removes the front object from this context's scope chain, and
  returns the removed object.

  If the scope chain is already empty, this function returns an
  invalid QScriptValue.
*/
QScriptValue QScriptContext::popScope()
{
    JSC::CallFrame *frame = reinterpret_cast<JSC::CallFrame *>(this);
    QScriptValue result = QScript::scriptEngineFromExec(frame)->scriptValueFromJSCValue(frame->scopeChain()->object);
    frame->setScopeChain(frame->scopeChain()->pop());
    return result;
}

QT_END_NAMESPACE
