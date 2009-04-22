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

#ifndef QKEYEVENTTRANSITION_H
#define QKEYEVENTTRANSITION_H

#if defined(QT_EXPERIMENTAL_SOLUTION)
# include "qeventtransition.h"
#else
# include <QtCore/qeventtransition.h>
#endif
QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Core)

class QKeyEventTransitionPrivate;
class Q_GUI_EXPORT QKeyEventTransition : public QEventTransition
{
    Q_OBJECT
    Q_PROPERTY(int key READ key WRITE setKey)
public:
    QKeyEventTransition(QState *sourceState = 0);
    QKeyEventTransition(QObject *object, QEvent::Type type, int key,
                        QState *sourceState = 0);
    QKeyEventTransition(QObject *object, QEvent::Type type, int key,
                        const QList<QAbstractState*> &targets,
                        QState *sourceState = 0);
    ~QKeyEventTransition();

    int key() const;
    void setKey(int key);

    Qt::KeyboardModifiers modifiers() const;
    void setModifiers(Qt::KeyboardModifiers modifiers);

protected:
    void onTransition();
    bool eventTest(QEvent *event) const;
    bool testEventCondition(QEvent *event) const;

private:
    Q_DISABLE_COPY(QKeyEventTransition)
    Q_DECLARE_PRIVATE(QKeyEventTransition)
};

QT_END_NAMESPACE

QT_END_HEADER

#endif
