/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#include "qgesture.h"
#include "private/qgesture_p.h"

QT_BEGIN_NAMESPACE

 /*!
    \class QGesture
    \since 4.6

    \brief The QGesture class represents a gesture, containing all
    properties that describe a gesture.

    QGesture objects are delivered to widgets and
    \l{QGraphicsObject}{QGraphicsObject}s with a QGestureEvent.

    The class has a list of properties that can be queried by the user to get
    some gesture-specific arguments (i.e. an scale factor of the Pinch
    gesture).

    When creating custom gesture recognizers, they might add new dynamic
    properties to the QGesture object, or custom gesture recognizer developers
    might subclass the QGesture class (or any of classes that derive from it)
    to provide additional information.

    \sa QGestureEvent, QGestureRecognizer
*/

/*!
    Constructs a new gesture object with the given \a parent.

    QGesture objects are created by gesture recognizers in the
    QGestureRecognizer::createGesture() function.
*/
QGesture::QGesture(QObject *parent)
    : QObject(*new QGesturePrivate, parent)
{
    d_func()->gestureType = Qt::CustomGesture;
}

/*!
    \internal
*/
QGesture::QGesture(QGesturePrivate &dd, QObject *parent)
    : QObject(dd, parent)
{
}

/*!
    Destroys the gesture object.
*/
QGesture::~QGesture()
{
}

/*!
    \property QGesture::state

    The current state of the gesture.
*/

/*!
    \property QGesture::gestureType

    The type of the gesture.
*/

/*!
    \property QGesture::hotSpot

    \brief The point that is used to find the receiver for the gesture event.

    If the hotSpot is not set, targetObject is used as the receiver of the
    gesture event.
*/

/*!
    \property QGesture::hasHotSpot

    Whether the hotSpot property is set.
*/

/*!
    \property QGesture::targetObject

    The target object which will receive the gesture event if the hotSpot is
    not set.
*/

Qt::GestureType QGesture::gestureType() const
{
    return d_func()->gestureType;
}

Qt::GestureState QGesture::state() const
{
    return d_func()->state;
}

QObject *QGesture::targetObject() const
{
    return d_func()->targetObject;
}

void QGesture::setTargetObject(QObject *value)
{
    d_func()->targetObject = value;
}

QPointF QGesture::hotSpot() const
{
    return d_func()->hotSpot;
}

void QGesture::setHotSpot(const QPointF &value)
{
    Q_D(QGesture);
    d->hotSpot = value;
    d->isHotSpotSet = true;
}

bool QGesture::hasHotSpot() const
{
    return d_func()->isHotSpotSet;
}

void QGesture::unsetHotSpot()
{
    d_func()->isHotSpotSet = false;
}

// QPanGesture

QPanGesture::QPanGesture(QObject *parent)
    : QGesture(*new QPanGesturePrivate, parent)
{
    d_func()->gestureType = Qt::PanGesture;
}

QSizeF QPanGesture::totalOffset() const
{
    return d_func()->totalOffset;
}

QSizeF QPanGesture::lastOffset() const
{
    return d_func()->lastOffset;
}

QSizeF QPanGesture::offset() const
{
    return d_func()->offset;
}

qreal QPanGesture::acceleration() const
{
    return d_func()->acceleration;
}


void QPanGesture::setTotalOffset(const QSizeF &value)
{
    d_func()->totalOffset = value;
}

void QPanGesture::setLastOffset(const QSizeF &value)
{
    d_func()->lastOffset = value;
}

void QPanGesture::setOffset(const QSizeF &value)
{
    d_func()->offset = value;
}

void QPanGesture::setAcceleration(qreal value)
{
    d_func()->acceleration = value;
}

// QPinchGesture

QPinchGesture::QPinchGesture(QObject *parent)
    : QGesture(*new QPinchGesturePrivate, parent)
{
    d_func()->gestureType = Qt::PinchGesture;
}

QPinchGesture::WhatChanged QPinchGesture::whatChanged() const
{
    return d_func()->whatChanged;
}

void QPinchGesture::setWhatChanged(QPinchGesture::WhatChanged value)
{
    d_func()->whatChanged = value;
}


QPointF QPinchGesture::startCenterPoint() const
{
    return d_func()->startCenterPoint;
}

QPointF QPinchGesture::lastCenterPoint() const
{
    return d_func()->lastCenterPoint;
}

QPointF QPinchGesture::centerPoint() const
{
    return d_func()->centerPoint;
}

void QPinchGesture::setStartCenterPoint(const QPointF &value)
{
    d_func()->startCenterPoint = value;
}

void QPinchGesture::setLastCenterPoint(const QPointF &value)
{
    d_func()->lastCenterPoint = value;
}

void QPinchGesture::setCenterPoint(const QPointF &value)
{
    d_func()->centerPoint = value;
}


qreal QPinchGesture::totalScaleFactor() const
{
    return d_func()->totalScaleFactor;
}

qreal QPinchGesture::lastScaleFactor() const
{
    return d_func()->lastScaleFactor;
}

qreal QPinchGesture::scaleFactor() const
{
    return d_func()->scaleFactor;
}

void QPinchGesture::setTotalScaleFactor(qreal value)
{
    d_func()->totalScaleFactor = value;
}

void QPinchGesture::setLastScaleFactor(qreal value)
{
    d_func()->lastScaleFactor = value;
}

void QPinchGesture::setScaleFactor(qreal value)
{
    d_func()->scaleFactor = value;
}


qreal QPinchGesture::totalRotationAngle() const
{
    return d_func()->totalRotationAngle;
}

qreal QPinchGesture::lastRotationAngle() const
{
    return d_func()->lastRotationAngle;
}

qreal QPinchGesture::rotationAngle() const
{
    return d_func()->rotationAngle;
}

void QPinchGesture::setTotalRotationAngle(qreal value)
{
    d_func()->totalRotationAngle = value;
}

void QPinchGesture::setLastRotationAngle(qreal value)
{
    d_func()->lastRotationAngle = value;
}

void QPinchGesture::setRotationAngle(qreal value)
{
    d_func()->rotationAngle = value;
}

// QSwipeGesture

QSwipeGesture::QSwipeGesture(QObject *parent)
    : QGesture(*new QSwipeGesturePrivate, parent)
{
    d_func()->gestureType = Qt::SwipeGesture;
}

QSwipeGesture::SwipeDirection QSwipeGesture::horizontalDirection() const
{
    return d_func()->horizontalDirection;
}

QSwipeGesture::SwipeDirection QSwipeGesture::verticalDirection() const
{
    return d_func()->verticalDirection;
}

void QSwipeGesture::setHorizontalDirection(QSwipeGesture::SwipeDirection value)
{
    d_func()->horizontalDirection = value;
}

void QSwipeGesture::setVerticalDirection(QSwipeGesture::SwipeDirection value)
{
    d_func()->verticalDirection = value;
}

qreal QSwipeGesture::swipeAngle() const
{
    return d_func()->swipeAngle;
}

void QSwipeGesture::setSwipeAngle(qreal value)
{
    d_func()->swipeAngle = value;
}

QT_END_NAMESPACE
