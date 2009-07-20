/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
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

#include <QtDeclarative/qmlbindablevalue.h>
#include <private/qmltransitionmanager_p.h>
#include <private/qmlstate_p.h>

QT_BEGIN_NAMESPACE

class QmlTransitionManagerPrivate
{
public:
    QmlTransitionManagerPrivate()
    : state(0), transition(0) {}

    void applyBindings();
    typedef QList<SimpleAction> SimpleActionList;
    QmlState *state;
    QmlTransition *transition;
    QmlStateOperation::ActionList bindingsList;
    SimpleActionList completeList;
};

QmlTransitionManager::QmlTransitionManager()
: d(new QmlTransitionManagerPrivate)
{
}

void QmlTransitionManager::setState(QmlState *s)
{
    d->state = s;
}

QmlTransitionManager::~QmlTransitionManager()
{
    delete d; d = 0;
}

void QmlTransitionManager::complete() 
{
    d->applyBindings();

    for (int ii = 0; ii < d->completeList.count(); ++ii) {
        const QmlMetaProperty &prop = d->completeList.at(ii).property;
        prop.write(d->completeList.at(ii).value);
    }

    d->completeList.clear();

    if (d->state) 
        static_cast<QmlStatePrivate*>(QObjectPrivate::get(d->state))->complete();
}

void QmlTransitionManagerPrivate::applyBindings()
{
    foreach(const Action &action, bindingsList) {
        if (action.toBinding) {
            action.property.setBinding(action.toBinding);
            action.toBinding->forceUpdate();
        }
    }

    bindingsList.clear();
}

void QmlTransitionManager::transition(const QList<Action> &list,
                                      QmlTransition *transition)
{
    cancel();

    QmlStateOperation::ActionList applyList = list;
    // Determine which actions are binding changes.
    foreach(const Action &action, applyList) {
        if (action.toBinding)
            d->bindingsList << action;
        if (action.fromBinding)
            action.property.setBinding(0); // Disable current binding
    }

    // Animated transitions need both the start and the end value for
    // each property change.  In the presence of bindings, the end values
    // are non-trivial to calculate.  As a "best effort" attempt, we first
    // apply all the property and binding changes, then read all the actual
    // final values, then roll back the changes and proceed as normal.
    //
    // This doesn't catch everything, and it might be a little fragile in
    // some cases - but whatcha going to do?

    if (!d->bindingsList.isEmpty()) {

        // Apply all the property and binding changes
        foreach(const Action &action, applyList) {
            if (action.toBinding) {
                action.property.setBinding(action.toBinding);
                action.toBinding->forceUpdate();
            } else if (!action.event) {
                action.property.write(action.toValue);
            }
        }

        // Read all the end values for binding changes
        for (int ii = 0; ii < applyList.size(); ++ii) {
            Action *action = &applyList[ii];
            if (action->event)
                continue;

            const QmlMetaProperty &prop = action->property;
            if (action->toBinding) 
                action->toValue = prop.read();
        }

        // Revert back to the original values
        foreach(const Action &action, applyList) {
            if (action.event)
                continue;

            if (action.toBinding)
                action.property.setBinding(0); // Make sure this is disabled during the transition

            action.property.write(action.fromValue);
        }
    }

    if (transition) {
        QList<QmlMetaProperty> touched;
        d->transition = transition;
        d->transition->prepare(applyList, touched, this);

        // Modify the action list to remove actions handled in the transition
        for (int ii = 0; ii < applyList.count(); ++ii) {
            const Action &action = applyList.at(ii);

            if (action.event) {

                if (action.actionDone) {
                    applyList.removeAt(ii);
                    --ii;
                }

            } else {

                if (touched.contains(action.property)) {
                    if (action.toValue != action.fromValue) 
                        d->completeList << 
                            SimpleAction(action, SimpleAction::EndState);

                    applyList.removeAt(ii);
                    --ii;
                }

            }
        }
    }

    // Any actions remaining have not been handled by the transition and should
    // be applied immediately.  We skip applying transitions, as they are all
    // applied at the end in applyBindings() to avoid any nastiness mid 
    // transition
    foreach(const Action &action, applyList) {
        if (action.event) {
            action.event->execute();
        } else {
            action.property.write(action.toValue);
        }
    }
    if (!transition)
        d->applyBindings(); //### merge into above foreach?

}

void QmlTransitionManager::cancel()
{
    if (d->transition) {
        // ### this could potentially trigger a complete in rare circumstances
        d->transition->stop();  
        d->transition = 0;
    }

    d->bindingsList.clear();
    d->completeList.clear();

}

QT_END_NAMESPACE
