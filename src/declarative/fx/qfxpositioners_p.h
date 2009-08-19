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

#ifndef QFXLAYOUTS_P_H
#define QFXLAYOUTS_P_H

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

#include <private/qfxitem_p.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtDeclarative/qfxpositioners.h>
#include <QtDeclarative/qmlstate.h>
#include <private/qmltransitionmanager_p.h>
#include <QtDeclarative/qmlstateoperations.h>

QT_BEGIN_NAMESPACE
class QFxBasePositionerPrivate : public QFxItemPrivate
{
    Q_DECLARE_PUBLIC(QFxBasePositioner)

public:
    QFxBasePositionerPrivate()
        : _ep(false), _componentComplete(false), _spacing(0),
        aut(QFxBasePositioner::None), moveTransition(0), addTransition(0),
        removeTransition(0), _movingItem(0)
    {
    }

    void init(QFxBasePositioner::AutoUpdateType at)
    {
        aut = at;
    }

    bool _ep;
    bool _componentComplete;
    int _spacing;
    QFxBasePositioner::AutoUpdateType aut;
    QmlTransition *moveTransition;
    QmlTransition *addTransition;
    QmlTransition *removeTransition;
    QSet<QFxItem *> _items;
    QSet<QFxItem *> _leavingItems;
    QSet<QFxItem *> _stableItems;
    QSet<QFxItem *> _newItems;
    QSet<QFxItem *> _animated;
    QmlStateOperation::ActionList addActions;
    QmlStateOperation::ActionList moveActions;
    QmlStateOperation::ActionList removeActions;
    QmlTransitionManager addTransitionManager;
    QmlTransitionManager moveTransitionManager;
    QmlTransitionManager removeTransitionManager;
//    QmlStateGroup *stateGroup;
    QFxItem *_movingItem;
};

QT_END_NAMESPACE
#endif
