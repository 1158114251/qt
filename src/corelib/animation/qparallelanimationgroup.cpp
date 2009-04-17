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

/*!
    \class QParallelAnimationGroup
    \brief The QParallelAnimationGroup class provides a parallel group of animations.
    \since 4.5
    \ingroup animation
    \preliminary

    The animations are all started at the same time, and run in parallel. The animation group
    finishes when the longest lasting animation has finished.
*/

#ifndef QT_NO_ANIMATION

#include "qparallelanimationgroup.h"
#include "qparallelanimationgroup_p.h"
//#define QANIMATION_DEBUG
QT_BEGIN_NAMESPACE

/*!
    Constructs a QParallelAnimationGroup.
    \a parent is passed to QObject's constructor.
*/
QParallelAnimationGroup::QParallelAnimationGroup(QObject *parent)
    : QAnimationGroup(*new QParallelAnimationGroupPrivate, parent)
{
}

/*!
    \internal
*/
QParallelAnimationGroup::QParallelAnimationGroup(QParallelAnimationGroupPrivate &dd,
                                                 QObject *parent)
    : QAnimationGroup(dd, parent)
{
}

/*!
    Destroys the animation group. It will also destroy all its animations.
*/
QParallelAnimationGroup::~QParallelAnimationGroup()
{
}

/*!
    \reimp
*/
int QParallelAnimationGroup::duration() const
{
    Q_D(const QParallelAnimationGroup);
    int ret = 0;

    for (int i = 0; i < d->animations.size(); ++i) {
        QAbstractAnimation *animation = d->animations.at(i);
        const int currentDuration = animation->totalDuration();
        if (currentDuration == -1)
            return -1; // Undetermined length

        ret = qMax(ret, currentDuration);
    }

    return ret;
}

/*!
    \reimp
*/
void QParallelAnimationGroup::updateCurrentTime(int)
{
    Q_D(QParallelAnimationGroup);
    if (d->animations.isEmpty())
        return;

    if (d->currentIteration > d->lastIteration) {
        // simulate completion of the loop
        int dura = duration();
        if (dura > 0) {
            foreach (QAbstractAnimation *animation, d->animations) {
                animation->setCurrentTime(dura);   // will stop
            }
        }
    } else if (d->currentIteration < d->lastIteration) {
        // simulate completion of the loop seeking backwards
        foreach (QAbstractAnimation *animation, d->animations) {
            animation->setCurrentTime(0);
            animation->stop();
        }
    }

    bool timeFwd = ((d->currentIteration == d->lastIteration && d->currentTime >= d->lastCurrentTime)
                   || d->currentIteration > d->lastIteration);
#ifdef QANIMATION_DEBUG
    qDebug("QParallellAnimationGroup %5d: setCurrentTime(%d), loop:%d, last:%d, timeFwd:%d, lastcurrent:%d, %d",
        __LINE__, d->currentTime, d->currentIteration, d->lastIteration, timeFwd, d->lastCurrentTime, state());
#endif
    // finally move into the actual time of the current loop
    foreach (QAbstractAnimation *animation, d->animations) {
        const int dura = animation->totalDuration();
        if (dura == -1 && d->isUncontrolledAnimationFinished(animation))
            continue;
        if (dura == -1 || (d->currentTime <= dura && dura != 0)
            || (dura == 0 && d->currentIteration != d->lastIteration)) {
            switch (state()) {
            case Running:
                animation->start();
                break;
            case Paused:
                animation->pause();
                break;
            case Stopped:
            default:
                break;
            }
        }

        if (dura <= 0) {
            if (dura == -1)
                animation->setCurrentTime(d->currentTime);
            continue;
        }

        if ((timeFwd && d->lastCurrentTime <= dura)
            || (!timeFwd && d->currentTime <= dura))
                animation->setCurrentTime(d->currentTime);
        if (d->currentTime > dura)
            animation->stop();
    }
    d->lastIteration = d->currentIteration;
    d->lastCurrentTime = d->currentTime;
}

/*!
    \reimp
*/
void QParallelAnimationGroup::updateState(QAbstractAnimation::State oldState,
                                          QAbstractAnimation::State newState)
{
    Q_D(QParallelAnimationGroup);
    QAnimationGroup::updateState(oldState, newState);

    switch (newState) {
    case Stopped:
        foreach (QAbstractAnimation *animation, d->animations)
            animation->stop();
        d->disconnectUncontrolledAnimations();
        break;
    case Paused:
        foreach (QAbstractAnimation *animation, d->animations)
            animation->pause();
        break;
    case Running:
        d->connectUncontrolledAnimations();
        foreach (QAbstractAnimation *animation, d->animations) {
            animation->stop();
            animation->setDirection(d->direction);
            animation->start();
        }
        break;
    }
}

void QParallelAnimationGroupPrivate::_q_uncontrolledAnimationFinished()
{
    Q_Q(QParallelAnimationGroup);

    QAbstractAnimation *animation = qobject_cast<QAbstractAnimation *>(q->sender());
    Q_ASSERT(animation);

    int uncontrolledRunningCount = 0;
    if (animation->duration() == -1 || animation->iterationCount() < 0) {
        QHash<QAbstractAnimation *, int>::iterator it = uncontrolledFinishTime.begin();
        while (it != uncontrolledFinishTime.end()) {
            if (it.key() == animation) {
                *it = animation->currentTime();
            }
            if (it.value() == -1)
                ++uncontrolledRunningCount;
            ++it;
        }
    }

    if (uncontrolledRunningCount > 0)
        return;

    int maxDuration = 0;
    foreach (QAbstractAnimation *a, animations)
        maxDuration = qMax(maxDuration, a->totalDuration());

    if (currentTime >= maxDuration)
        q->stop();
}

void QParallelAnimationGroupPrivate::disconnectUncontrolledAnimations()
{
    Q_Q(QParallelAnimationGroup);

    QHash<QAbstractAnimation *, int>::iterator it = uncontrolledFinishTime.begin();
    while (it != uncontrolledFinishTime.end()) {
        QObject::disconnect(it.key(), SIGNAL(finished()), q, SLOT(_q_uncontrolledAnimationFinished()));
        ++it;
    }

    uncontrolledFinishTime.clear();
}

void QParallelAnimationGroupPrivate::connectUncontrolledAnimations()
{
    Q_Q(QParallelAnimationGroup);

    foreach (QAbstractAnimation *animation, animations) {
        if (animation->duration() == -1 || animation->iterationCount() < 0) {
            uncontrolledFinishTime[animation] = -1;
            QObject::connect(animation, SIGNAL(finished()), q, SLOT(_q_uncontrolledAnimationFinished()));
        }
    }
}

bool QParallelAnimationGroupPrivate::isUncontrolledAnimationFinished(QAbstractAnimation *anim) const
{
    return uncontrolledFinishTime.value(anim, -1) >= 0;
}

/*!
    \reimp
*/
void QParallelAnimationGroup::updateDirection(QAbstractAnimation::Direction direction)
{
    Q_D(QParallelAnimationGroup);
    //we need to update the direction of the current animation
    if (state() != Stopped) {
        foreach(QAbstractAnimation *anim, d->animations) {
            anim->setDirection(direction);
        }
    } else {
        if (direction == Forward) {
            d->lastIteration = 0;
            d->lastCurrentTime = 0;
        } else {
            // Looping backwards with iterationCount == -1 does not really work well...
            d->lastIteration = (d->iterationCount == -1 ? 0 : d->iterationCount - 1);
            d->lastCurrentTime = duration();
        }
    }
}

/*!
    \reimp
*/
bool QParallelAnimationGroup::event(QEvent *event)
{
    return QAnimationGroup::event(event);
}

QT_END_NAMESPACE

#include "moc_qparallelanimationgroup.cpp"

#endif //QT_NO_ANIMATION
