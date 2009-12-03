/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QMLGRAPHICSLAYOUTS_P_H
#define QMLGRAPHICSLAYOUTS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <private/qmlgraphicsitem_p.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <private/qmlgraphicspositioners_p.h>
#include <private/qmlstate_p.h>
#include <private/qmltransitionmanager_p_p.h>
#include <private/qmlstateoperations_p.h>
#include <QtCore/QTimer>

QT_BEGIN_NAMESPACE
class QmlGraphicsBasePositionerPrivate : public QmlGraphicsItemPrivate
{
    Q_DECLARE_PUBLIC(QmlGraphicsBasePositioner)

public:
    QmlGraphicsBasePositionerPrivate()
        : _ep(false), _componentComplete(false), _spacing(0),
        aut(QmlGraphicsBasePositioner::None), moveTransition(0), addTransition(0),
        removeTransition(0), _movingItem(0), queuedPositioning(false)
    {
    }

    ~QmlGraphicsBasePositionerPrivate()
    {
        watched.removeAll(0);
        foreach(QmlGraphicsItem* other, watched)
            unwatchChanges(other);//Need to deregister from a list in QmlGI Private
    }

    void init(QmlGraphicsBasePositioner::AutoUpdateType at)
    {
        aut = at;
        if (prePosIdx == -1) {
            prePosIdx = QmlGraphicsBasePositioner::staticMetaObject.indexOfSlot("prePositioning()");
            visibleIdx = QmlGraphicsItem::staticMetaObject.indexOfSignal("visibleChanged()");
            opacityIdx = QmlGraphicsItem::staticMetaObject.indexOfSignal("opacityChanged()");
        }
    }

    bool _ep;
    bool _componentComplete;
    int _spacing;
    QmlGraphicsBasePositioner::AutoUpdateType aut;
    QmlTransition *moveTransition;
    QmlTransition *addTransition;
    QmlTransition *removeTransition;
    QSet<QmlGraphicsItem *> _items;
    QSet<QmlGraphicsItem *> _leavingItems;
    QSet<QmlGraphicsItem *> _stableItems;
    QSet<QmlGraphicsItem *> _newItems;
    QSet<QmlGraphicsItem *> _animated;
    QmlStateOperation::ActionList addActions;
    QmlStateOperation::ActionList moveActions;
    QmlStateOperation::ActionList removeActions;
    QmlTransitionManager addTransitionManager;
    QmlTransitionManager moveTransitionManager;
    QmlTransitionManager removeTransitionManager;
    QmlGraphicsItem *_movingItem;

    void watchChanges(QmlGraphicsItem *other);
    void unwatchChanges(QmlGraphicsItem* other);
    QList<QGuard<QmlGraphicsItem> > watched;
    bool queuedPositioning;

    static int prePosIdx;
    static int visibleIdx;
    static int opacityIdx;

    virtual void otherSiblingOrderChange(QmlGraphicsItemPrivate* other)
    {
        Q_Q(QmlGraphicsBasePositioner);
        Q_UNUSED(other);
        if(!queuedPositioning){
            //Delay is due to many children often being reordered at once
            QTimer::singleShot(0,q,SLOT(prePositioning()));
            queuedPositioning = true;
        }
    }
};

QT_END_NAMESPACE
#endif
