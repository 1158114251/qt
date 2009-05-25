/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
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

#include "random_ai_plugin.h"

#include <QState>
#include <QtPlugin>

#include <time.h>

QState *RandomAiPlugin::create(QState *parentState, QObject *tank)
{
    qsrand(uint(time(NULL)));

    QState *topLevel = new QState(parentState);

    QState *selectNextActionState = new SelectActionState(topLevel);
    topLevel->setInitialState(selectNextActionState);

    QState *fireState = new RandomDistanceState(topLevel);
    connect(fireState, SIGNAL(distanceComputed(qreal)), tank, SLOT(fireCannon()));
    selectNextActionState->addTransition(selectNextActionState, SIGNAL(fireSelected()), fireState);

    QState *moveForwardsState = new RandomDistanceState(topLevel);
    connect(moveForwardsState, SIGNAL(distanceComputed(qreal)), tank, SLOT(moveForwards(qreal)));
    selectNextActionState->addTransition(selectNextActionState, SIGNAL(moveForwardsSelected()), moveForwardsState);

    QState *moveBackwardsState = new RandomDistanceState(topLevel);
    connect(moveBackwardsState, SIGNAL(distanceComputed(qreal)), tank, SLOT(moveBackwards(qreal)));
    selectNextActionState->addTransition(selectNextActionState, SIGNAL(moveBackwardsSelected()), moveBackwardsState);

    QState *turnState = new RandomDistanceState(topLevel);
    connect(turnState, SIGNAL(distanceComputed(qreal)), tank, SLOT(turn(qreal)));
    selectNextActionState->addTransition(selectNextActionState, SIGNAL(turnSelected()), turnState);

    topLevel->addTransition(tank, SIGNAL(actionCompleted()), selectNextActionState);

    return topLevel;
}

Q_EXPORT_PLUGIN2(random_ai, RandomAiPlugin)
