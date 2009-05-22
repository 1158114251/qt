/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#include "qanimationstate.h"

#include <QtCore/qstate.h>
#include <private/qstate_p.h>


QT_BEGIN_NAMESPACE

/*!
\class QAnimationState

\brief The QAnimationState class provides state that handle an animation and emit
a signal when this animation is finished.

\ingroup statemachine

QAnimationState provides a state that handle an animation. It will start this animation
when the state is entered and stop it when it is leaved. When the animation has finished the
state emit animationFinished signal.
QAnimationState is part of \l{The State Machine Framework}.

\code
QStateMachine machine;
QAnimationState *s = new QAnimationState(machine->rootState());
QPropertyAnimation *animation = new QPropertyAnimation(obj, "pos");
s->setAnimation(animation);
QState *s2 = new QState(machine->rootState());
s->addTransition(s, SIGNAL(animationFinished()), s2);
machine.start();
\endcode

\sa QState, {The Animation Framework}
*/


#ifndef QT_NO_ANIMATION

class QAnimationStatePrivate : public QStatePrivate
{
    Q_DECLARE_PUBLIC(QAnimationState)
public:
    QAnimationStatePrivate()
        : animation(0)
    {

    }
    ~QAnimationStatePrivate() {}

    QAbstractAnimation *animation;
};

/*!
  Constructs a new state with the given \a parent state.
*/
QAnimationState::QAnimationState(QState *parent)
    : QState(*new QAnimationStatePrivate, parent)
{
}

/*!
  Destroys the animation state.
*/
QAnimationState::~QAnimationState()
{
}

/*!
  Set an \a animation for this QAnimationState. If an animation was previously handle by this
  state then it won't emit animationFinished for the old animation. The QAnimationState doesn't
  take the ownership of the animation.
*/
void QAnimationState::setAnimation(QAbstractAnimation *animation)
{
    Q_D(QAnimationState);

    if (animation == d->animation)
        return;

    //Disconnect from the previous animation if exist
    if(d->animation)
        disconnect(d->animation, SIGNAL(finished()), this, SIGNAL(animationFinished()));

    d->animation = animation;

    if (d->animation) {
        //connect the new animation
        connect(d->animation, SIGNAL(finished()), this, SIGNAL(animationFinished()));
    }
}

/*!
  Returns the animation handle by this animation state, or 0 if there is no animation.
*/
QAbstractAnimation* QAnimationState::animation() const
{
    Q_D(const QAnimationState);
    return d->animation;
}

/*!
  \reimp
*/
void QAnimationState::onEntry(QEvent *)
{
    Q_D(QAnimationState);
    if (d->animation)
        d->animation->start();
}

/*!
  \reimp
*/
void QAnimationState::onExit(QEvent *)
{
    Q_D(QAnimationState);
    if (d->animation)
        d->animation->stop();
}

/*!
  \reimp
*/
bool QAnimationState::event(QEvent *e)
{
    return QState::event(e);
}

QT_END_NAMESPACE

#endif
