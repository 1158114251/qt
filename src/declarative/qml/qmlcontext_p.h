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
#include <private/qguard_p.h>
#include <private/qmlengine_p.h>
#include <private/qmlintegercache_p.h>
#include <private/qmltypenamecache_p.h>

QT_BEGIN_NAMESPACE

class QmlContext;
class QmlExpression;
class QmlEngine;
class QmlExpression;
class QmlExpressionPrivate;
class QmlAbstractExpression;
class QmlBinding_Id;

class QmlContextPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QmlContext)
public:
    QmlContextPrivate();

    QmlContext *parent;
    QmlEngine *engine;
    bool isInternal;

    QmlIntegerCache *propertyNames;
    QList<QVariant> propertyValues;
    int notifyIndex;

    QObjectList defaultObjects;
    int highPriorityCount;

    QList<QScriptValue> scripts;
    void addScript(const QString &script, QObject *scope, 
                   const QString &fileName = QString(), int lineNumber = 1);

    QUrl url;

    QmlTypeNameCache *imports;

    void init();

    void dump();
    void dump(int depth);

    void invalidateEngines();
    void refreshExpressions();
    QSet<QmlContext *> childContexts;

    QmlAbstractExpression *expressions;

    QObjectList contextObjects;

    struct ContextGuard : public QGuard<QObject>
    {
        ContextGuard() : priv(0), bindings(0) {}
        QmlContextPrivate *priv;
        QmlBinding_Id *bindings;
        ContextGuard &operator=(QObject *obj) {
            (QGuard<QObject>&)*this = obj; return *this;
        }
        void objectDestroyed(QObject *) { priv->destroyed(this); }
    };
    ContextGuard *idValues;
    int idValueCount;
    void setIdProperty(int, QObject *);
    void setIdPropertyData(QmlIntegerCache *);
    void destroyed(ContextGuard *);

    static QmlContextPrivate *get(QmlContext *context) {
        return static_cast<QmlContextPrivate *>(QObjectPrivate::get(context));
    }

    // Only used for debugging
    QList<QPointer<QObject> > instances;
};

QT_END_NAMESPACE

#endif // QMLCONTEXT_P_H
