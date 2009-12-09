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

#ifndef QMLGRAPHICSLAYOUTS_H
#define QMLGRAPHICSLAYOUTS_H

#include "qmlgraphicsitem.h"

#include "../util/qmlstate_p.h"

#include <QtCore/QObject>
#include <QtCore/QString>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)
class QmlGraphicsBasePositionerPrivate;

class Q_DECLARATIVE_EXPORT QmlGraphicsBasePositioner : public QmlGraphicsItem
{
    Q_OBJECT

    Q_PROPERTY(int spacing READ spacing WRITE setSpacing NOTIFY spacingChanged)
    Q_PROPERTY(QmlTransition *move READ move WRITE setMove)
    Q_PROPERTY(QmlTransition *add READ add WRITE setAdd)
    Q_PROPERTY(QmlTransition *remove READ remove WRITE setRemove)
public:
    enum AutoUpdateType { None = 0x0, Horizontal = 0x1, Vertical = 0x2, Both = 0x3 };
    QmlGraphicsBasePositioner(AutoUpdateType, QmlGraphicsItem *parent);

    int spacing() const;
    void setSpacing(int);

    QmlTransition *move() const;
    void setMove(QmlTransition *);

    QmlTransition *add() const;
    void setAdd(QmlTransition *);

    QmlTransition *remove() const;
    void setRemove(QmlTransition *);

protected:
    virtual void componentComplete();
    virtual QVariant itemChange(GraphicsItemChange, const QVariant &);
    virtual bool event(QEvent *);
    QSet<QmlGraphicsItem *>* newItems();
    QSet<QmlGraphicsItem *>* leavingItems();
    QSet<QmlGraphicsItem *>* items();
    void applyAdd(const QList<QPair<QString, QVariant> >& changes, QmlGraphicsItem* target);
    void applyMove(const QList<QPair<QString, QVariant> >& changes, QmlGraphicsItem* target);
    void applyRemove(const QList<QPair<QString, QVariant> >& changes, QmlGraphicsItem* target);
    void finishApplyTransitions();

Q_SIGNALS:
    void layoutItemChanged();
    void spacingChanged();

protected Q_SLOTS:
    virtual void doPositioning()=0;
    void prePositioning();

protected:
    QmlGraphicsBasePositioner(QmlGraphicsBasePositionerPrivate &dd, AutoUpdateType at, QmlGraphicsItem *parent);
    void setMovingItem(QmlGraphicsItem *);
    QList<QmlGraphicsItem *> positionedItems;

private:
    void applyTransition(const QList<QPair<QString, QVariant> >& changes, QmlGraphicsItem* target,
            QmlStateOperation::ActionList &actions);
    Q_DISABLE_COPY(QmlGraphicsBasePositioner)
    Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QmlGraphicsBasePositioner)
};

class Q_DECLARATIVE_EXPORT QmlGraphicsColumn : public QmlGraphicsBasePositioner
{
    Q_OBJECT
public:
    QmlGraphicsColumn(QmlGraphicsItem *parent=0);
protected Q_SLOTS:
    virtual void doPositioning();
private:
    Q_DISABLE_COPY(QmlGraphicsColumn)
};

class Q_DECLARATIVE_EXPORT QmlGraphicsRow: public QmlGraphicsBasePositioner
{
    Q_OBJECT
public:
    QmlGraphicsRow(QmlGraphicsItem *parent=0);
protected Q_SLOTS:
    virtual void doPositioning();
private:
    Q_DISABLE_COPY(QmlGraphicsRow)
};

class Q_DECLARATIVE_EXPORT QmlGraphicsGrid : public QmlGraphicsBasePositioner
{
    Q_OBJECT
    Q_PROPERTY(int rows READ rows WRITE setRows)
    Q_PROPERTY(int columns READ columns WRITE setcolumns)
public:
    QmlGraphicsGrid(QmlGraphicsItem *parent=0);

    int rows() const {return _rows;}
    void setRows(const int rows){_rows = rows;}

    int columns() const {return _columns;}
    void setcolumns(const int columns){_columns = columns;}
protected Q_SLOTS:
    virtual void doPositioning();

private:
    int _rows;
    int _columns;
    Q_DISABLE_COPY(QmlGraphicsGrid)
};

class QmlGraphicsFlowPrivate;
class Q_DECLARATIVE_EXPORT QmlGraphicsFlow: public QmlGraphicsBasePositioner
{
    Q_OBJECT
    Q_PROPERTY(Flow flow READ flow WRITE setFlow NOTIFY flowChanged)
public:
    QmlGraphicsFlow(QmlGraphicsItem *parent=0);

    Q_ENUMS(Flow)
    enum Flow { LeftToRight, TopToBottom };
    Flow flow() const;
    void setFlow(Flow);

Q_SIGNALS:
    void flowChanged();

protected Q_SLOTS:
    virtual void doPositioning();

protected:
    QmlGraphicsFlow(QmlGraphicsFlowPrivate &dd, QmlGraphicsItem *parent);
private:
    Q_DISABLE_COPY(QmlGraphicsFlow)
    Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QmlGraphicsFlow)
};


QT_END_NAMESPACE

QML_DECLARE_TYPE(QmlGraphicsColumn)
QML_DECLARE_TYPE(QmlGraphicsRow)
QML_DECLARE_TYPE(QmlGraphicsGrid)
QML_DECLARE_TYPE(QmlGraphicsFlow)

QT_END_HEADER

#endif
