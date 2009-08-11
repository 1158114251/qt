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

#ifndef QMLEXPRESSION_P_H
#define QMLEXPRESSION_P_H

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

#include "qmlbasicscript_p.h"
#include "qmlexpression.h"
#include "qmlengine_p.h"
#include <private/qguard_p.h>
#include <QtScript/qscriptvalue.h>

QT_BEGIN_NAMESPACE

class QmlAbstractExpression
{
public:
    QmlAbstractExpression();
    virtual ~QmlAbstractExpression();

    bool isValid() const;

    QmlContext *context() const;
    void setContext(QmlContext *);

private:
    friend class QmlContext;
    QmlContext *m_context;
    QmlAbstractExpression **m_prevExpression;
    QmlAbstractExpression  *m_nextExpression;
};

class QmlExpression;
class QString;
class QmlExpressionPrivate : public QObjectPrivate, public QmlAbstractExpression
{
    Q_DECLARE_PUBLIC(QmlExpression)
public:
    QmlExpressionPrivate();
    ~QmlExpressionPrivate();

    enum CompiledDataType {
        BasicScriptEngineData = 1,
        PreTransformedQtScriptData = 2
    };

    void init(QmlContext *, const QString &, QObject *);
    void init(QmlContext *, void *, QmlRefCount *, QObject *);

    QString expression;
    bool expressionFunctionValid:1;
    bool expressionRewritten:1;
    QScriptValue expressionFunction;

    QmlBasicScript sse;
    QObject *me;
    bool trackChange;

    QString fileName;
    int line;

    QVariant evalSSE();
    QVariant evalQtScript();

    struct SignalGuard : public QGuard<QObject> {
        SignalGuard() : isDuplicate(false), notifyIndex(-1) {}

        SignalGuard &operator=(QObject *obj) {
            QGuard<QObject>::operator=(obj);
            return *this;
        }
        SignalGuard &operator=(const SignalGuard &o) {
            QGuard<QObject>::operator=(o);
            isDuplicate = o.isDuplicate;
            notifyIndex = o.notifyIndex;
            return *this;
        }

        bool isDuplicate:1;
        int notifyIndex:31;
    };
    SignalGuard *guardList;
    int guardListLength;
    void updateGuards(const QPODVector<QmlEnginePrivate::CapturedProperty> &properties);
    void clearGuards();
};

QT_END_NAMESPACE

#endif // QMLEXPRESSION_P_H
