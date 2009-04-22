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

#ifndef QFXMOUSEREGION_P_H
#define QFXMOUSEREGION_P_H

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

#include "qdatetime.h"
#include "qbasictimer.h"
#include "qfxitem_p.h"

QT_BEGIN_NAMESPACE

class QFxMouseRegionPrivate : public QFxItemPrivate
{
    Q_DECLARE_PUBLIC(QFxMouseRegion)

public:
    QFxMouseRegionPrivate()
      : absorb(true), hovered(false), inside(true), pressed(false), longPress(0), drag(0)
    {
    }

    void init()
    {
        Q_Q(QFxMouseRegion);
        q->setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
        q->setOptions(QSimpleCanvasItem::HoverEvents | QSimpleCanvasItem::MouseEvents);
    }

    void bindButtonValue(Qt::MouseButton);

    bool absorb;
    bool hovered;
    bool inside;
    bool pressed;
    bool longPress;
    QFxDrag drag;
    bool moved;
    bool dragX;
    bool dragY;
    bool dragged;
    QPointF start;
    QPointF startScene;
    int startX;
    int startY;
    QPointF lastPos;
    QBasicTimer pressAndHoldTimer;
};

QT_END_NAMESPACE

#endif // QFXMOUSEREGION_P_H
