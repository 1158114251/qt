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

#ifndef QGRAPHICSITEM_P_H
#define QGRAPHICSITEM_P_H

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

#include "qgraphicsitem.h"
#include "qpixmapcache.h"

#include <QtCore/qpoint.h>

#if !defined(QT_NO_GRAPHICSVIEW) || (QT_EDITION & QT_MODULE_GRAPHICSVIEW) != QT_MODULE_GRAPHICSVIEW

QT_BEGIN_NAMESPACE

class QGraphicsItemPrivate;

class QGraphicsItemCache
{
public:
    QGraphicsItemCache() : allExposed(false) { }

    // ItemCoordinateCache only
    QRect boundingRect;
    QSize fixedSize;
    QPixmapCache::Key key;

    // DeviceCoordinateCache only
    struct DeviceData {
        DeviceData() {}
        QTransform lastTransform;
        QPoint cacheIndent;
        QPixmapCache::Key key;
    };
    QMap<QPaintDevice *, DeviceData> deviceData;

    // List of logical exposed rects
    QVector<QRectF> exposed;
    bool allExposed;

    // Empty cache
    void purge();
};

class Q_AUTOTEST_EXPORT QGraphicsItemPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsItem)
public:
    enum Extra {
        ExtraToolTip,
        ExtraCursor,
        ExtraCacheData,
        ExtraMaxDeviceCoordCacheSize,
        ExtraBoundingRegionGranularity
    };

    enum AncestorFlag {
        NoFlag = 0,
        AncestorHandlesChildEvents = 0x1,
        AncestorClipsChildren = 0x2,
        AncestorIgnoresTransformations = 0x4
    };

    inline QGraphicsItemPrivate()
        : z(0),
        opacity(1.),
        scene(0),
        parent(0),
        transformData(0),
        index(-1),
        depth(0),
        acceptedMouseButtons(0x1f),
        visible(1),
        explicitlyHidden(0),
        enabled(1),
        explicitlyDisabled(0),
        selected(0),
        acceptsHover(0),
        acceptDrops(0),
        isMemberOfGroup(0),
        handlesChildEvents(0),
        itemDiscovered(0),
        hasCursor(0),
        ancestorFlags(0),
        cacheMode(0),
        hasBoundingRegionGranularity(0),
        isWidget(0),
        dirty(0),
        dirtyChildren(0),
        localCollisionHack(0),
        dirtyClipPath(1),
        emptyClipPath(0),
        inSetPosHelper(0),
        needSortChildren(1),
        allChildrenDirty(0),
        fullUpdatePending(0),
        flags(0),
        dirtyChildrenBoundingRect(1),
        paintedViewBoundingRectsNeedRepaint(0),
        dirtySceneTransform(1),
        geometryChanged(0),
        inDestructor(0),
        globalStackingOrder(-1),
        q_ptr(0)
    {
    }

    inline virtual ~QGraphicsItemPrivate()
    { }

    void updateAncestorFlag(QGraphicsItem::GraphicsItemFlag childFlag,
                            AncestorFlag flag = NoFlag, bool enabled = false, bool root = true);
    void setIsMemberOfGroup(bool enabled);
    void remapItemPos(QEvent *event, QGraphicsItem *item);
    QPointF genericMapFromScene(const QPointF &pos, const QWidget *viewport) const;
    inline bool itemIsUntransformable() const
    {
        return (flags & QGraphicsItem::ItemIgnoresTransformations)
            || (ancestorFlags & AncestorIgnoresTransformations);
    }

    void combineTransformToParent(QTransform *x, const QTransform *viewTransform = 0) const;
    void combineTransformFromParent(QTransform *x, const QTransform *viewTransform = 0) const;
    
    // ### Qt 5: Remove. Workaround for reimplementation added after Qt 4.4.
    virtual QVariant inputMethodQueryHelper(Qt::InputMethodQuery query) const;
    static bool movableAncestorIsSelected(const QGraphicsItem *item);

    void setPosHelper(const QPointF &pos);
    void setTransformHelper(const QTransform &transform);
    void setVisibleHelper(bool newVisible, bool explicitly, bool update = true);
    void setEnabledHelper(bool newEnabled, bool explicitly, bool update = true);
    bool discardUpdateRequest(bool ignoreClipping = false, bool ignoreVisibleBit = false,
                              bool ignoreDirtyBit = false, bool ignoreOpacity = false) const;
    void resolveDepth(int parentDepth);
    void addChild(QGraphicsItem *child);
    void removeChild(QGraphicsItem *child);
    void setParentItemHelper(QGraphicsItem *parent);
    void childrenBoundingRectHelper(QTransform *x, QRectF *rect);
    void initStyleOption(QStyleOptionGraphicsItem *option, const QTransform &worldTransform,
                         const QRegion &exposedRegion, bool allItems = false) const;

    virtual void resolveFont(uint inheritedMask)
    {
        for (int i = 0; i < children.size(); ++i)
            children.at(i)->d_ptr->resolveFont(inheritedMask);
    }

    virtual void resolvePalette(uint inheritedMask)
    {
        for (int i = 0; i < children.size(); ++i)
            children.at(i)->d_ptr->resolveFont(inheritedMask);
    }

    virtual bool isProxyWidget() const;

    inline QVariant extra(Extra type) const
    {
        for (int i = 0; i < extras.size(); ++i) {
            const ExtraStruct &extra = extras.at(i);
            if (extra.type == type)
                return extra.value;
        }
        return QVariant();
    }

    inline void setExtra(Extra type, const QVariant &value)
    {
        int index = -1;
        for (int i = 0; i < extras.size(); ++i) {
            if (extras.at(i).type == type) {
                index = i;
                break;
            }
        }

        if (index == -1) {
            extras << ExtraStruct(type, value);
        } else {
            extras[index].value = value;
        }
    }

    inline void unsetExtra(Extra type)
    {
        for (int i = 0; i < extras.size(); ++i) {
            if (extras.at(i).type == type) {
                extras.removeAt(i);
                return;
            }
        }
    }
    
    struct ExtraStruct {
        ExtraStruct(Extra type, QVariant value)
            : type(type), value(value)
        { }

        Extra type;
        QVariant value;

        bool operator<(Extra extra) const
        { return type < extra; }
    };
    
    QList<ExtraStruct> extras;

    QGraphicsItemCache *maybeExtraItemCache() const;
    QGraphicsItemCache *extraItemCache() const;
    void removeExtraItemCache();

    inline void setCachedClipPath(const QPainterPath &path)
    {
        cachedClipPath = path;
        dirtyClipPath = 0;
        emptyClipPath = 0;
    }

    inline void setEmptyCachedClipPath()
    {
        emptyClipPath = 1;
        dirtyClipPath = 0;
    }

    void setEmptyCachedClipPathRecursively(const QRectF &emptyIfOutsideThisRect = QRectF());

    inline void invalidateCachedClipPath()
    { /*static int count = 0 ;qWarning("%i", ++count);*/ dirtyClipPath = 1; emptyClipPath = 0; }

    void invalidateCachedClipPathRecursively(bool childrenOnly = false, const QRectF &emptyIfOutsideThisRect = QRectF());
    void updateCachedClipPathFromSetPosHelper(const QPointF &newPos);
    void ensureSceneTransformRecursive(QGraphicsItem **topMostDirtyItem);

    inline void invalidateChildrenSceneTransform()
    {
        for (int i = 0; i < children.size(); ++i)
            children.at(i)->d_ptr->dirtySceneTransform = 1;
    }

    inline qreal calcEffectiveOpacity() const
    {
        qreal o = opacity;
        QGraphicsItem *p = parent;
        int myFlags = flags;
        while (p) {
            int parentFlags = p->d_ptr->flags;

            // If I have a parent, and I don't ignore my parent's opacity, and my
            // parent propagates to me, then combine my local opacity with my parent's
            // effective opacity into my effective opacity.
            if ((myFlags & QGraphicsItem::ItemIgnoresParentOpacity)
                || (parentFlags & QGraphicsItem::ItemDoesntPropagateOpacityToChildren)) {
                break;
            }

            o *= p->d_ptr->opacity;
            p = p->d_ptr->parent;
            myFlags = parentFlags;
        }
        return o;
    }

    inline bool isFullyTransparent() const
    {
        if (opacity < 0.001)
            return true;
        if (!parent)
            return false;

        return calcEffectiveOpacity() < 0.001;
    }

    inline qreal effectiveOpacity() const {
        if (!parent || !opacity)
            return opacity;

        return calcEffectiveOpacity();
    }

    inline bool childrenCombineOpacity() const
    {
        if (!children.size())
            return true;
        if (flags & QGraphicsItem::ItemDoesntPropagateOpacityToChildren)
            return false;

        for (int i = 0; i < children.size(); ++i) {
            if (children.at(i)->d_ptr->flags & QGraphicsItem::ItemIgnoresParentOpacity)
                return false;
        }
        return true;
    }

    inline bool isClippedAway() const
    { return !dirtyClipPath && q_func()->isClipped() && (emptyClipPath || cachedClipPath.isEmpty()); }

    inline bool childrenClippedToShape() const
    { return (flags & QGraphicsItem::ItemClipsChildrenToShape) || children.isEmpty(); }

    inline bool isInvisible() const
    {
        return !visible
               || (childrenClippedToShape() && isClippedAway())
               || (childrenCombineOpacity() && isFullyTransparent());
    }

    inline QTransform transformToParent() const;

    QPainterPath cachedClipPath;
    QRectF childrenBoundingRect;
    QRectF needsRepaint;
    QMap<QWidget *, QRect> paintedViewBoundingRects;
    QPointF pos;
    qreal z;
    qreal opacity;
    QGraphicsScene *scene;
    QGraphicsItem *parent;
    QList<QGraphicsItem *> children;
    class TransformData;
    TransformData *transformData;
    QTransform sceneTransform;
    int index;
    int depth;

    // Packed 32 bits
    quint32 acceptedMouseButtons : 5;
    quint32 visible : 1;
    quint32 explicitlyHidden : 1;
    quint32 enabled : 1;
    quint32 explicitlyDisabled : 1;
    quint32 selected : 1;
    quint32 acceptsHover : 1;
    quint32 acceptDrops : 1;
    quint32 isMemberOfGroup : 1;
    quint32 handlesChildEvents : 1;
    quint32 itemDiscovered : 1;
    quint32 hasCursor : 1;
    quint32 ancestorFlags : 3;
    quint32 cacheMode : 2;
    quint32 hasBoundingRegionGranularity : 1;
    quint32 isWidget : 1;
    quint32 dirty : 1;    
    quint32 dirtyChildren : 1;    
    quint32 localCollisionHack : 1;
    quint32 dirtyClipPath : 1;
    quint32 emptyClipPath : 1;
    quint32 inSetPosHelper : 1;
    quint32 needSortChildren : 1;
    quint32 allChildrenDirty : 1;
    quint32 fullUpdatePending : 1;

    // New 32 bits
    quint32 flags : 12;
    quint32 dirtyChildrenBoundingRect : 1;
    quint32 paintedViewBoundingRectsNeedRepaint : 1;
    quint32 dirtySceneTransform  : 1;
    quint32 geometryChanged : 1;
    quint32 inDestructor : 1;
    quint32 unused : 15; // feel free to use

    // Optional stacking order
    int globalStackingOrder;
    QGraphicsItem *q_ptr;
};

struct QGraphicsItemPrivate::TransformData {
    QTransform transform;
    qreal xScale;
    qreal yScale;
    qreal xRotation;
    qreal yRotation;
    qreal zRotation;
    qreal horizontalShear;
    qreal verticalShear;
    qreal xOrigin;
    qreal yOrigin;

    TransformData() :
        xScale(1.0), yScale(1.0), xRotation(0.0), yRotation(0.0), zRotation(0.0),
        horizontalShear(0.0), verticalShear(0.0), xOrigin(0.0), yOrigin(0.0)
    {}

    QTransform computedFullTransform() const
    {
        QTransform x;
        x.translate(xOrigin, yOrigin);
        x = transform * x;
        x.rotate(xRotation, Qt::XAxis);
        x.rotate(yRotation, Qt::YAxis);
        x.rotate(zRotation, Qt::ZAxis);
        x.shear(horizontalShear, verticalShear);
        x.scale(xScale, yScale);
        x.translate(-xOrigin, -yOrigin);
        return x;
    }
};

/*
   return the full transform of the item to the parent.  This include the position and all the transform data
*/
inline QTransform QGraphicsItemPrivate::transformToParent() const
{
    QTransform matrix;
    combineTransformToParent(&matrix);
    return matrix;
}

QT_END_NAMESPACE

#endif // QT_NO_GRAPHICSVIEW

#endif
