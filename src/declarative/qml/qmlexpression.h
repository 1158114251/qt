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

#ifndef QMLEXPRESSION_H
#define QMLEXPRESSION_H

#include <QtCore/qobject.h>
#include <QtCore/qvariant.h>
#include <qmlerror.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)

class QString;
class QmlRefCount;
class QmlEngine;
class QmlContext;
class QmlExpressionPrivate;
class QmlBasicScript;
class Q_DECLARATIVE_EXPORT QmlExpression : public QObject
{
    Q_OBJECT
public:
    QmlExpression();
    QmlExpression(QmlContext *, const QString &, QObject *);
    virtual ~QmlExpression();

    QmlEngine *engine() const;
    QmlContext *context() const;

    QString expression() const;
    void clearExpression();
    virtual void setExpression(const QString &);
    bool isConstant() const;

    bool trackChange() const;
    void setTrackChange(bool);

    QString sourceFile() const;
    int lineNumber() const;
    void setSourceLocation(const QString &fileName, int line);

    QObject *scopeObject() const;

    bool hasError() const;
    void clearError();
    QmlError error() const;

public Q_SLOTS:
    QVariant value(bool *isUndefined = 0);

Q_SIGNALS:
    virtual void valueChanged();

protected:
    QmlExpression(QmlContext *, const QString &, QObject *, 
                  QmlExpressionPrivate &dd);
    QmlExpression(QmlContext *, void *, QmlRefCount *rc, QObject *me, const QString &,
                  int, QmlExpressionPrivate &dd);

private Q_SLOTS:
    void __q_notify();

private:
    Q_DECLARE_PRIVATE(QmlExpression)
    friend class QmlDebugger;
    friend class QmlContext;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QMLEXPRESSION_H

