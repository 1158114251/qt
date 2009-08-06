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

#include "qscriptglobalobject_p.h"

#ifndef QT_NO_SCRIPT

#include "../api/qscriptengine.h"
#include "../api/qscriptengine_p.h"

QT_BEGIN_NAMESPACE

namespace JSC
{

ASSERT_CLASS_FITS_IN_CELL(QScript::GlobalObject);
ASSERT_CLASS_FITS_IN_CELL(QScript::OriginalGlobalObjectProxy);

} // namespace JSC

namespace QScript
{

GlobalObject::GlobalObject()
    : JSC::JSGlobalObject(), customGlobalObject(0)
{
}

GlobalObject::~GlobalObject()
{
}

void GlobalObject::mark()
{
    Q_ASSERT(!marked());
    JSC::JSGlobalObject::mark();
    if (customGlobalObject && !customGlobalObject->marked())
        customGlobalObject->mark();
}

bool GlobalObject::getOwnPropertySlot(JSC::ExecState* exec,
                                      const JSC::Identifier& propertyName,
                                      JSC::PropertySlot& slot)
{
    QScriptEnginePrivate *engine = scriptEngineFromExec(exec);
    if (propertyName == exec->propertyNames().arguments && engine->currentFrame->argumentCount() > 0) {
        JSC::JSValue args = engine->scriptValueToJSCValue(engine->contextForFrame(engine->currentFrame)->argumentsObject());
        slot.setValue(args);
        return true;
    }
    if (customGlobalObject)
        return customGlobalObject->getOwnPropertySlot(exec, propertyName, slot);
    return JSC::JSGlobalObject::getOwnPropertySlot(exec, propertyName, slot);
}

void GlobalObject::put(JSC::ExecState* exec, const JSC::Identifier& propertyName,
                       JSC::JSValue value, JSC::PutPropertySlot& slot)
{
    if (customGlobalObject)
        customGlobalObject->put(exec, propertyName, value, slot);
    else
        JSC::JSGlobalObject::put(exec, propertyName, value, slot);
}

bool GlobalObject::deleteProperty(JSC::ExecState* exec,
                                  const JSC::Identifier& propertyName, bool checkDontDelete)
{
    if (customGlobalObject)
        return customGlobalObject->deleteProperty(exec, propertyName, checkDontDelete);
    return JSC::JSGlobalObject::deleteProperty(exec, propertyName, checkDontDelete);
}

bool GlobalObject::getPropertyAttributes(JSC::ExecState* exec, const JSC::Identifier& propertyName,
                                         unsigned& attributes) const
{
    if (customGlobalObject)
        return customGlobalObject->getPropertyAttributes(exec, propertyName, attributes);
    return JSC::JSGlobalObject::getPropertyAttributes(exec, propertyName, attributes);
}

void GlobalObject::getPropertyNames(JSC::ExecState* exec, JSC::PropertyNameArray& propertyNames, unsigned listedAttributes)
{
    if (customGlobalObject)
        customGlobalObject->getPropertyNames(exec, propertyNames, listedAttributes);
    else
        JSC::JSGlobalObject::getPropertyNames(exec, propertyNames, listedAttributes);
}

} // namespace QScript

QT_END_NAMESPACE

#endif // QT_NO_SCRIPT
