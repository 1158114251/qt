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

#include "qgraphicsanchorlayout_p.h"

QGraphicsAnchorLayout::QGraphicsAnchorLayout(QGraphicsLayoutItem *parent)
    : QGraphicsLayout(*new QGraphicsAnchorLayoutPrivate(), parent)
{
    Q_D(QGraphicsAnchorLayout);
    d->createLayoutEdges();
}

QGraphicsAnchorLayout::~QGraphicsAnchorLayout()
{
    Q_D(QGraphicsAnchorLayout);

    for (int i = count() - 1; i >= 0; --i) {
        QGraphicsLayoutItem *item = d->items.at(i);
        removeAt(i);
        if (item) {
            if (item->ownedByLayout())
                delete item;
        }
    }

    d->removeCenterConstraints(this, QGraphicsAnchorLayoutPrivate::Horizontal);
    d->removeCenterConstraints(this, QGraphicsAnchorLayoutPrivate::Vertical);
    d->deleteLayoutEdges();

    Q_ASSERT(d->itemCenterConstraints[0].isEmpty());
    Q_ASSERT(d->itemCenterConstraints[1].isEmpty());
    Q_ASSERT(d->items.isEmpty());
    Q_ASSERT(d->m_vertexList.isEmpty());
}

/*!
 * Creates an anchor between the edge \a firstEdge of item \a firstItem and the edge \a secondEdge
 * of item \a secondItem. The magnitude of the anchor is picked up from the style. Anchors
 * between  a layout edge and an item edge will have a size of 0.
 * If there is already an anchor between the edges, the the new anchor will replace the old one.
 *
 * \a firstItem and \a secondItem are automatically added to the layout if they are not part
 * of the layout. This means that count() can increase with up to 2.
 *
 * The spacing an anchor will get depends on the type of anchor. For instance, anchors from the
 * Right edge of one item to the Left edge of another (or vice versa) will use the default
 * horizontal spacing. The same behaviour applies to Bottom to Top anchors, (but they will use
 * the default vertical spacing). For all other anchor combinations, the spacing will be 0.
 * All anchoring functions will follow this rule.
 *
 * \sa removeAnchor, anchorCorner, anchorWidth, anchorHeight, anchorGeometry
 */
void QGraphicsAnchorLayout::anchor(QGraphicsLayoutItem *firstItem,
                                   Edge firstEdge,
                                   QGraphicsLayoutItem *secondItem,
                                   Edge secondEdge)
{
    Q_D(QGraphicsAnchorLayout);
    d->anchor(firstItem, firstEdge, secondItem, secondEdge);
    invalidate();
}

/*!
 * \overload
 *
 * By calling this function the caller can specify the magnitude of the anchor with \a spacing.
 *
 */
void QGraphicsAnchorLayout::anchor(QGraphicsLayoutItem *firstItem,
                                   Edge firstEdge,
                                   QGraphicsLayoutItem *secondItem,
                                   Edge secondEdge, qreal spacing)
{
    Q_D(QGraphicsAnchorLayout);
    d->anchor(firstItem, firstEdge, secondItem, secondEdge, &spacing);
    invalidate();
}

/*!
 * Creates two anchors between \a firstItem and \a secondItem, where one is for the horizontal
 * edge and another one for the vertical edge that the corners \a firstCorner and \a
 * secondCorner specifies.
 * The magnitude of the anchors is picked up from the style.
 *
 * This is a convenience function, since anchoring corners can be expressed as anchoring two edges.
 * For instance,
 * \code
 *  layout->anchor(layout, QGraphicsAnchorLayout::Top, b, QGraphicsAnchorLayout::Top);
 *  layout->anchor(layout, QGraphicsAnchorLayout::Left, b, QGraphicsAnchorLayout::Left);
 * \endcode
 *
 * has the same effect as
 *
 * \code
 * layout->anchor(layout, Qt::TopLeft, b, Qt::TopLeft);
 * \endcode
 *
 * If there is already an anchor between the edge pairs, it will be replaced by the anchors that
 * this function specifies.
 *
 * \a firstItem and \a secondItem are automatically added to the layout if they are not part
 * of the layout. This means that count() can increase with up to 2.
 */
void QGraphicsAnchorLayout::anchorCorner(QGraphicsLayoutItem *firstItem,
                                         Qt::Corner firstCorner,
                                         QGraphicsLayoutItem *secondItem,
                                         Qt::Corner secondCorner)
{
    Q_D(QGraphicsAnchorLayout);

    // Horizontal anchor
    QGraphicsAnchorLayout::Edge firstEdge = (firstCorner & 1 ? QGraphicsAnchorLayout::Right: QGraphicsAnchorLayout::Left);
    QGraphicsAnchorLayout::Edge secondEdge = (secondCorner & 1 ? QGraphicsAnchorLayout::Right: QGraphicsAnchorLayout::Left);
    d->anchor(firstItem, firstEdge, secondItem, secondEdge);

    // Vertical anchor
    firstEdge = (firstCorner & 2 ? QGraphicsAnchorLayout::Bottom: QGraphicsAnchorLayout::Top);
    secondEdge = (secondCorner & 2 ? QGraphicsAnchorLayout::Bottom: QGraphicsAnchorLayout::Top);
    d->anchor(firstItem, firstEdge, secondItem, secondEdge);

    invalidate();
}

/*!
 * \overload
 *
 * By calling this function the caller can specify the magnitude of the anchor with \a spacing.
 *
 */
void QGraphicsAnchorLayout::anchorCorner(QGraphicsLayoutItem *firstItem,
                                         Qt::Corner firstCorner,
                                         QGraphicsLayoutItem *secondItem,
                                         Qt::Corner secondCorner, qreal spacing)
{
    Q_D(QGraphicsAnchorLayout);

    // Horizontal anchor
    QGraphicsAnchorLayout::Edge firstEdge = (firstCorner & 1 ? QGraphicsAnchorLayout::Right: QGraphicsAnchorLayout::Left);
    QGraphicsAnchorLayout::Edge secondEdge = (secondCorner & 1 ? QGraphicsAnchorLayout::Right: QGraphicsAnchorLayout::Left);
    d->anchor(firstItem, firstEdge, secondItem, secondEdge, &spacing);

    // Vertical anchor
    firstEdge = (firstCorner & 2 ? QGraphicsAnchorLayout::Bottom: QGraphicsAnchorLayout::Top);
    secondEdge = (secondCorner & 2 ? QGraphicsAnchorLayout::Bottom: QGraphicsAnchorLayout::Top);
    d->anchor(firstItem, firstEdge, secondItem, secondEdge, &spacing);

    invalidate();
}

void QGraphicsAnchorLayout::removeAnchor(QGraphicsLayoutItem *firstItem, Edge firstEdge,
                                         QGraphicsLayoutItem *secondItem, Edge secondEdge)
{
    Q_D(QGraphicsAnchorLayout);
    if ((firstItem == 0) || (secondItem == 0)) {
        qWarning("QGraphicsAnchorLayout::removeAnchor: "
                 "Cannot remove anchor between NULL items");
        return;
    }

    if (firstItem == secondItem) {
        qWarning("QGraphicsAnchorLayout::removeAnchor: "
                 "Cannot remove anchor from the item to itself");
        return;
    }

    if (d->edgeOrientation(secondEdge) != d->edgeOrientation(firstEdge)) {
        qWarning("QGraphicsAnchorLayout::removeAnchor: "
                 "Cannot remove anchor from edges of different orientations");
        return;
    }

    d->removeAnchor(firstItem, firstEdge, secondItem, secondEdge);
    invalidate();
}

/*!
    Sets the default horizontal spacing for the anchor layout to \a spacing.

    \sa horizontalSpacing, setVerticalSpacing, setSpacing
*/
void QGraphicsAnchorLayout::setHorizontalSpacing(qreal spacing)
{
    Q_D(QGraphicsAnchorLayout);
    d->spacings[0] = spacing;
    invalidate();
}

/*!
    Sets the default vertical spacing for the anchor layout to \a spacing.

    \sa verticalSpacing, setHorizontalSpacing, setSpacing
*/
void QGraphicsAnchorLayout::setVerticalSpacing(qreal spacing)
{
    Q_D(QGraphicsAnchorLayout);
    d->spacings[1] = spacing;
    invalidate();
}

/*!
    Sets the default horizontal and the default vertical spacing for the anchor layout to \a spacing.

    If an item is anchored with no spacing associated with the anchor, it will use the default
    spacing.
    \sa setHorizontalSpacing, setVerticalSpacing
*/
void QGraphicsAnchorLayout::setSpacing(qreal spacing)
{
    Q_D(QGraphicsAnchorLayout);
    d->spacings[0] = d->spacings[1] = spacing;
    invalidate();
}

/*!
    Returns the default horizontal spacing for the anchor layout.

    \sa verticalSpacing, setHorizontalSpacing
*/
qreal QGraphicsAnchorLayout::horizontalSpacing() const
{
    Q_D(const QGraphicsAnchorLayout);
    return d->effectiveSpacing(QGraphicsAnchorLayoutPrivate::Horizontal);
}

/*!
    Returns the default vertical spacing for the anchor layout.

    \sa horizontalSpacing, setVerticalSpacing
*/
qreal QGraphicsAnchorLayout::verticalSpacing() const
{
    Q_D(const QGraphicsAnchorLayout);
    return d->effectiveSpacing(QGraphicsAnchorLayoutPrivate::Vertical);
}

void QGraphicsAnchorLayout::setGeometry(const QRectF &geom)
{
    Q_D(QGraphicsAnchorLayout);

    // ### REMOVE IT WHEN calculateVertexPositions and setItemsGeometries are
    // simplification compatible!
    d->restoreSimplifiedGraph(QGraphicsAnchorLayoutPrivate::Horizontal);
    d->restoreSimplifiedGraph(QGraphicsAnchorLayoutPrivate::Vertical);

    QGraphicsLayout::setGeometry(geom);
    d->calculateVertexPositions(QGraphicsAnchorLayoutPrivate::Horizontal);
    d->calculateVertexPositions(QGraphicsAnchorLayoutPrivate::Vertical);
    d->setItemsGeometries();
}

void QGraphicsAnchorLayout::removeAt(int index)
{
    Q_D(QGraphicsAnchorLayout);
    QGraphicsLayoutItem *item = d->items.value(index);

    if (!item)
        return;

    // Removing an item affects both horizontal and vertical graphs
    d->restoreSimplifiedGraph(QGraphicsAnchorLayoutPrivate::Horizontal);
    d->restoreSimplifiedGraph(QGraphicsAnchorLayoutPrivate::Vertical);

    d->removeCenterConstraints(item, QGraphicsAnchorLayoutPrivate::Horizontal);
    d->removeCenterConstraints(item, QGraphicsAnchorLayoutPrivate::Vertical);
    d->removeAnchors(item);
    d->items.remove(index);

    item->setParentLayoutItem(0);
    invalidate();
}

int QGraphicsAnchorLayout::count() const
{
    Q_D(const QGraphicsAnchorLayout);
    return d->items.size();
}

QGraphicsLayoutItem *QGraphicsAnchorLayout::itemAt(int index) const
{
    Q_D(const QGraphicsAnchorLayout);
    return d->items.value(index);
}

void QGraphicsAnchorLayout::invalidate()
{
    Q_D(QGraphicsAnchorLayout);
    QGraphicsLayout::invalidate();
    d->calculateGraphCacheDirty = 1;
}

QSizeF QGraphicsAnchorLayout::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_UNUSED(which);
    Q_UNUSED(constraint);
    Q_D(const QGraphicsAnchorLayout);

    // Some setup calculations are delayed until the information is
    // actually needed, avoiding unnecessary recalculations when
    // adding multiple anchors.

    // sizeHint() / effectiveSizeHint() already have a cache
    // mechanism, using invalidate() to force recalculation. However
    // sizeHint() is called three times after invalidation (for max,
    // min and pref), but we just need do our setup once.

    const_cast<QGraphicsAnchorLayoutPrivate *>(d)->calculateGraphs();

    // ### apply constraint!
    QSizeF engineSizeHint(
        d->sizeHints[QGraphicsAnchorLayoutPrivate::Horizontal][which],
        d->sizeHints[QGraphicsAnchorLayoutPrivate::Vertical][which]);

    qreal left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);

    return engineSizeHint + QSizeF(left + right, top + bottom);
}
