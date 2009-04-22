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

#include "qabstractstate.h"
#include "qabstractstate_p.h"
#include "qstatemachine.h"
#include "qstatemachine_p.h"
#include "qstate.h"

QT_BEGIN_NAMESPACE

/*!
  \class QAbstractState

  \brief The QAbstractState class is the base class of states of a QStateMachine.

  \since 4.6
  \ingroup statemachine

  The QAbstractState class is the abstract base class of states that are part
  of a QStateMachine. It defines the interface that all state objects have in
  common. QAbstractState is part of \l{The State Machine Framework}.

  The assignProperty() function is used for defining property assignments that
  should be performed when a state is entered.

  The entered() signal is emitted when the state has been entered. The
  exited() signal is emitted when the state has been exited.

  The parentState() function returns the state's parent state.

  \section1 Subclassing

  The onEntry() function is called when the state is entered; reimplement this
  function to perform custom processing when the state is entered.

  The onExit() function is called when the state is exited; reimplement this
  function to perform custom processing when the state is exited.
*/

/*!
   \enum QAbstractState::RestorePolicy

   This enum specifies the restore policy type for a state. The restore policy
   takes effect when the machine enters a state which sets one or more
   properties. If the restore policy of the state is set to RestoreProperties,
   the state machine will save the original value of the property before the
   new value is set.

   Later, when the machine either enters a state which has its restore policy
   set to DoNotRestoreProperties or when it enters a state which does not set
   a value for the given property, the property will automatically be restored
   to its initial value.

   Only one initial value will be saved for any given property. If a value for a property has 
   already been saved by the state machine, it will not be overwritten until the property has been
   successfully restored. Once the property has been restored, the state machine will clear the 
   initial value until it enters a new state which sets the property and which has RestoreProperties
   as its restore policy.

   \value GlobalRestorePolicy The restore policy for the state should be retrieved using 
          QStateMachine::globalRestorePolicy()
   \value DoNotRestoreProperties The state machine should not save the initial values of properties 
          set in the state and restore them later.
   \value RestoreProperties The state machine should save the initial values of properties 
          set in the state and restore them later.


   \sa setRestorePolicy(), restorePolicy(), QAbstractState::assignProperty()
*/

/*!
   \property QAbstractState::restorePolicy

    \brief the restore policy of this state
*/

QAbstractStatePrivate::QAbstractStatePrivate()
    : restorePolicy(QAbstractState::GlobalRestorePolicy)
{
}

QAbstractStatePrivate *QAbstractStatePrivate::get(QAbstractState *q)
{
    return q->d_func();
}

const QAbstractStatePrivate *QAbstractStatePrivate::get(const QAbstractState *q)
{
    return q->d_func();
}

QStateMachine *QAbstractStatePrivate::machine() const
{
    Q_Q(const QAbstractState);
    QObject *par = q->parent();
    while (par != 0) {
        if (QStateMachine *mach = qobject_cast<QStateMachine*>(par))
            return mach;
        par = par->parent();
    }
    return 0;
}

void QAbstractStatePrivate::callOnEntry()
{
    Q_Q(QAbstractState);
    q->onEntry();
}

void QAbstractStatePrivate::callOnExit()
{
    Q_Q(QAbstractState);
    q->onExit();
}

void QAbstractStatePrivate::emitEntered()
{
    Q_Q(QAbstractState);
    emit q->entered();
}

void QAbstractStatePrivate::emitExited()
{
    Q_Q(QAbstractState);
    emit q->exited();
}

/*!
  Constructs a new state with the given \a parent state.
*/
QAbstractState::QAbstractState(QState *parent)
    : QObject(
#ifndef QT_STATEMACHINE_SOLUTION
        *new QAbstractStatePrivate,
#endif
        parent)
#ifdef QT_STATEMACHINE_SOLUTION
    , d_ptr(new QAbstractStatePrivate)
#endif
{
#ifdef QT_STATEMACHINE_SOLUTION
    d_ptr->q_ptr = this;
#endif
}

/*!
  \internal
*/
QAbstractState::QAbstractState(QAbstractStatePrivate &dd, QState *parent)
    : QObject(
#ifndef QT_STATEMACHINE_SOLUTION
        dd,
#endif
        parent)
#ifdef QT_STATEMACHINE_SOLUTION
    , d_ptr(&dd)
#endif
{
#ifdef QT_STATEMACHINE_SOLUTION
    d_ptr->q_ptr = this;
#endif
}

/*!
  Destroys this state.
*/
QAbstractState::~QAbstractState()
{
#ifdef QT_STATEMACHINE_SOLUTION
    delete d_ptr;
#endif
}

/*!
  Returns this state's parent state, or 0 if the state has no parent state.
*/
QState *QAbstractState::parentState() const
{
    return qobject_cast<QState*>(parent());
}

/*!
  Instructs this state to set the property with the given \a name of the given
  \a object to the given \a value when the state is entered.
*/
void QAbstractState::assignProperty(QObject *object, const char *name,
                                    const QVariant &value)
{
    Q_D(QAbstractState);
    for (int i = 0; i < d->propertyAssignments.size(); ++i) {
        QPropertyAssignment &assn = d->propertyAssignments[i];
        if ((assn.object == object) && (assn.propertyName == name)) {
            assn.value = value;
            return;
        }
    }
    d->propertyAssignments.append(QPropertyAssignment(object, name, value));
}

/*!
  Sets the restore policy of this state to \a restorePolicy. 
  
  The default restore policy is QAbstractState::GlobalRestorePolicy.
*/
void QAbstractState::setRestorePolicy(RestorePolicy restorePolicy)
{
    Q_D(QAbstractState);
    d->restorePolicy = restorePolicy;
}

/*!
  Returns the restore policy for this state.
*/
QAbstractState::RestorePolicy QAbstractState::restorePolicy() const
{
    Q_D(const QAbstractState);
    return d->restorePolicy;
}

/*!
  \fn QAbstractState::onExit()

  This function is called when the state is exited.  Reimplement this function
  to perform custom processing when the state is exited.
*/

/*!
  \fn QAbstractState::onEntry()

  This function is called when the state is entered. Reimplement this function
  to perform custom processing when the state is entered.
*/

/*!
  \fn QAbstractState::entered()

  This signal is emitted when the state has been entered (after onEntry() has
  been called).
*/

/*!
  \fn QAbstractState::exited()

  This signal is emitted when the state has been exited (after onExit() has
  been called).
*/

/*!
  \reimp
*/
bool QAbstractState::event(QEvent *e)
{
    return QObject::event(e);
}

QT_END_NAMESPACE
