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

#ifndef QMLGRAPHICSREPEATER_H
#define QMLGRAPHICSREPEATER_H

#include <QtDeclarative/qmlgraphicsitem.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)

class QmlGraphicsRepeaterPrivate;
class Q_DECLARATIVE_EXPORT QmlGraphicsRepeater : public QmlGraphicsItem
{
    Q_OBJECT

    Q_PROPERTY(QVariant model READ model WRITE setModel)
    Q_PROPERTY(QmlComponent *delegate READ delegate WRITE setDelegate)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_CLASSINFO("DefaultProperty", "delegate")

public:
    QmlGraphicsRepeater(QmlGraphicsItem *parent=0);
    virtual ~QmlGraphicsRepeater();

    QVariant model() const;
    void setModel(const QVariant &);

    QmlComponent *delegate() const;
    void setDelegate(QmlComponent *);

    int count() const;

Q_SIGNALS:
    void countChanged();

private:
    void clear();
    void regenerate();

protected:
    virtual void componentComplete();
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    QmlGraphicsRepeater(QmlGraphicsRepeaterPrivate &dd, QmlGraphicsItem *parent);

private Q_SLOTS:
    void itemsInserted(int,int);
    void itemsRemoved(int,int);
    void itemsMoved(int,int,int);

private:
    Q_DISABLE_COPY(QmlGraphicsRepeater)
    Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QmlGraphicsRepeater)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QmlGraphicsRepeater)

QT_END_HEADER

#endif // QMLGRAPHICSREPEATER_H
