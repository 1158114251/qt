/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
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

#ifndef QGESTURE_H
#define QGESTURE_H

#include "qobject.h"
#include "qlist.h"
#include "qdatetime.h"
#include "qpoint.h"
#include "qrect.h"
#include "qsharedpointer.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

class QGesturePrivate;
class Q_GUI_EXPORT QGesture : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QGesture)
public:
    explicit QGesture(QObject *parent, const QString &type,
                      Qt::GestureState state = Qt::GestureStarted);
    QGesture(QObject *parent,
             const QString &type, const QPoint &startPos,
             const QPoint &lastPos, const QPoint &pos, const QRect &rect,
             const QPoint &hotSpot, const QDateTime &startTime,
             uint duration, Qt::GestureState state);
    virtual ~QGesture();

    inline QString gestureType() const { return gestureType_; }
    inline Qt::GestureState state() const { return gestureState_; }

    QRect rect() const;
    QPoint hotSpot() const;
    QDateTime startTime() const;
    uint duration() const;

    QPoint startPos() const;
    QPoint lastPos() const;
    QPoint pos() const;

protected:
    QGesture(QGesturePrivate &dd, QObject *parent, const QString &type, Qt::GestureState state);
    //### virtual void translateCoordinates(const QPoint &offset);

private:
    QString gestureType_;
    Qt::GestureState gestureState_;
};

class QPannableGesturePrivate;
class Q_GUI_EXPORT QPannableGesture : public QGesture
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QPannableGesture)

public:
    enum DirectionType
    {
        None = 0,
        LeftDown = 1,
        DownLeft = LeftDown,
        Down = 2,
        RightDown = 3,
        DownRight = RightDown,
        Left = 4,
        Right = 6,
        LeftUp = 7,
        UpLeft = LeftUp,
        Up = 8,
        RightUp = 9,
        UpRight = RightUp
    };

public:
    QPannableGesture(QObject *parent, const QPoint &startPos,
                     const QPoint &lastPos, const QPoint &pos, const QRect &rect,
                     const QPoint &hotSpot, const QDateTime &startTime,
                     uint duration, Qt::GestureState state);
    ~QPannableGesture();

    DirectionType lastDirection() const;
    DirectionType direction() const;

    friend class QGestureRecognizerPan;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QGESTURE_H
