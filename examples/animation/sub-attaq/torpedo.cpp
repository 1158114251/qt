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

//Own
#include "torpedo.h"
#include "pixmapitem.h"
#include "boat.h"
#include "graphicsscene.h"
#include "animationmanager.h"

#if defined(QT_EXPERIMENTAL_SOLUTION)
#include "qpropertyanimation.h"
#include "qstatemachine.h"
#include "qfinalstate.h"
#include "qanimationstate.h"
#else
#include <QPropertyAnimation>
#include <QAnimationState>
#include <QStateMachine>
#include <QFinalState>
#endif

Torpedo::Torpedo(QGraphicsItem * parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent,wFlags), currentSpeed(0), launchAnimation(0)
{
    pixmapItem = new PixmapItem(QString::fromLatin1("torpedo"),GraphicsScene::Big, this);
    setZValue(2);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setFlags(QGraphicsItem::ItemIsMovable);
    resize(pixmapItem->boundingRect().size());
}

void Torpedo::launch()
{
    launchAnimation = new QPropertyAnimation(this, "pos");
    AnimationManager::self()->registerAnimation(launchAnimation);
    launchAnimation->setEndValue(QPointF(x(),qobject_cast<GraphicsScene *>(scene())->sealLevel() - 15));
    launchAnimation->setEasingCurve(QEasingCurve::InQuad);
    launchAnimation->setDuration(y()/currentSpeed*10);
    connect(launchAnimation,SIGNAL(valueChanged(const QVariant &)),this,SLOT(onAnimationLaunchValueChanged(const QVariant &)));

    //We setup the state machine of the torpedo
    QStateMachine *machine = new QStateMachine(this);

    //This state is when the launch animation is playing
    QAnimationState *launched = new QAnimationState(launchAnimation,machine->rootState());

    machine->setInitialState(launched);

    //End
    QFinalState *final = new QFinalState(machine->rootState());

    //### Add a nice animation when the torpedo is destroyed
    launched->addTransition(this, SIGNAL(torpedoExplosed()),final);

    //If the animation is finished, then we move to the final state
    launched->addFinishedTransition(final);

    //The machine has finished to be executed, then the boat is dead
    connect(machine,SIGNAL(finished()),this, SIGNAL(torpedoExecutionFinished()));

    machine->start();
}

void Torpedo::setCurrentSpeed(int speed)
{
    if (speed < 0) {
        qWarning("Torpedo::setCurrentSpeed : The speed is invalid");
        return;
    }
    currentSpeed = speed;
}

void Torpedo::onAnimationLaunchValueChanged(const QVariant &)
{
    foreach (QGraphicsItem *item , collidingItems(Qt::IntersectsItemBoundingRect)) {
        if (item->type() == Boat::Type) {
            Boat *b = static_cast<Boat *>(item);
            b->destroy();
        }
    }
}

void Torpedo::destroy()
{
    launchAnimation->stop();
    emit torpedoExplosed();
}
