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

#ifndef QSTANDARDGESTURES_H
#define QSTANDARDGESTURES_H

#include <QtGui/qevent.h>
#include <QtCore/qbasictimer.h>

#include <QtGui/qgesture.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

class QPanGesturePrivate;
class Q_GUI_EXPORT QPanGesture : public QGesture
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QPanGesture)

    Q_PROPERTY(QSizeF totalOffset READ totalOffset)
    Q_PROPERTY(QSizeF lastOffset READ lastOffset)
    Q_PROPERTY(QSizeF offset READ offset)

public:
    QPanGesture(QWidget *gestureTarget, QObject *parent = 0);

    bool filterEvent(QEvent *event);

    QSizeF totalOffset() const;
    QSizeF lastOffset() const;
    QSizeF offset() const;

protected:
    void reset();

private:
    bool event(QEvent *event);
    bool eventFilter(QObject *receiver, QEvent *event);

    friend class QWidget;
    friend class QAbstractScrollAreaPrivate;
};

class QPinchGesturePrivate;
class Q_GUI_EXPORT QPinchGesture : public QGesture
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QPinchGesture)

public:
    enum WhatChange {
        ScaleFactorChanged = 0x1,
        RotationAngleChanged = 0x2,
        CenterPointChanged = 0x4
    };
    Q_DECLARE_FLAGS(WhatChanged, WhatChange)

    Q_PROPERTY(WhatChanged whatChanged READ whatChanged)

    Q_PROPERTY(qreal totalScaleFactor READ totalScaleFactor)
    Q_PROPERTY(qreal lastScaleFactor READ lastScaleFactor)
    Q_PROPERTY(qreal scaleFactor READ scaleFactor)

    Q_PROPERTY(qreal totalRotationAngle READ totalRotationAngle)
    Q_PROPERTY(qreal lastRotationAngle READ lastRotationAngle)
    Q_PROPERTY(qreal rotationAngle READ rotationAngle)

    Q_PROPERTY(QPointF startCenterPoint READ startCenterPoint)
    Q_PROPERTY(QPointF lastCenterPoint READ lastCenterPoint)
    Q_PROPERTY(QPointF centerPoint READ centerPoint)

public:

    QPinchGesture(QWidget *gestureTarget, QObject *parent = 0);

    bool filterEvent(QEvent *event);
    void reset();

    WhatChanged whatChanged() const;

    QPointF startCenterPoint() const;
    QPointF lastCenterPoint() const;
    QPointF centerPoint() const;

    qreal totalScaleFactor() const;
    qreal lastScaleFactor() const;
    qreal scaleFactor() const;

    qreal totalRotationAngle() const;
    qreal lastRotationAngle() const;
    qreal rotationAngle() const;

private:
    bool event(QEvent *event);
    bool eventFilter(QObject *receiver, QEvent *event);

    friend class QWidget;
};

class QSwipeGesturePrivate;
class Q_GUI_EXPORT QSwipeGesture : public QGesture
{
    Q_OBJECT
    Q_ENUMS(SwipeDirection)

    Q_PROPERTY(SwipeDirection horizontalDirection READ horizontalDirection)
    Q_PROPERTY(SwipeDirection verticalDirection READ verticalDirection)
    Q_PROPERTY(qreal swipeAngle READ swipeAngle)

    Q_DECLARE_PRIVATE(QSwipeGesture)

public:
    enum SwipeDirection { NoDirection, Left, Right, Up, Down };
    QSwipeGesture(QWidget *gestureTarget, QObject *parent = 0);

    bool filterEvent(QEvent *event);
    void reset();

    SwipeDirection horizontalDirection() const;
    SwipeDirection verticalDirection() const;
    qreal swipeAngle() const;

private:
    bool eventFilter(QObject *receiver, QEvent *event);

    friend class QWidget;
};
QT_END_NAMESPACE

QT_END_HEADER

#endif // QSTANDARDGESTURES_H
