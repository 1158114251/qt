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

#ifndef QFXPATHVIEW_P_H
#define QFXPATHVIEW_P_H

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
#include "qfxpathview.h"
#include "qfxitem_p.h"
#include "qfxvisualitemmodel.h"
#include "qml.h"
#include "qmltimelinevalueproxy.h"
#include "private/qmlanimation_p.h"

QT_BEGIN_NAMESPACE

typedef struct PathViewItem{
    int index;
    QFxItem* item;
}PathViewItem;

class QFxPathViewPrivate : public QFxItemPrivate
{
    Q_DECLARE_PUBLIC(QFxPathView)

public:
    QFxPathViewPrivate()
      : path(0), currentIndex(0), startPc(0), lastDist(0)
        , lastElapsed(0), stealMouse(false), ownModel(false), activeItem(0)
        , snapPos(0), dragMargin(0), moveOffset(this, &QFxPathViewPrivate::setOffset)
        , firstIndex(0), pathItems(-1), pathOffset(0), model(0)
        , moveReason(Other)
    {
        fixupOffsetEvent = QmlTimeLineEvent::timeLineEvent<QFxPathViewPrivate, &QFxPathViewPrivate::fixOffset>(&moveOffset, this);
    }

    void init()
    {
        Q_Q(QFxPathView);
        _offset = 0;
        q->setAcceptedMouseButtons(Qt::NoButton);
        q->setOptions(QSimpleCanvasItem::MouseFilter | QSimpleCanvasItem::MouseEvents | QSimpleCanvasItem::IsFocusRealm);
        q->connect(&tl, SIGNAL(updated()), q, SLOT(ticked()));
    }

    int calcCurrentIndex();
    void updateCurrent();
    void fixOffset();
    void setOffset(qreal offset);
    void regenerate();
    void updateItem(QFxItem *, qreal);
    void snapToCurrent();
    QPointF pointNear(const QPointF &point, qreal *nearPercent=0) const;

    QFxPath *path;
    int currentIndex;
    qreal startPc;
    QPointF startPoint;
    qreal lastDist;
    int lastElapsed;
    qreal _offset;
    int stealMouse : 1;
    int ownModel : 1;
    QTime lastPosTime;
    QPointF lastPos;
    QFxItem *activeItem;
    qreal snapPos;
    qreal dragMargin;
    QmlTimeLine tl;
    QmlTimeLineValueProxy<QFxPathViewPrivate> moveOffset;
    QmlTimeLineEvent fixupOffsetEvent;
    int firstIndex;
    int pathItems;
    int pathOffset;
    QList<QFxItem *> items;
    QFxVisualItemModel *model;
    QVariant modelVariant;
    enum MovementReason { Other, Key, Mouse };
    MovementReason moveReason;
};

QT_END_NAMESPACE

#endif
