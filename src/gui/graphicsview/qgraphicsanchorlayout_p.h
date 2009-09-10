/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QGRAPHICSANCHORLAYOUT_P_H
#define QGRAPHICSANCHORLAYOUT_P_H

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

#include <QGraphicsWidget>

#include "qgraphicslayout_p.h"
#include "qgraphicsanchorlayout.h"
#include "qgraph_p.h"
#include "qsimplex_p.h"

QT_BEGIN_NAMESPACE

/*
  The public QGraphicsAnchorLayout interface represents an anchorage point
  as a pair of a <QGraphicsLayoutItem *> and a <Qt::AnchorPoint>.

  Internally though, it has a graph of anchorage points (vertices) and
  anchors (edges), represented by the AnchorVertex and AnchorData structs
  respectively.
*/

/*!
  \internal

  Represents a vertex (anchorage point) in the internal graph
*/
struct AnchorVertex {
    AnchorVertex(QGraphicsLayoutItem *item, Qt::AnchorPoint edge)
        : m_item(item), m_edge(edge) {}

    AnchorVertex()
        : m_item(0), m_edge(Qt::AnchorPoint(0)) {}

#ifdef QT_DEBUG
    inline QString toString() const;
#endif
    QGraphicsLayoutItem *m_item;
    Qt::AnchorPoint m_edge;

    // Current distance from this vertex to the layout edge (Left or Top)
    // Value is calculated from the current anchors sizes.
    qreal distance;
};

#ifdef QT_DEBUG
inline QString AnchorVertex::toString() const
{
    if (!this || !m_item) {
        return QLatin1String("NULL");
    }
    QString edge;
    switch (m_edge) {
    case Qt::AnchorLeft:
        edge = QLatin1String("Left");
        break;
    case Qt::AnchorHorizontalCenter:
        edge = QLatin1String("HorizontalCenter");
        break;
    case Qt::AnchorRight:
        edge = QLatin1String("Right");
        break;
    case Qt::AnchorTop:
        edge = QLatin1String("Top");
        break;
    case Qt::AnchorVerticalCenter:
        edge = QLatin1String("VerticalCenter");
        break;
    case Qt::AnchorBottom:
        edge = QLatin1String("Bottom");
        break;
    default:
        edge = QLatin1String("None");
        break;
    }
    QString itemName;
    if (m_item->isLayout()) {
        itemName = QLatin1String("layout");
    } else {
        if (QGraphicsItem *item = m_item->graphicsItem()) {
            itemName = item->data(0).toString();
        }
    }
    edge.insert(0, QLatin1String("%1_"));
    return edge.arg(itemName);
}
#endif

/*!
  \internal

  Represents an edge (anchor) in the internal graph.
*/
struct AnchorData : public QSimplexVariable {
    enum Type {
        Normal = 0,
        Sequential,
        Parallel
    };
    AnchorData(qreal minimumSize, qreal preferredSize, qreal maximumSize)
        : QSimplexVariable(), from(0), to(0),
          minSize(minimumSize), prefSize(preferredSize),
          maxSize(maximumSize), sizeAtMinimum(preferredSize),
          sizeAtPreferred(preferredSize), sizeAtMaximum(preferredSize),
          skipInPreferred(0), type(Normal), hasSize(true),
          isLayoutAnchor(false) {}

    AnchorData(qreal size)
        : QSimplexVariable(), from(0), to(0),
          minSize(size), prefSize(size), maxSize(size),
          sizeAtMinimum(size), sizeAtPreferred(size), sizeAtMaximum(size),
          skipInPreferred(0), type(Normal), hasSize(true),
          isLayoutAnchor(false) {}

    AnchorData()
        : QSimplexVariable(), from(0), to(0),
          minSize(0), prefSize(0), maxSize(0),
          sizeAtMinimum(0), sizeAtPreferred(0), sizeAtMaximum(0),
          skipInPreferred(0), type(Normal), hasSize(false),
          isLayoutAnchor(false) {}

    virtual void updateChildrenSizes() {}
    virtual void refreshSizeHints(qreal effectiveSpacing);

    virtual ~AnchorData() {}

#ifdef QT_DEBUG
    void dump(int indent = 2);
    inline QString toString() const;
    QString name;
#endif

    inline void setFixedSize(qreal size)
    {
        minSize = size;
        prefSize = size;
        maxSize = size;
        sizeAtMinimum = size;
        sizeAtPreferred = size;
        sizeAtMaximum = size;
        hasSize = true;
    }

    inline void unsetSize()
    {
        hasSize = false;
    }

    // Anchor is semantically directed
    AnchorVertex *from;
    AnchorVertex *to;

    // Size restrictions of this edge. For anchors internal to items, these
    // values are derived from the respective item size hints. For anchors
    // that were added by users, these values are equal to the specified anchor
    // size.
    qreal minSize;
    qreal prefSize;
    qreal maxSize;

    // These attributes define which sizes should that anchor be in when the
    // layout is at its minimum, preferred or maximum sizes. Values are
    // calculated by the Simplex solver based on the current layout setup.
    qreal sizeAtMinimum;
    qreal sizeAtPreferred;
    qreal sizeAtMaximum;

    uint skipInPreferred : 1;
    uint type : 2;            // either Normal, Sequential or Parallel
    uint hasSize : 1;         // if false, get size from style.
    uint isLayoutAnchor : 1;  // if this anchor is connected to a layout 'edge'
protected:
    AnchorData(Type type, qreal size = 0)
        : QSimplexVariable(), from(0), to(0),
          minSize(size), prefSize(size),
          maxSize(size), sizeAtMinimum(size),
          sizeAtPreferred(size), sizeAtMaximum(size),
          skipInPreferred(0), type(type), hasSize(true),
          isLayoutAnchor(false) {}
};

#ifdef QT_DEBUG
inline QString AnchorData::toString() const
{
    return QString::fromAscii("Anchor(%1)").arg(name);
}
#endif

struct SequentialAnchorData : public AnchorData
{
    SequentialAnchorData() : AnchorData(AnchorData::Sequential)
    {
#ifdef QT_DEBUG
        name = QLatin1String("SequentialAnchorData");
#endif
    }

    virtual void updateChildrenSizes();
    virtual void refreshSizeHints(qreal effectiveSpacing);

    void setVertices(const QVector<AnchorVertex*> &vertices)
    {
        m_children = vertices;
#ifdef QT_DEBUG
        name = QString::fromAscii("%1 -- %2").arg(vertices.first()->toString(), vertices.last()->toString());
#endif
    }

    QVector<AnchorVertex*> m_children;          // list of vertices in the sequence
    QVector<AnchorData*> m_edges;               // keep the list of edges too.
};

struct ParallelAnchorData : public AnchorData
{
    ParallelAnchorData(AnchorData *first, AnchorData *second)
        : AnchorData(AnchorData::Parallel),
        firstEdge(first), secondEdge(second)
    {
        // ### Those asserts force that both child anchors have the same direction,
        // but can't we simplify a pair of anchors in opposite directions?
        Q_ASSERT(first->from == second->from);
        Q_ASSERT(first->to == second->to);
        from = first->from;
        to = first->to;
#ifdef QT_DEBUG
        name = QString::fromAscii("%1 | %2").arg(first->toString(), second->toString());
#endif
    }

    virtual void updateChildrenSizes();
    virtual void refreshSizeHints(qreal effectiveSpacing);

    AnchorData* firstEdge;
    AnchorData* secondEdge;
};

/*!
  \internal

  Representation of a valid path for a given vertex in the graph.
  In this struct, "positives" is the set of anchors that have been
  traversed in the forward direction, while "negatives" is the set
  with the ones walked backwards.

  This paths are compared against each other to produce LP Constraints,
  the exact order in which the anchors were traversed is not relevant.
*/
class GraphPath
{
public:
    GraphPath() {}

    QSimplexConstraint *constraint(const GraphPath &path) const;
#ifdef QT_DEBUG
    QString toString() const;
#endif
    QSet<AnchorData *> positives;
    QSet<AnchorData *> negatives;
};


/*!
  \internal

  QGraphicsAnchorLayout private methods and attributes.
*/
class QGraphicsAnchorLayoutPrivate : public QGraphicsLayoutPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsAnchorLayout)

public:
    // When the layout geometry is different from its Minimum, Preferred
    // or Maximum values, interpolation is used to calculate the geometries
    // of the items.
    //
    // Interval represents which interpolation interval are we operating in.
    enum Interval {
        MinToPreferred = 0,
        PreferredToMax
    };

    // Several structures internal to the layout are duplicated to handle
    // both Horizontal and Vertical restrictions.
    //
    // Orientation is used to reference the right structure in each context
    enum Orientation {
        Horizontal = 0,
        Vertical,
        NOrientations
    };

    QGraphicsAnchorLayoutPrivate();

    static Qt::AnchorPoint oppositeEdge(
        Qt::AnchorPoint edge);

    static Orientation edgeOrientation(Qt::AnchorPoint edge);

    static Qt::AnchorPoint pickEdge(Qt::AnchorPoint edge, Orientation orientation)
    {
        if (orientation == Vertical && int(edge) <= 2)
            return (Qt::AnchorPoint)(edge + 3);
        else if (orientation == Horizontal && int(edge) >= 3) {
            return (Qt::AnchorPoint)(edge - 3);
        }
        return edge;
    }

    // Init methods
    void createLayoutEdges();
    void deleteLayoutEdges();
    void createItemEdges(QGraphicsLayoutItem *item);
    void createCenterAnchors(QGraphicsLayoutItem *item, Qt::AnchorPoint centerEdge);
    void removeCenterAnchors(QGraphicsLayoutItem *item, Qt::AnchorPoint centerEdge, bool substitute = true);
    void removeCenterConstraints(QGraphicsLayoutItem *item, Orientation orientation);

    // helper function used by the 4 API functions
    void anchor(QGraphicsLayoutItem *firstItem,
                Qt::AnchorPoint firstEdge,
                QGraphicsLayoutItem *secondItem,
                Qt::AnchorPoint secondEdge,
                qreal *spacing = 0);

    // Anchor Manipulation methods
    void addAnchor(QGraphicsLayoutItem *firstItem,
                   Qt::AnchorPoint firstEdge,
                   QGraphicsLayoutItem *secondItem,
                   Qt::AnchorPoint secondEdge,
                   AnchorData *data);

    void removeAnchor(QGraphicsLayoutItem *firstItem,
                      Qt::AnchorPoint firstEdge,
                      QGraphicsLayoutItem *secondItem,
                      Qt::AnchorPoint secondEdge);

    bool setAnchorSize(const QGraphicsLayoutItem *firstItem,
                       Qt::AnchorPoint firstEdge,
                       const QGraphicsLayoutItem *secondItem,
                       Qt::AnchorPoint secondEdge,
                       const qreal *anchorSize);

    bool anchorSize(const QGraphicsLayoutItem *firstItem,
                    Qt::AnchorPoint firstEdge,
                    const QGraphicsLayoutItem *secondItem,
                    Qt::AnchorPoint secondEdge,
                    qreal *minSize = 0,
                    qreal *prefSize = 0,
                    qreal *maxSize = 0) const;

    void removeAnchors(QGraphicsLayoutItem *item);

    void removeVertex(QGraphicsLayoutItem *item, Qt::AnchorPoint edge);

    void correctEdgeDirection(QGraphicsLayoutItem *&firstItem,
                              Qt::AnchorPoint &firstEdge,
                              QGraphicsLayoutItem *&secondItem,
                              Qt::AnchorPoint &secondEdge);
    // for getting the actual spacing (will query the style if the
    // spacing is not explicitly set).
    qreal effectiveSpacing(Orientation orientation) const;

    // Activation methods
    void simplifyGraph(Orientation orientation);
    bool simplifyGraphIteration(Orientation orientation);
    void restoreSimplifiedGraph(Orientation orientation);

    void calculateGraphs();
    void calculateGraphs(Orientation orientation);
    void setAnchorSizeHintsFromItems(Orientation orientation);
    void findPaths(Orientation orientation);
    void constraintsFromPaths(Orientation orientation);
    QList<QSimplexConstraint *> constraintsFromSizeHints(const QList<AnchorData *> &anchors);
    QList<QList<QSimplexConstraint *> > getGraphParts(Orientation orientation);

    inline AnchorVertex *internalVertex(const QPair<QGraphicsLayoutItem*, Qt::AnchorPoint> &itemEdge) const
    {
        return m_vertexList.value(itemEdge).first;
    }

    inline AnchorVertex *internalVertex(const QGraphicsLayoutItem *item, Qt::AnchorPoint edge) const
    {
        return internalVertex(qMakePair(const_cast<QGraphicsLayoutItem *>(item), edge));
    }

    AnchorVertex *addInternalVertex(QGraphicsLayoutItem *item, Qt::AnchorPoint edge);
    void removeInternalVertex(QGraphicsLayoutItem *item, Qt::AnchorPoint edge);

    // Geometry interpolation methods
    void setItemsGeometries();

    void calculateVertexPositions(Orientation orientation);
    void setupEdgesInterpolation(Orientation orientation);
    void interpolateEdge(AnchorVertex *base, AnchorData *edge, Orientation orientation);
    void interpolateSequentialEdges(AnchorVertex *base, SequentialAnchorData *edge,
                                    Orientation orientation);
    void interpolateParallelEdges(AnchorVertex *base, ParallelAnchorData *edge,
                                  Orientation orientation);

    // Linear Programming solver methods
    QPair<qreal, qreal> solveMinMax(QList<QSimplexConstraint *> constraints,
                                    GraphPath path);
    void solvePreferred(QList<QSimplexConstraint *> constraints);

#ifdef QT_DEBUG
    void dumpGraph();
#endif


    qreal spacings[NOrientations];
    // Size hints from simplex engine
    qreal sizeHints[2][3];

    // Items
    QVector<QGraphicsLayoutItem *> items;

    // Mapping between high level anchorage points (Item, Edge) to low level
    // ones (Graph Vertices)

    QHash<QPair<QGraphicsLayoutItem*, Qt::AnchorPoint>, QPair<AnchorVertex *, int> > m_vertexList;

    // Internal graph of anchorage points and anchors, for both orientations
    Graph<AnchorVertex, AnchorData> graph[2];

    // Graph paths and constraints, for both orientations
    QMultiHash<AnchorVertex *, GraphPath> graphPaths[2];
    QList<QSimplexConstraint *> constraints[2];
    QList<QSimplexConstraint *> itemCenterConstraints[2];

    // The interpolation interval and progress based on the current size
    // as well as the key values (minimum, preferred and maximum)
    Interval interpolationInterval[2];
    qreal interpolationProgress[2];

    // ###
    bool graphSimplified[2];

    uint calculateGraphCacheDirty : 1;
};

QT_END_NAMESPACE

#endif
