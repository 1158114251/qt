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

#include "qsignaltransition.h"
#include "qsignaltransition_p.h"
#include "qsignalevent.h"
#include "qstate.h"
#include "qstate_p.h"
#include "qstatemachine.h"
#include "qstatemachine_p.h"
#include <qdebug.h>

QT_BEGIN_NAMESPACE

/*!
  \class QSignalTransition

  \brief The QSignalTransition class provides a transition based on a Qt signal.

  \since 4.6
  \ingroup statemachine

  Typically you would use the overload of QState::addTransition() that takes a
  sender and signal as arguments, rather than creating QSignalTransition
  objects directly. QSignalTransition is part of \l{The State Machine
  Framework}.

  You can subclass QSignalTransition and reimplement eventTest() to make a
  signal transition conditional; the event object passed to eventTest() will
  be a QSignalEvent object. Example:

  \code
  class CheckedTransition : public QSignalTransition
  {
  public:
      CheckedTransition(QCheckBox *check)
          : QSignalTransition(check, SIGNAL(stateChanged(int))) {}
  protected:
      bool eventTest(QEvent *e) const {
          if (!QSignalTransition::eventTest(e))
              return false;
          QSignalEvent *se = static_cast<QSignalEvent*>(e);
          return (se->arguments().at(0).toInt() == Qt::Checked);
      }
  };

  ...

  QCheckBox *check = new QCheckBox();
  check->setTristate(true);

  QState *s1 = new QState();
  QState *s2 = new QState();
  CheckedTransition *t1 = new CheckedTransition(check);
  t1->setTargetState(s2);
  s1->addTransition(t1);
  \endcode
*/

/*!
    \property QSignalTransition::object

    \brief the sender object that this signal transition is associated with
*/

/*!
    \property QSignalTransition::signal

    \brief the signal that this signal transition is associated with
*/

QSignalTransitionPrivate::QSignalTransitionPrivate()
{
    sender = 0;
    signalIndex = -1;
}

QSignalTransitionPrivate *QSignalTransitionPrivate::get(QSignalTransition *q)
{
    return q->d_func();
}

void QSignalTransitionPrivate::invalidate()
{
    Q_Q(QSignalTransition);
    if (signalIndex != -1) {
        QState *source = sourceState();
        QStatePrivate *source_d = QStatePrivate::get(source);
        QStateMachinePrivate *mach = QStateMachinePrivate::get(source_d->machine());
        if (mach) {
            mach->unregisterSignalTransition(q);
            if (mach->configuration.contains(source))
                mach->registerSignalTransition(q);
        }
    }
}

/*!
  Constructs a new signal transition with the given \a sourceState.
*/
QSignalTransition::QSignalTransition(QState *sourceState)
    : QAbstractTransition(*new QSignalTransitionPrivate, sourceState)
{
}

/*!
  Constructs a new signal transition associated with the given \a signal of
  the given \a sender, and with the given \a sourceState.
*/
QSignalTransition::QSignalTransition(QObject *sender, const char *signal,
                                     QState *sourceState)
    : QAbstractTransition(*new QSignalTransitionPrivate, sourceState)
{
    Q_D(QSignalTransition);
    d->sender = sender;
    d->signal = signal;
}

/*!
  Constructs a new signal transition associated with the given \a signal of
  the given \a sender. The transition has the given \a targets and \a
  sourceState.
*/
QSignalTransition::QSignalTransition(QObject *sender, const char *signal,
                                     const QList<QAbstractState*> &targets,
                                     QState *sourceState)
    : QAbstractTransition(*new QSignalTransitionPrivate, targets, sourceState)
{
    Q_D(QSignalTransition);
    d->sender = sender;
    d->signal = signal;
}

/*!
  Destroys this signal transition.
*/
QSignalTransition::~QSignalTransition()
{
}

/*!
  Returns the sender object associated with this signal transition.
*/
QObject *QSignalTransition::senderObject() const
{
    Q_D(const QSignalTransition);
    return d->sender;
}

/*!
  Sets the \a sender object associated with this signal transition.
*/
void QSignalTransition::setSenderObject(QObject *sender)
{
    Q_D(QSignalTransition);
    if (sender == d->sender)
        return;
    d->sender = sender;
    d->invalidate();
}

/*!
  Returns the signal associated with this signal transition.
*/
QByteArray QSignalTransition::signal() const
{
    Q_D(const QSignalTransition);
    return d->signal;
}

/*!
  Sets the \a signal associated with this signal transition.
*/
void QSignalTransition::setSignal(const QByteArray &signal)
{
    Q_D(QSignalTransition);
    if (signal == d->signal)
        return;
    d->signal = signal;
    d->invalidate();
}

/*!
  \reimp

  The \a event is a QSignalEvent object.  The default implementation returns
  true if the event's sender and signal index match this transition, and
  returns false otherwise.
*/
bool QSignalTransition::eventTest(QEvent *event) const
{
    Q_D(const QSignalTransition);
#ifndef QT_STATEMACHINE_SOLUTION
    if (event->type() == QEvent::Signal) {
#else
    if (event->type() == QEvent::Type(QEvent::User-1)) {
#endif
        if (d->signalIndex == -1)
            return false;
        QSignalEvent *se = static_cast<QSignalEvent*>(event);
        return (se->sender() == d->sender)
            && (se->signalIndex() == d->signalIndex);
    }
    return false;
}

/*!
  \reimp
*/
void QSignalTransition::onTransition()
{
}

/*!
  \reimp
*/
bool QSignalTransition::event(QEvent *e)
{
    return QAbstractTransition::event(e);
}

QT_END_NAMESPACE
