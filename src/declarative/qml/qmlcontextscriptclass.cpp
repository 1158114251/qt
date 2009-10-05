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

#include "qmlcontextscriptclass_p.h"
#include <private/qmlengine_p.h>
#include <private/qmlcontext_p.h>

QT_BEGIN_NAMESPACE

struct ContextData {
    ContextData(QmlContext *c) : context(c) {}
    QGuard<QmlContext> context;
};

/*
    The QmlContextScriptClass handles property access for a QmlContext
    via QtScript.
 */
QmlContextScriptClass::QmlContextScriptClass(QmlEngine *bindEngine)
: QScriptDeclarativeClass(QmlEnginePrivate::getScriptEngine(bindEngine)), engine(bindEngine),
  lastPropertyIndex(-1), lastDefaultObject(-1)
{
}

QmlContextScriptClass::~QmlContextScriptClass()
{
}

QScriptValue QmlContextScriptClass::newContext(QmlContext *context)
{
    QScriptEngine *scriptEngine = QmlEnginePrivate::getScriptEngine(engine);

    return newObject(scriptEngine, this, (Object)new ContextData(context));
}

QScriptClass::QueryFlags 
QmlContextScriptClass::queryProperty(const Object &object, const Identifier &name, 
                                     QScriptClass::QueryFlags flags)
{
    Q_UNUSED(flags);
    QmlContext *bindContext = ((ContextData *)object)->context.data();
    if (!bindContext)
        return 0;

    QmlEnginePrivate *ep = QmlEnginePrivate::get(engine);
    QmlContextPrivate *cp = QmlContextPrivate::get(bindContext);
    
    lastPropertyIndex = -1;
    lastDefaultObject = -1;

    lastPropertyIndex = cp->propertyNames?cp->propertyNames->value(name):-1;
    if (lastPropertyIndex != -1)
        return QScriptClass::HandlesReadAccess;

    // ### Check for attached properties
#if 0
    QmlType *type = 0; ImportedNamespace *ns = 0;
    if (currentExpression && bindContext == currentExpression->context() && 
        propName.at(0).isUpper() && resolveType(bindContext->d_func()->imports, propName.toUtf8(), &type, 0, 0, 0, &ns)) {
        
        if (type || ns) {
            // Must be either an attached property, or an enum
            resolveData.object = bindContext->d_func()->defaultObjects.first();
            resolveData.type = type;
            resolveData.ns = ns;
            return QScriptClass::HandlesReadAccess;
        }

    } 
#endif

    for (int ii = 0; ii < cp->defaultObjects.count(); ++ii) {
        QScriptClass::QueryFlags rv = 
            ep->objectClass->queryProperty(cp->defaultObjects.at(ii), name, flags);

        if (rv) {
            lastDefaultObject = ii;
            return rv;
        }
    }

    return 0;
}

QScriptValue QmlContextScriptClass::property(const Object &object, const Identifier &name)
{
    Q_UNUSED(object);

    Q_ASSERT(lastPropertyIndex != -1 || lastDefaultObject != -1);

    QmlContext *bindContext = ((ContextData *)object)->context.data();
    Q_ASSERT(bindContext);

    QmlEnginePrivate *ep = QmlEnginePrivate::get(engine);
    QmlContextPrivate *cp = QmlContextPrivate::get(bindContext);


    // ### Check for attached properties
#if 0
    if (resolveData.type || resolveData.ns) {
        QmlTypeNameBridge tnb = { 
            resolveData.object, 
            resolveData.type, 
            resolveData.ns 
        };
        return scriptEngine.newObject(typeNameClass, scriptEngine.newVariant(qVariantFromValue(tnb)));
    } 
#endif

    if (lastPropertyIndex != -1) {

        QScriptValue rv;
        if (lastPropertyIndex < cp->idValueCount) {
            rv =  ep->objectClass->newQObject(cp->idValues[lastPropertyIndex].data());
        } else {
            QVariant value = cp->propertyValues.at(lastPropertyIndex);
            if (QmlMetaType::isObject(value.userType())) {
                rv = ep->objectClass->newQObject(QmlMetaType::toQObject(value));
            } else {
                // ### Shouldn't this be qScriptValueFromValue()
                rv = ep->scriptEngine.newVariant(value);
            }
        }

        ep->capturedProperties << 
            QmlEnginePrivate::CapturedProperty(bindContext, -1, 
                                               lastPropertyIndex + cp->notifyIndex);

        return rv;
    } else {

        // Default object property
        return ep->objectClass->property(cp->defaultObjects.at(lastDefaultObject), name);

    }
}

void QmlContextScriptClass::setProperty(const Object &object, const Identifier &name, 
                                        const QScriptValue &value)
{
    Q_ASSERT(lastDefaultObject != -1);

    QmlContext *bindContext = ((ContextData *)object)->context.data();
    Q_ASSERT(bindContext);

    QmlEnginePrivate *ep = QmlEnginePrivate::get(engine);
    QmlContextPrivate *cp = QmlContextPrivate::get(bindContext);

    ep->objectClass->setProperty(cp->defaultObjects.at(lastDefaultObject), name, value);
}

QT_END_NAMESPACE
