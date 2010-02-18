/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef QMLGRAPHICSFLICKABLE_P_H
#define QMLGRAPHICSFLICKABLE_P_H

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

#include "qmlgraphicsflickable_p.h"

#include "qmlgraphicsitem_p.h"

#include <qml.h>
#include <qmltimeline_p_p.h>
#include <qmlanimation_p_p.h>

#include <qdatetime.h>

QT_BEGIN_NAMESPACE

class QmlGraphicsFlickableVisibleArea;
class QmlGraphicsFlickablePrivate : public QmlGraphicsItemPrivate
{
    Q_DECLARE_PUBLIC(QmlGraphicsFlickable)

public:
    QmlGraphicsFlickablePrivate();
    void init();
    virtual void flickX(qreal velocity);
    virtual void flickY(qreal velocity);
    virtual void fixupX();
    virtual void fixupY();
    void updateBeginningEnd();

    void captureDelayedPress(QGraphicsSceneMouseEvent *event);
    void clearDelayedPress();

    void setRoundedViewportX(qreal x);
    void setRoundedViewportY(qreal y);

public:
    QmlGraphicsItem *viewport;
    QmlTimeLineValueProxy<QmlGraphicsFlickablePrivate> _moveX;
    QmlTimeLineValueProxy<QmlGraphicsFlickablePrivate> _moveY;
    QmlTimeLine timeline;
    qreal vWidth;
    qreal vHeight;
    bool overShoot : 1;
    bool flicked : 1;
    bool moving : 1;
    bool stealMouse : 1;
    bool pressed : 1;
    bool atXEnd : 1;
    bool atXBeginning : 1;
    bool atYEnd : 1;
    bool atYBeginning : 1;
    bool interactive : 1;
    QTime lastPosTime;
    QPointF lastPos;
    QPointF pressPos;
    qreal pressX;
    qreal pressY;
    qreal velocityX;
    qreal velocityY;
    QTime pressTime;
    QmlTimeLineEvent fixupXEvent;
    QmlTimeLineEvent fixupYEvent;
    qreal deceleration;
    qreal maxVelocity;
    QTime velocityTime;
    QPointF lastFlickablePosition;
    qreal reportedVelocitySmoothing;
    qreal flickTargetX;
    qreal flickTargetY;
    QGraphicsSceneMouseEvent *delayedPressEvent;
    QGraphicsItem *delayedPressTarget;
    QBasicTimer delayedPressTimer;
    int pressDelay;
    int fixupDuration;

    void updateVelocity();
    struct Velocity : public QmlTimeLineValue
    {
        Velocity(QmlGraphicsFlickablePrivate *p)
            : parent(p) {}
        virtual void setValue(qreal v) {
            QmlTimeLineValue::setValue(v);
            parent->updateVelocity();
        }
        QmlGraphicsFlickablePrivate *parent;
    };
    Velocity horizontalVelocity;
    Velocity verticalVelocity;
    int vTime;
    QmlTimeLine velocityTimeline;
    QmlGraphicsFlickableVisibleArea *visibleArea;
    QmlGraphicsFlickable::FlickDirection flickDirection;

    void handleMousePressEvent(QGraphicsSceneMouseEvent *);
    void handleMouseMoveEvent(QGraphicsSceneMouseEvent *);
    void handleMouseReleaseEvent(QGraphicsSceneMouseEvent *);

    // flickableData property
    void data_removeAt(int);
    int data_count() const;
    void data_append(QObject *);
    void data_insert(int, QObject *);
    QObject *data_at(int) const;
    void data_clear();

    friend class QmlGraphicsFlickableVisibleArea;
    QML_DECLARE_LIST_PROXY(QmlGraphicsFlickablePrivate, QObject *, data)
};

class QmlGraphicsFlickableVisibleArea : public QObject
{
    Q_OBJECT

    Q_PROPERTY(qreal xPosition READ xPosition NOTIFY pageChanged)
    Q_PROPERTY(qreal yPosition READ yPosition NOTIFY pageChanged)
    Q_PROPERTY(qreal widthRatio READ widthRatio NOTIFY pageChanged)
    Q_PROPERTY(qreal heightRatio READ heightRatio NOTIFY pageChanged)

public:
    QmlGraphicsFlickableVisibleArea(QmlGraphicsFlickable *parent=0);

    qreal xPosition() const;
    qreal widthRatio() const;
    qreal yPosition() const;
    qreal heightRatio() const;

    void updateVisible();

signals:
    void pageChanged();

private:
    QmlGraphicsFlickable *flickable;
    qreal m_xPosition;
    qreal m_widthRatio;
    qreal m_yPosition;
    qreal m_heightRatio;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QmlGraphicsFlickableVisibleArea)

#endif
