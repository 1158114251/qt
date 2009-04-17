/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
****************************************************************************/

#include "qbasicmouseeventtransition_p.h"
#include <QtGui/qevent.h>
#include <QtGui/qpainterpath.h>
#include <qdebug.h>

#if defined(QT_EXPERIMENTAL_SOLUTION)
# include "qabstracttransition_p.h"
#else
# include <private/qabstracttransition_p.h>
#endif

QT_BEGIN_NAMESPACE

/*!
  \internal
  \class QBasicMouseEventTransition

  \brief The QBasicMouseEventTransition class provides a transition for Qt mouse events.
*/

class QBasicMouseEventTransitionPrivate : public QAbstractTransitionPrivate
{
    Q_DECLARE_PUBLIC(QBasicMouseEventTransition)
public:
    QBasicMouseEventTransitionPrivate();

    static QBasicMouseEventTransitionPrivate *get(QBasicMouseEventTransition *q);

    QEvent::Type eventType;
    Qt::MouseButton button;
    QPainterPath path;
};

QBasicMouseEventTransitionPrivate::QBasicMouseEventTransitionPrivate()
{
    eventType = QEvent::None;
    button = Qt::NoButton;
}

QBasicMouseEventTransitionPrivate *QBasicMouseEventTransitionPrivate::get(QBasicMouseEventTransition *q)
{
    return q->d_func();
}

/*!
  Constructs a new mouse event transition with the given \a sourceState.
*/
QBasicMouseEventTransition::QBasicMouseEventTransition(QState *sourceState)
    : QAbstractTransition(*new QBasicMouseEventTransitionPrivate, sourceState)
{
}

/*!
  Constructs a new mouse event transition for events of the given \a type.
*/
QBasicMouseEventTransition::QBasicMouseEventTransition(QEvent::Type type,
                                                       Qt::MouseButton button,
                                                       QState *sourceState)
    : QAbstractTransition(*new QBasicMouseEventTransitionPrivate, sourceState)
{
    Q_D(QBasicMouseEventTransition);
    d->eventType = type;
    d->button = button;
}

/*!
  Destroys this mouse event transition.
*/
QBasicMouseEventTransition::~QBasicMouseEventTransition()
{
}

/*!
  Returns the event type that this mouse event transition is associated with.
*/
QEvent::Type QBasicMouseEventTransition::eventType() const
{
    Q_D(const QBasicMouseEventTransition);
    return d->eventType;
}

/*!
  Sets the event \a type that this mouse event transition is associated with.
*/
void QBasicMouseEventTransition::setEventType(QEvent::Type type)
{
    Q_D(QBasicMouseEventTransition);
    d->eventType = type;
}

/*!
  Returns the button that this mouse event transition checks for.
*/
Qt::MouseButton QBasicMouseEventTransition::button() const
{
    Q_D(const QBasicMouseEventTransition);
    return d->button;
}

/*!
  Sets the button that this mouse event transition will check for.
*/
void QBasicMouseEventTransition::setButton(Qt::MouseButton button)
{
    Q_D(QBasicMouseEventTransition);
    d->button = button;
}

/*!
  Returns the path for this mouse event transition.
*/
QPainterPath QBasicMouseEventTransition::path() const
{
    Q_D(const QBasicMouseEventTransition);
    return d->path;
}

/*!
  Sets the path for this mouse event transition.
*/
void QBasicMouseEventTransition::setPath(const QPainterPath &path)
{
    Q_D(QBasicMouseEventTransition);
    d->path = path;
}

/*!
  \reimp
*/
bool QBasicMouseEventTransition::eventTest(QEvent *event) const
{
    Q_D(const QBasicMouseEventTransition);
    if (event->type() == d->eventType) {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        return (me->button() == d->button)
            && (d->path.isEmpty() || d->path.contains(me->pos()));
    }
    return false;
}

/*!
  \reimp
*/
void QBasicMouseEventTransition::onTransition()
{
}

QT_END_NAMESPACE
