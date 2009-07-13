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

#ifndef QMLCONTEXT_P_H
#define QMLCONTEXT_P_H

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

#include <QtDeclarative/qmlcontext.h>
#include <private/qobject_p.h>
#include <private/qmldeclarativedata_p.h>
#include <QtCore/qhash.h>
#include <QtScript/qscriptvalue.h>
#include <QtCore/qset.h>

QT_BEGIN_NAMESPACE

class QmlContext;
class QmlExpression;
class QmlEngine;
class QmlExpression;
class QmlCompiledComponent;

class QmlContextPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QmlContext)
public:
    QmlContextPrivate();

    QmlContext *parent;
    QmlEngine *engine;
    bool isInternal;

    QHash<QString, int> propertyNames;
    QList<QVariant> propertyValues;
    int notifyIndex;

    QObjectList defaultObjects;
    int highPriorityCount;

    QScriptValueList scopeChain;

    QUrl url;
    QByteArray typeName; 
    int startLine;
    int endLine;

    void init();

    void dump();
    void dump(int depth);

    void destroyed(QObject *);

    enum Priority {
        HighPriority,
        NormalPriority
    };
    void addDefaultObject(QObject *, Priority);

    void invalidateEngines();
    QSet<QmlContext *> childContexts;
    QSet<QmlExpression *> childExpressions;

    QmlSimpleDeclarativeData contextData;
    QObjectList contextObjects;

    // Only used for debugging
    QList<QPointer<QObject> > instances;
};

QT_END_NAMESPACE

#endif // QMLCONTEXT_P_H
