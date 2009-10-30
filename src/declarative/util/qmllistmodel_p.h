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

#ifndef QMLLISTMODEL_H
#define QMLLISTMODEL_H

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QVariant>
#include <QtDeclarative/qfxglobal.h>
#include <QtDeclarative/qml.h>
#include <private/qlistmodelinterface_p.h>
#include <QtScript/qscriptvalue.h>


QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)

struct ModelNode;
class QmlListModel : public QListModelInterface
{
    Q_OBJECT
    Q_PROPERTY(int count READ count)

public:
    QmlListModel(QObject *parent=0);
    ~QmlListModel();

    virtual QList<int> roles() const;
    virtual QString toString(int role) const;
    virtual int count() const;
    virtual QHash<int,QVariant> data(int index, const QList<int> &roles = (QList<int>())) const;

    Q_INVOKABLE void clear();
    Q_INVOKABLE void remove(int index);
    Q_INVOKABLE void append(const QScriptValue&);
    Q_INVOKABLE void insert(int index, const QScriptValue&);
    Q_INVOKABLE void set(int index, const QScriptValue&);
    Q_INVOKABLE void set(int index, const QString& property, const QVariant& value);
    Q_INVOKABLE void move(int from, int to, int count);

private:
    QVariant valueForNode(ModelNode *) const;
    mutable QStringList roleStrings;
    friend class QmlListModelParser;
    friend struct ModelNode;

    void checkRoles() const;
    void addRole(const QString &) const;
    mutable bool _rolesOk;
    ModelNode *_root;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QmlListModel)

QT_END_HEADER

#endif // QMLLISTMODEL_H
