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

#ifndef QMACSWIPEGESTURERECOGNIZER_MAC_P_H
#define QMACSWIPEGESTURERECOGNIZER_MAC_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qtimer.h"
#include "qpoint.h"
#include "qgesturerecognizer.h"

QT_BEGIN_NAMESPACE

class QMacSwipeGestureRecognizer : public QGestureRecognizer
{
public:
    QMacSwipeGestureRecognizer();

    QGesture *create(QObject *target);
    QGestureRecognizer::Result recognize(QGesture *gesture, QObject *watched, QEvent *event);
    void reset(QGesture *gesture);
};

class QMacPinchGestureRecognizer : public QGestureRecognizer
{
public:
    QMacPinchGestureRecognizer();

    QGesture *create(QObject *target);
    QGestureRecognizer::Result recognize(QGesture *gesture, QObject *watched, QEvent *event);
    void reset(QGesture *gesture);
};

#if defined(QT_MAC_USE_COCOA)

class QMacPanGestureRecognizer : public QObject, public QGestureRecognizer
{
public:
    QMacPanGestureRecognizer();

    QGesture *create(QObject *target);
    QGestureRecognizer::Result recognize(QGesture *gesture, QObject *watched, QEvent *event);
    void reset(QGesture *gesture);
private:
    QPointF _startPos;
    QBasicTimer _panTimer;
    bool _panCanceled;
};

#endif

QT_END_NAMESPACE

#endif // QMACSWIPEGESTURERECOGNIZER_MAC_P_H
