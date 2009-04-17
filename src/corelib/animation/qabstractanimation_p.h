/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
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

#ifndef QABSTRACTANIMATION_P_H
#define QABSTRACTANIMATION_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of QIODevice. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qbasictimer.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qtimer.h>
#ifndef QT_EXPERIMENTAL_SOLUTION
#include <private/qobject_p.h>
#endif

QT_BEGIN_NAMESPACE

class QAnimationGroup;
class QAbstractAnimation;
#ifdef QT_EXPERIMENTAL_SOLUTION
class QAbstractAnimationPrivate
#else
class QAbstractAnimationPrivate : public QObjectPrivate
#endif
{
public:
    QAbstractAnimationPrivate()
        : state(QAbstractAnimation::Stopped),
          direction(QAbstractAnimation::Forward),
          deleteWhenStopped(false),
          totalCurrentTime(0),
          currentTime(0),
          iterationCount(1),
          currentIteration(0),
          group(0)
    {
    }

    virtual ~QAbstractAnimationPrivate() {}

    static QAbstractAnimationPrivate *get(QAbstractAnimation *q)
    {
        return q->d_func();
    }

    QAbstractAnimation::State state;
    QAbstractAnimation::Direction direction;
    bool deleteWhenStopped;
    void setState(QAbstractAnimation::State state);

    int totalCurrentTime;
    int currentTime;
    int iterationCount;
    int currentIteration;

    QAnimationGroup *group;
#ifdef QT_EXPERIMENTAL_SOLUTION
    QAbstractAnimation *q_ptr;
#endif

private:
    Q_DECLARE_PUBLIC(QAbstractAnimation)
};


class QUnifiedTimer : public QObject
{
private:
    QUnifiedTimer();

public:
    static QUnifiedTimer *instance();

    void timerEvent(QTimerEvent *);
    void updateTimer();
    void registerAnimation(QAbstractAnimation *animation);
    void unregisterAnimation(QAbstractAnimation *animation);


private:
    void updateRecentlyStartedAnimations();

    QBasicTimer animationTimer, startStopAnimationTimer;
    QTime time;
    int lastTick;
    QList<QAbstractAnimation*> animations, animationsToStart;
};

QT_END_NAMESPACE
#endif
