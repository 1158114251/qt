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

#include "qeventtransition.h"
#include "qeventtransition_p.h"
#include "qwrappedevent.h"
#include "qstate.h"
#include "qstate_p.h"
#include "qstatemachine.h"
#include "qstatemachine_p.h"
#include <qdebug.h>

QT_BEGIN_NAMESPACE

/*!
  \class QEventTransition

  \brief The QEventTransition class provides a QObject-specific transition for Qt events.

  \since 4.6
  \ingroup statemachine

  A QEventTransition object binds an event to a particular QObject.
  QEventTransition is part of \l{The State Machine Framework}.

  Example:

  \code
  QPushButton *button = ...;
  QState *s1 = ...;
  QState *s2 = ...;
  // If in s1 and the button receives an Enter event, transition to s2
  QEventTransition *enterTransition = new QEventTransition(button, QEvent::Enter);
  enterTransition->setTargetState(s2);
  s1->addTransition(enterTransition);
  // If in s2 and the button receives an Exit event, transition back to s1
  QEventTransition *leaveTransition = new QEventTransition(button, QEvent::Leave);
  leaveTransition->setTargetState(s1);
  s2->addTransition(leaveTransition);
  \endcode

  \section1 Subclassing

  When reimplementing the eventTest() function, you should first call the base
  implementation to verify that the event is a QWrappedEvent for the proper
  object and event type. You may then cast the event to a QWrappedEvent and
  get the original event by calling QWrappedEvent::event(), and perform
  additional checks on that object.

  \sa QState::addTransition()
*/

/*!
    \property QEventTransition::eventObject

    \brief the event source that this event transition is associated with
*/

/*!
    \property QEventTransition::eventType

    \brief the type of event that this event transition is associated with
*/
QEventTransitionPrivate::QEventTransitionPrivate()
{
    object = 0;
    eventType = QEvent::None;
    registered = false;
}

QEventTransitionPrivate *QEventTransitionPrivate::get(QEventTransition *q)
{
    return q->d_func();
}

void QEventTransitionPrivate::invalidate()
{
    Q_Q(QEventTransition);
    if (registered) {
        QState *source = sourceState();
        QStatePrivate *source_d = QStatePrivate::get(source);
        QStateMachinePrivate *mach = QStateMachinePrivate::get(source_d->machine());
        if (mach) {
            mach->unregisterEventTransition(q);
            if (mach->configuration.contains(source))
                mach->registerEventTransition(q);
        }
    }
}

/*!
  Constructs a new QEventTransition object with the given \a sourceState.
*/
QEventTransition::QEventTransition(QState *sourceState)
    : QAbstractTransition(*new QEventTransitionPrivate, sourceState)
{
}

/*!
  Constructs a new QEventTransition object associated with events of the given
  \a type for the given \a object, and with the given \a sourceState.
*/
QEventTransition::QEventTransition(QObject *object, QEvent::Type type,
                                   QState *sourceState)
    : QAbstractTransition(*new QEventTransitionPrivate, sourceState)
{
    Q_D(QEventTransition);
    d->registered = false;
    d->object = object;
    d->eventType = type;
}

/*!
  Constructs a new QEventTransition object associated with events of the given
  \a type for the given \a object. The transition has the given \a targets and
  \a sourceState.
*/
QEventTransition::QEventTransition(QObject *object, QEvent::Type type,
                                   const QList<QAbstractState*> &targets,
                                   QState *sourceState)
    : QAbstractTransition(*new QEventTransitionPrivate, targets, sourceState)
{
    Q_D(QEventTransition);
    d->registered = false;
    d->object = object;
    d->eventType = type;
}

/*!
  \internal
*/
QEventTransition::QEventTransition(QEventTransitionPrivate &dd, QState *parent)
    : QAbstractTransition(dd, parent)
{
}

/*!
  \internal
*/
QEventTransition::QEventTransition(QEventTransitionPrivate &dd, QObject *object,
                                   QEvent::Type type, QState *parent)
    : QAbstractTransition(dd, parent)
{
    Q_D(QEventTransition);
    d->registered = false;
    d->object = object;
    d->eventType = type;
}

/*!
  \internal
*/
QEventTransition::QEventTransition(QEventTransitionPrivate &dd, QObject *object,
                                   QEvent::Type type, const QList<QAbstractState*> &targets,
                                   QState *parent)
    : QAbstractTransition(dd, targets, parent)
{
    Q_D(QEventTransition);
    d->registered = false;
    d->object = object;
    d->eventType = type;
}

/*!
  Destroys this QObject event transition.
*/
QEventTransition::~QEventTransition()
{
}

/*!
  Returns the event type that this event transition is associated with.
*/
QEvent::Type QEventTransition::eventType() const
{
    Q_D(const QEventTransition);
    return d->eventType;
}

/*!
  Sets the event \a type that this event transition is associated with.
*/
void QEventTransition::setEventType(QEvent::Type type)
{
    Q_D(QEventTransition);
    if (d->eventType == type)
        return;
    d->eventType = type;
    d->invalidate();
}

/*!
  Returns the event source associated with this event transition.
*/
QObject *QEventTransition::eventObject() const
{
    Q_D(const QEventTransition);
    return d->object;
}

/*!
  Sets the event source associated with this event transition to be the given
  \a object.
*/
void QEventTransition::setEventObject(QObject *object)
{
    Q_D(QEventTransition);
    if (d->object == object)
        return;
    d->object = object;
    d->invalidate();
}

/*!
  \reimp
*/
bool QEventTransition::eventTest(QEvent *event)
{
    Q_D(const QEventTransition);
    if (event->type() == QEvent::Wrapped) {
        QWrappedEvent *we = static_cast<QWrappedEvent*>(event);
        return (we->object() == d->object)
            && (we->event()->type() == d->eventType);
    }
    return false;
}

/*!
  \reimp
*/
void QEventTransition::onTransition(QEvent *event)
{
    Q_UNUSED(event);
}

/*!
  \reimp
*/
bool QEventTransition::event(QEvent *e)
{
    return QAbstractTransition::event(e);
}

QT_END_NAMESPACE
