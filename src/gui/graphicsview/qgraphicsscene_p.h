/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QGRAPHICSSCENE_P_H
#define QGRAPHICSSCENE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qapplication_*.cpp, qwidget*.cpp and qfiledialog.cpp.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#include "qgraphicsscene.h"

#if !defined(QT_NO_GRAPHICSVIEW) || (QT_EDITION & QT_MODULE_GRAPHICSVIEW) != QT_MODULE_GRAPHICSVIEW

#include "qgraphicsscenebsptreeindex_p.h"
#include "qgraphicsscenelinearindex_p.h"
#include "qgraphicssceneindex.h"
#include "qgraphicsview.h"
#include "qgraphicsitem_p.h"

#include <private/qobject_p.h>
#include <QtCore/qbitarray.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qset.h>
#include <QtGui/qfont.h>
#include <QtGui/qpalette.h>
#include <QtGui/qstyle.h>
#include <QtGui/qstyleoption.h>

QT_BEGIN_NAMESPACE

class QGraphicsView;
class QGraphicsWidget;

class QGraphicsScenePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsScene)
public:
    QGraphicsScenePrivate();
    void init();

    static QGraphicsScenePrivate *get(QGraphicsScene *q);

    quint32 changedSignalMask;

    QGraphicsScene::ItemIndexMethod indexMethod;
    int bspTreeDepth;

    int lastItemCount;

    QGraphicsSceneIndex *index;

    QRectF sceneRect;
    bool hasSceneRect;
    QRectF growingItemsBoundingRect;
    QRectF largestUntransformableItem;

    void _q_emitUpdated();
    QList<QRectF> updatedRects;
    bool updateAll;
    bool calledEmitUpdated;
    bool processDirtyItemsEmitted;

    QPainterPath selectionArea;
    int selectionChanging;
    QSet<QGraphicsItem *> selectedItems;
    QList<QGraphicsItem *> pendingUpdateItems;
    QList<QGraphicsItem *> unpolishedItems;
    QList<QGraphicsItem *> topLevelItems;
    bool needSortTopLevelItems;
    QMap<QGraphicsItem *, QPointF> movingItemsInitialPositions;
    void registerTopLevelItem(QGraphicsItem *item);
    void unregisterTopLevelItem(QGraphicsItem *item);
    void _q_updateLater();
    void _q_polishItems();

    void _q_processDirtyItems();

    void removeItemHelper(QGraphicsItem *item);

    QBrush backgroundBrush;
    QBrush foregroundBrush;

    bool stickyFocus;
    bool hasFocus;
    QGraphicsItem *focusItem;
    QGraphicsItem *lastFocusItem;
    QGraphicsWidget *tabFocusFirst;
    QGraphicsWidget *activeWindow;
    int activationRefCount;

    QList<QGraphicsWidget *> popupWidgets;
    void addPopup(QGraphicsWidget *widget);
    void removePopup(QGraphicsWidget *widget, bool itemIsDying = false);

    QGraphicsItem *lastMouseGrabberItem;
    bool lastMouseGrabberItemHasImplicitMouseGrab;
    QList<QGraphicsItem *> mouseGrabberItems;
    void grabMouse(QGraphicsItem *item, bool implicit = false);
    void ungrabMouse(QGraphicsItem *item, bool itemIsDying = false);
    void clearMouseGrabber();

    QList<QGraphicsItem *> keyboardGrabberItems;
    void grabKeyboard(QGraphicsItem *item);
    void ungrabKeyboard(QGraphicsItem *item, bool itemIsDying = false);
    void clearKeyboardGrabber();
    
    QGraphicsItem *dragDropItem;
    QGraphicsWidget *enterWidget;
    Qt::DropAction lastDropAction;
    QList<QGraphicsItem *> cachedItemsUnderMouse;
    QList<QGraphicsItem *> hoverItems;
    bool allItemsIgnoreHoverEvents;
    bool allItemsUseDefaultCursor;
    void enableMouseTrackingOnViews();
    QMap<Qt::MouseButton, QPointF> mouseGrabberButtonDownPos;
    QMap<Qt::MouseButton, QPointF> mouseGrabberButtonDownScenePos;
    QMap<Qt::MouseButton, QPoint> mouseGrabberButtonDownScreenPos;
    QList<QGraphicsItem *> itemsAtPosition(const QPoint &screenPos,
                                           const QPointF &scenePos,
                                           QWidget *widget) const;
    static bool itemCollidesWithPath(QGraphicsItem *item, const QPainterPath &path, Qt::ItemSelectionMode mode);
    void storeMouseButtonsForMouseGrabber(QGraphicsSceneMouseEvent *event);

    QList<QGraphicsView *> views;
    bool painterStateProtection;

    QMultiMap<QGraphicsItem *, QGraphicsItem *> sceneEventFilters;
    void installSceneEventFilter(QGraphicsItem *watched, QGraphicsItem *filter);
    void removeSceneEventFilter(QGraphicsItem *watched, QGraphicsItem *filter);
    bool filterEvent(QGraphicsItem *item, QEvent *event);
    bool sendEvent(QGraphicsItem *item, QEvent *event);

    bool dispatchHoverEvent(QGraphicsSceneHoverEvent *hoverEvent);
    bool itemAcceptsHoverEvents_helper(const QGraphicsItem *item) const;
    void leaveScene();

    void cloneDragDropEvent(QGraphicsSceneDragDropEvent *dest,
                           QGraphicsSceneDragDropEvent *source);
    void sendDragDropEvent(QGraphicsItem *item,
                           QGraphicsSceneDragDropEvent *dragDropEvent);
    void sendHoverEvent(QEvent::Type type, QGraphicsItem *item,
                        QGraphicsSceneHoverEvent *hoverEvent);
    void sendMouseEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mousePressEventHandler(QGraphicsSceneMouseEvent *mouseEvent);
    QGraphicsWidget *windowForItem(const QGraphicsItem *item) const;

    void recursive_items_helper(QGraphicsItem *item, QRectF rect, QList<QGraphicsItem *> *items,
                                const QTransform &parentTransform, const QTransform &viewTransform,
                                Qt::ItemSelectionMode mode, Qt::SortOrder order, qreal parentOpacity = 1.0) const;

    void drawItemHelper(QGraphicsItem *item, QPainter *painter,
                        const QStyleOptionGraphicsItem *option, QWidget *widget,
                        bool painterStateProtection);
    
    void drawSubtreeRecursive(QGraphicsItem *item, QPainter *painter, const QTransform &viewTransform,
                              QRegion *exposedRegion, QWidget *widget,
                              QList<QGraphicsItem *> *topLevelItems = 0, qreal parentOpacity = qreal(1.0));
    void markDirty(QGraphicsItem *item, const QRectF &rect = QRectF(), bool invalidateChildren = false,
                   bool maybeDirtyClipPath = false, bool force = false, bool ignoreOpacity = false,
                   bool removingItemFromScene = false);
    void processDirtyItemsRecursive(QGraphicsItem *item, bool dirtyAncestorContainsChildren = false);

    inline void resetDirtyItem(QGraphicsItem *item)
    {
        Q_ASSERT(item);
        item->d_ptr->dirty = 0;
        item->d_ptr->paintedViewBoundingRectsNeedRepaint = 0;
        item->d_ptr->geometryChanged = 0;
        item->d_ptr->dirtyChildren = 0;
        item->d_ptr->needsRepaint = QRectF();
        item->d_ptr->allChildrenDirty = 0;
        item->d_ptr->fullUpdatePending = 0;
    }

    QStyle *style;
    QFont font;
    void setFont_helper(const QFont &font);
    void resolveFont();
    void updateFont(const QFont &font);
    QPalette palette;
    void setPalette_helper(const QPalette &palette);
    void resolvePalette();
    void updatePalette(const QPalette &palette);

    QStyleOptionGraphicsItem styleOptionTmp;
};

static inline bool QRectF_intersects(const QRectF &s, const QRectF &r)
{
    qreal xp = s.left();
    qreal yp = s.top();
    qreal w = s.width();
    qreal h = s.height();
    qreal l1 = xp;
    qreal r1 = xp;
    if (w < 0)
        l1 += w;
    else
        r1 += w;

    qreal l2 = r.left();
    qreal r2 = r.left();
    if (w < 0)
        l2 += r.width();
    else
        r2 += r.width();

    if (l1 >= r2 || l2 >= r1)
        return false;

    qreal t1 = yp;
    qreal b1 = yp;
    if (h < 0)
        t1 += h;
    else
        b1 += h;

    qreal t2 = r.top();
    qreal b2 = r.top();
    if (r.height() < 0)
        t2 += r.height();
    else
        b2 += r.height();

    return !(t1 >= b2 || t2 >= b1);
}

// QRectF::intersects() returns false always if either the source or target
// rectangle's width or height are 0. This works around that problem.
static inline void _q_adjustRect(QRectF *rect)
{
    Q_ASSERT(rect);
    if (!rect->width())
        rect->adjust(-0.00001, 0, 0.00001, 0);
    if (!rect->height())
        rect->adjust(0, -0.00001, 0, 0.00001);
}

static inline QRectF adjustedItemBoundingRect(const QGraphicsItem *item)
{
    Q_ASSERT(item);
    QRectF boundingRect(item->boundingRect());
    _q_adjustRect(&boundingRect);
    return boundingRect;
}

QT_END_NAMESPACE

#endif // QT_NO_GRAPHICSVIEW

#endif
