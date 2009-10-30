/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qmlbasicscript_p.h"
#include <QColor>
#include <QDebug>
#include <private/qmlengine_p.h>
#include <private/qmlcontext_p.h>
#include <QStack>
#include <private/qfxperf_p.h>
#include <private/qmlrefcount_p.h>
#include <private/qmljsast_p.h>
#include <private/qmljsengine_p.h>

QT_BEGIN_NAMESPACE

DEFINE_BOOL_CONFIG_OPTION(qmlBasicScriptDump, QML_BASICSCRIPT_DUMP);

using namespace QmlJS;

struct ScriptInstruction {
    enum {
        LoadIdObject,    // fetch
        FetchConstant,   // constant
        FetchContextConstant, // constant
        FetchRootConstant, // constant

        Equals,      // NA

        Int,         // integer
        Bool,        // boolean
    } type;

    union {
        struct {
            int idx;
        } fetch;
        struct {
            int value;
        } integer;
        struct {
            bool value;
        } boolean;
        struct {
            short idx;
            short notify;
            int type;
        } constant;
    };
};

class QmlBasicScriptPrivate
{
public:
    enum Flags { OwnData = 0x00000001 };

    int size;
    int stateSize;
    int instructionCount;
    int exprLen;

    ScriptInstruction *instructions() const { return (ScriptInstruction *)((char *)this + sizeof(QmlBasicScriptPrivate)); }

    const char *expr() const
    {
        return (const char *)(instructions() + instructionCount);
    }

    const char *data() const
    {
        return (const char *)(instructions() + instructionCount) + exprLen + 1;
    }

    static unsigned int alignRound(int s)
    {
        if (s % 4)
            s += 4 - (s % 4);
        return s;
    }
};

static QVariant fetch_value(QObject *o, int idx, int type)
{
    if (!o)
        return QVariant();

    switch(type) {
        case QVariant::String:
            {
                QString val;
                void *args[] = { &val, 0 };
                QMetaObject::metacall(o, QMetaObject::ReadProperty, idx, args);
                return QVariant(val);
            }
            break;
        case QVariant::UInt:
            {
                uint val;
                void *args[] = { &val, 0 };
                QMetaObject::metacall(o, QMetaObject::ReadProperty, idx, args);
                return QVariant(val);
            }
            break;
        case QVariant::Int:
            {
                int val;
                void *args[] = { &val, 0 };
                QMetaObject::metacall(o, QMetaObject::ReadProperty, idx, args);
                return QVariant(val);
            }
            break;
        case QMetaType::Float:
            {
                float val;
                void *args[] = { &val, 0 };
                QMetaObject::metacall(o, QMetaObject::ReadProperty, idx, args);
                return QVariant(val);
            }
            break;
        case QVariant::Double:
            {
                double val;
                void *args[] = { &val, 0 };
                QMetaObject::metacall(o, QMetaObject::ReadProperty, idx, args);
                return QVariant(val);
            }
            break;
        case QVariant::Color:
            {
                QColor val;
                void *args[] = { &val, 0 };
                QMetaObject::metacall(o, QMetaObject::ReadProperty, idx, args);
                return QVariant(val);
            }
            break;
        case QVariant::Bool:
            {
                bool val;
                void *args[] = { &val, 0 };
                QMetaObject::metacall(o, QMetaObject::ReadProperty, idx, args);
                return QVariant(val);
            }
            break;
        default:
            {
                // If the object is null, we extract the predicted type.  While this isn't
                // 100% reliable, in many cases it gives us better error messages if we
                // assign this null-object to an incompatible property
                if (QmlMetaType::isObject(type)) {
                    // NOTE: This assumes a cast to QObject does not alter the
                    // object pointer
                    QObject *val = 0;
                    void *args[] = { &val, 0 };
                    QMetaObject::metacall(o, QMetaObject::ReadProperty, idx, args);
                    if (!val) return QVariant(type, &val);
                    else return QVariant::fromValue(val);
                } else {
                    QVariant var = o->metaObject()->property(idx).read(o);
                    if (QmlMetaType::isObject(var.userType())) {
                        QObject *obj = 0;
                        obj = *(QObject **)var.data();
                        if (!obj) var = QVariant(var.userType(), &obj);
                        else var = QVariant::fromValue(obj);
                    }
                    return var;
                }
            }
            break;
    };
}

struct QmlBasicScriptCompiler
{
    QmlBasicScriptCompiler()
    : script(0), stateSize(0) {}

    QmlBasicScript *script;
    int stateSize;

    QmlParser::Object *context;
    QmlParser::Object *component;
    QHash<QString, QmlParser::Object *> ids;

    bool compile(QmlJS::AST::Node *);

    bool compileExpression(QmlJS::AST::Node *);

    bool tryConstant(QmlJS::AST::Node *);
    bool parseConstant(QmlJS::AST::Node *);
    bool tryName(QmlJS::AST::Node *);
    bool parseName(QmlJS::AST::Node *);

    bool buildName(QStringList &, QmlJS::AST::Node *);
    const QMetaObject *fetch(int type, const QMetaObject *, int idx);

    bool tryBinaryExpression(QmlJS::AST::Node *);
    bool compileBinaryExpression(QmlJS::AST::Node *);

    QByteArray data;
    QList<ScriptInstruction> bytecode;
};

/*!
    \internal
    \class QmlBasicScript
    \brief The QmlBasicScript class provides a fast implementation of a limited subset of QmlJS bindings.

    QmlBasicScript instances are used to accelerate binding.  Instead of using
    the slower, fully fledged QmlJS engine, many simple bindings can be
    evaluated using the QmlBasicScript engine.

    To see if the QmlBasicScript engine can handle a binding, call compile()
    and check the return value, or isValid() afterwards.

    To accelerate binding, QmlBasicScript can return a precompiled
    version of itself that can be saved for future use.  Call compileData() to
    get an opaque pointer to the compiled state, and compileDataSize() for the
    size of this data in bytes.  This data can be saved and passed to future
    instances of the QmlBasicScript constructor.  The initial copy of compile
    data is owned by the QmlBindScript instance on which compile() was called.
*/

/*!
    Create a new QmlBasicScript instance.
*/
QmlBasicScript::QmlBasicScript()
: flags(0), d(0), rc(0)
{
}

/*!
    Load the QmlBasicScript instance with saved \a data.

    \a data \b must be data previously acquired from calling compileData() on a
    previously created QmlBasicScript instance.  Any other data will almost
    certainly cause the QmlBasicScript engine to crash.

    \a data must continue to be valid throughout the QmlBasicScript instance
    life.  It does not assume ownership of the memory.

    If \a owner is set, it is referenced on creation and dereferenced on
    destruction of this instance.
*/

void QmlBasicScript::load(const char *data, QmlRefCount *owner)
{
    clear();
    d = (QmlBasicScriptPrivate *)data;
    rc = owner;
    if (rc) rc->addref();
}

/*!
    Return the text of the script expression.
 */
QByteArray QmlBasicScript::expression() const
{
    if (!d)
        return QByteArray();
    else
        return QByteArray(d->expr());
}

/*!
    Destroy the script instance.
*/
QmlBasicScript::~QmlBasicScript()
{
    clear();
}

/*!
    Clear this script.  The object will then be in its initial state, as though
    it were freshly constructed with default constructor.
*/
void QmlBasicScript::clear()
{
    if (flags & QmlBasicScriptPrivate::OwnData)
        free(d);
    if (rc) rc->release();
    d = 0;
    rc = 0;
    flags = 0;
}

/*!
    Dump the script instructions to stderr for debugging.
 */
void QmlBasicScript::dump()
{
    if (!d)
        return;

    qWarning() << d->instructionCount << "instructions:";
    for (int ii = 0; ii < d->instructionCount; ++ii) {
        const ScriptInstruction &instr = d->instructions()[ii];

        switch(instr.type) {
        case ScriptInstruction::LoadIdObject:
            qWarning().nospace() << "LOAD_ID_OBJECT";
            break;
        case ScriptInstruction::FetchConstant:
            qWarning().nospace() << "FETCH_CONSTANT";
            break;
        case ScriptInstruction::FetchContextConstant:
            qWarning().nospace() << "FETCH_CONTEXT_CONSTANT";
            break;
        case ScriptInstruction::FetchRootConstant:
            qWarning().nospace() << "FETCH_ROOT_CONSTANT";
            break;
        case ScriptInstruction::Equals:
            qWarning().nospace() << "EQUALS";
            break;
        case ScriptInstruction::Int:
            qWarning().nospace() << "INT\t\t" << instr.integer.value;
            break;
        case ScriptInstruction::Bool:
            qWarning().nospace() << "BOOL\t\t" << instr.boolean.value;
            break;
        default:
            qWarning().nospace() << "UNKNOWN";
            break;
        }
    }
}

/*!
    Return true if this is a valid script binding, otherwise returns false.
 */
bool QmlBasicScript::isValid() const
{
    return d != 0;
}

bool QmlBasicScript::compile(const Expression &expression)
{
    if (!expression.expression.asAST()) return false;

    QByteArray expr = expression.expression.asScript().toUtf8();
    const char *src = expr.constData();

    QmlBasicScriptCompiler bsc;
    bsc.script = this;
    bsc.context = expression.context;
    bsc.component = expression.component;
    bsc.ids = expression.ids;

    if (d) {
        if (flags & QmlBasicScriptPrivate::OwnData)
            free(d);
        d = 0;
        flags = 0;
    }

    if (bsc.compile(expression.expression.asAST())) {
        int len = ::strlen(src);
        flags = QmlBasicScriptPrivate::OwnData;
        int size = sizeof(QmlBasicScriptPrivate) +
                   bsc.bytecode.count() * sizeof(ScriptInstruction) +
                   QmlBasicScriptPrivate::alignRound(bsc.data.count() + len + 1);
        d = (QmlBasicScriptPrivate *) malloc(size);
        d->size = size;
        d->stateSize = bsc.stateSize;
        d->instructionCount = bsc.bytecode.count();
        d->exprLen = len;
        ::memcpy((char *)d->expr(), src, len + 1);
        for (int ii = 0; ii < d->instructionCount; ++ii)
            d->instructions()[ii] = bsc.bytecode.at(ii);
        ::memcpy((char *)d->data(), bsc.data.constData(), bsc.data.count());
    }

    if (d && qmlBasicScriptDump())
        dump();
    return d != 0;
}

bool QmlBasicScriptCompiler::compile(QmlJS::AST::Node *node)
{
    return compileExpression(node);
}

bool QmlBasicScriptCompiler::tryConstant(QmlJS::AST::Node *node)
{
    if (node->kind == AST::Node::Kind_TrueLiteral ||
        node->kind == AST::Node::Kind_FalseLiteral)
        return true;

    if (node->kind == AST::Node::Kind_NumericLiteral) {
        AST::NumericLiteral *lit = static_cast<AST::NumericLiteral *>(node);

        return double(int(lit->value)) == lit->value;
    }

    return false;
}

bool QmlBasicScriptCompiler::parseConstant(QmlJS::AST::Node *node)
{
    ScriptInstruction instr;

    if (node->kind == AST::Node::Kind_NumericLiteral) {
        AST::NumericLiteral *lit = static_cast<AST::NumericLiteral *>(node);
        instr.type = ScriptInstruction::Int;
        instr.integer.value = int(lit->value);
    } else {
        instr.type = ScriptInstruction::Bool;
        instr.boolean.value = node->kind == AST::Node::Kind_TrueLiteral;
    }

    bytecode.append(instr);

    return true;
}

bool QmlBasicScriptCompiler::tryName(QmlJS::AST::Node *node)
{
    return node->kind == AST::Node::Kind_IdentifierExpression ||
           node->kind == AST::Node::Kind_FieldMemberExpression;
}

bool QmlBasicScriptCompiler::buildName(QStringList &name,
                                       QmlJS::AST::Node *node)
{
    if (node->kind == AST::Node::Kind_IdentifierExpression) {
        name << static_cast<AST::IdentifierExpression*>(node)->name->asString();
    } else if (node->kind == AST::Node::Kind_FieldMemberExpression) {
        AST::FieldMemberExpression *expr =
            static_cast<AST::FieldMemberExpression *>(node);

        if (!buildName(name, expr->base))
            return false;

        name << expr->name->asString();
    } else {
        return false;
    }

    return true;
}

const QMetaObject *
QmlBasicScriptCompiler::fetch(int type, const QMetaObject *mo, int idx)
{
    ScriptInstruction instr;
    (int &)instr.type = type;
    instr.constant.idx = idx;
    QMetaProperty prop = mo->property(idx);
    if (prop.isConstant())
        instr.constant.notify = 0;
    else
        instr.constant.notify = prop.notifySignalIndex();
    instr.constant.type = prop.userType();
    bytecode << instr;
    return QmlMetaType::metaObjectForType(prop.userType());
}

bool QmlBasicScriptCompiler::parseName(AST::Node *node)
{
    QStringList nameParts;
    if (!buildName(nameParts, node))
        return false;

    QmlParser::Object *absType = 0;
    const QMetaObject *metaType = 0;

    for (int ii = 0; ii < nameParts.count(); ++ii) {
        const QString &name = nameParts.at(ii);

        // We don't handle signal properties
        if (name.length() > 2 && name.startsWith(QLatin1String("on")) &&
            name.at(2).isUpper())
            return false;

        if (ii == 0) {

            if (0) {
                // ### - Must test for an attached type name
            } else if (ids.contains(name)) {
                ScriptInstruction instr;
                instr.type = ScriptInstruction::LoadIdObject;
                instr.fetch.idx = ids.value(name)->idIndex;
                bytecode << instr;
                absType = ids.value(name);
            } else if(name.at(0).isLower()) {

                QByteArray utf8Name = name.toUtf8();
                const char *cname = utf8Name.constData();

                int d0Idx = context->metaObject()->indexOfProperty(cname);
                int d1Idx = -1;
                if (d0Idx == -1)
                    d1Idx = component->metaObject()->indexOfProperty(cname);

                if (d0Idx != -1) {
                    metaType = fetch(ScriptInstruction::FetchContextConstant,
                                     context->metaObject(), d0Idx);
                } else if(d1Idx != -1) {
                    metaType = fetch(ScriptInstruction::FetchRootConstant,
                                     component->metaObject(), d1Idx);
                } else {
                    return false;
                }

            } else {
                return false;
            }
        } else {

            if (!name.at(0).isLower())
                return false;

            const QMetaObject *mo = 0;
            if (absType)
                mo = absType->metaObject();
            else if(metaType)
                mo = metaType;
            else
                return false;

            QByteArray utf8Name = name.toUtf8();
            const char *cname = utf8Name.constData();
            int idx = mo->indexOfProperty(cname);
            if (idx == -1)
                return false;

            if (absType || mo->property(idx).isFinal()) {
                absType = 0; metaType = 0;
                metaType = fetch(ScriptInstruction::FetchConstant, mo, idx);
            } else {
                return false;
            }

        }
    }

    return true;
}

bool QmlBasicScriptCompiler::compileExpression(QmlJS::AST::Node *node)
{
    if (tryBinaryExpression(node))
        return compileBinaryExpression(node);
    else if (tryConstant(node))
        return parseConstant(node);
    else if (tryName(node))
        return parseName(node);
    else
        return false;
}

bool QmlBasicScriptCompiler::tryBinaryExpression(AST::Node *node)
{
    if (node->kind == AST::Node::Kind_BinaryExpression) {
        AST::BinaryExpression *expr =
            static_cast<AST::BinaryExpression *>(node);

        if (expr->op == QSOperator::Equal)
            return true;
    }
    return false;
}

bool QmlBasicScriptCompiler::compileBinaryExpression(AST::Node *node)
{
    if (node->kind == AST::Node::Kind_BinaryExpression) {
        AST::BinaryExpression *expr =
            static_cast<AST::BinaryExpression *>(node);

        if (!compileExpression(expr->left)) return false;
        if (!compileExpression(expr->right)) return false;

        ScriptInstruction instr;
        switch (expr->op) {
        case QSOperator::Equal:
            instr.type = ScriptInstruction::Equals;
            break;
        default:
            return false;
        }

        bytecode.append(instr);
        return true;
    }
    return false;
}

/*!
    Run the script in \a context and return the result.  
 */
QVariant QmlBasicScript::run(QmlContext *context, QObject *me)
{
    if (!isValid())
        return QVariant();

    QmlContextPrivate *contextPrivate = context->d_func();
    QmlEnginePrivate *enginePrivate = QmlEnginePrivate::get(context->engine());

    QStack<QVariant> stack;

    for (int idx = 0; idx < d->instructionCount; ++idx) {
        const ScriptInstruction &instr = d->instructions()[idx];

        switch(instr.type) {
            case ScriptInstruction::LoadIdObject:
            {
                stack.push(QVariant::fromValue(contextPrivate->idValues[instr.fetch.idx].data()));
                enginePrivate->capturedProperties <<
                    QmlEnginePrivate::CapturedProperty(context, -1, contextPrivate->notifyIndex + instr.fetch.idx);
            }
                break;

            case ScriptInstruction::FetchContextConstant:
            {
                stack.push(fetch_value(me, instr.constant.idx, instr.constant.type));
                if (me && instr.constant.notify != 0)
                    enginePrivate->capturedProperties <<
                        QmlEnginePrivate::CapturedProperty(me, instr.constant.idx, instr.constant.notify);
            }
                break;

            case ScriptInstruction::FetchRootConstant:
            {
                QObject *obj = contextPrivate->defaultObjects.at(0);

                stack.push(fetch_value(obj, instr.constant.idx, instr.constant.type));
                if (obj && instr.constant.notify != 0)
                    enginePrivate->capturedProperties <<
                        QmlEnginePrivate::CapturedProperty(obj, instr.constant.idx, instr.constant.notify);
            }
                break;

            case ScriptInstruction::FetchConstant:
            {
                QVariant o = stack.pop();
                QObject *obj = *(QObject **)o.constData();

                stack.push(fetch_value(obj, instr.constant.idx, instr.constant.type));
                if (obj && instr.constant.notify != 0)
                    enginePrivate->capturedProperties <<
                        QmlEnginePrivate::CapturedProperty(obj, instr.constant.idx, instr.constant.notify);
            }
                break;

            case ScriptInstruction::Int:
                stack.push(QVariant(instr.integer.value));
                break;

            case ScriptInstruction::Bool:
                stack.push(QVariant(instr.boolean.value));
                break;

            case ScriptInstruction::Equals:
                {
                    QVariant rhs = stack.pop();
                    QVariant lhs = stack.pop();

                    stack.push(rhs == lhs);
                }
                break;
            default:
                break;
        }
    }

    if (stack.isEmpty())
        return QVariant();
    else
        return stack.top();
}

bool QmlBasicScript::isSingleIdFetch() const
{
    if (!isValid())
        return false;

    return d->instructionCount == 1 && 
           d->instructions()[0].type == ScriptInstruction::LoadIdObject;
}

int QmlBasicScript::singleIdFetchIndex() const
{
    if (!isSingleIdFetch())
        return -1;

    return d->instructions()[0].fetch.idx;
}

bool QmlBasicScript::isSingleContextProperty() const
{
    if (!isValid())
        return false;

    return d->instructionCount == 1 && 
           d->instructions()[0].type == ScriptInstruction::FetchContextConstant;
}

int QmlBasicScript::singleContextPropertyIndex() const
{
    if (!isSingleContextProperty())
        return -1;

    return d->instructions()[0].constant.idx;
}

/*!
    Return a pointer to the script's compile data, or null if there is no data.
 */
const char *QmlBasicScript::compileData() const
{
    return (const char *)d;
}

/*!
    Return the size of the script's compile data, or zero if there is no data.
    The size will always be a multiple of 4.
 */
unsigned int QmlBasicScript::compileDataSize() const
{
    if (d)
        return d->size;
    else
        return 0;
}

QT_END_NAMESPACE
