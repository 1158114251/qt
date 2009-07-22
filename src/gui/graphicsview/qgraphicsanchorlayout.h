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

#ifndef QGRAPHICSANCHORLAYOUT_H
#define QGRAPHICSANCHORLAYOUT_H

#include <QtGui/qgraphicsitem.h>
#include <QtGui/qgraphicslayout.h>


QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#if !defined(QT_NO_GRAPHICSVIEW) || (QT_EDITION & QT_MODULE_GRAPHICSVIEW) != QT_MODULE_GRAPHICSVIEW

class QGraphicsAnchorLayoutPrivate;

class Q_GUI_EXPORT QGraphicsAnchorLayout : public QGraphicsLayout
{
public:
    enum Edge {
        Left = 0,
        HCenter,
        Right,
        Top,
        VCenter,
        Bottom
        };

    QGraphicsAnchorLayout(QGraphicsLayoutItem *parent = 0);
    virtual ~QGraphicsAnchorLayout();

    void anchor(QGraphicsLayoutItem *firstItem, Edge firstEdge,
                QGraphicsLayoutItem *secondItem, Edge secondEdge);

    void anchor(QGraphicsLayoutItem *firstItem, Edge firstEdge,
                QGraphicsLayoutItem *secondItem, Edge secondEdge,
                qreal spacing);

    void anchor(QGraphicsLayoutItem *firstItem, Qt::Corner firstCorner,
                QGraphicsLayoutItem *secondItem, Qt::Corner secondCorner);

    void anchor(QGraphicsLayoutItem *firstItem, Qt::Corner firstCorner,
                QGraphicsLayoutItem *secondItem, Qt::Corner secondCorner,
                qreal spacing);

    void removeAnchor(QGraphicsLayoutItem *firstItem, Edge firstEdge,
                      QGraphicsLayoutItem *secondItem, Edge secondEdge);

    inline void fillWidth(QGraphicsLayoutItem *item,
                          QGraphicsLayoutItem *relativeTo = 0);
    inline void fillHeight(QGraphicsLayoutItem *item,
                           QGraphicsLayoutItem *relativeTo = 0);
    inline void fill(QGraphicsLayoutItem *item,
                     QGraphicsLayoutItem *relativeTo = 0);

    void setSpacing(qreal spacing, Qt::Orientations orientations = Qt::Horizontal|Qt::Vertical);
    qreal spacing(Qt::Orientation) const;

    void removeAt(int index);
    void setGeometry(const QRectF &rect);
    int count() const;
    QGraphicsLayoutItem *itemAt(int index) const;

    void invalidate();
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

    ///////// DEBUG /////////
    void dumpGraph();
protected:

private:
    Q_DISABLE_COPY(QGraphicsAnchorLayout)
    Q_DECLARE_PRIVATE(QGraphicsAnchorLayout)
};

void QGraphicsAnchorLayout::fillWidth(QGraphicsLayoutItem *item,
                                      QGraphicsLayoutItem *relativeTo)
{
    if (!relativeTo)
        relativeTo = this;

    anchor(relativeTo, Left, item, Left);
    anchor(item, Right, relativeTo, Right);
}

void QGraphicsAnchorLayout::fillHeight(QGraphicsLayoutItem *item,
                                       QGraphicsLayoutItem *relativeTo)
{
    if (!relativeTo)
        relativeTo = this;

    anchor(relativeTo, Top, item, Top);
    anchor(item, Bottom, relativeTo, Bottom);
}

void QGraphicsAnchorLayout::fill(QGraphicsLayoutItem *item,
                                 QGraphicsLayoutItem *relativeTo)
{
    if (!relativeTo)
        relativeTo = this;

    fillWidth(item, relativeTo);
    fillHeight(item, relativeTo);
}

#endif

QT_END_NAMESPACE

QT_END_HEADER

#endif
