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

#include "qfxanchors_p.h"
#include "qfxitem.h"
#include <QDebug>
#include <QtDeclarative/qmlinfo.h>
#include <QtDeclarative/qmlbindablevalue.h>

QT_BEGIN_NAMESPACE

QML_DEFINE_TYPE(QFxAnchors,Anchors);

//TODO: should we cache relationships, so we don't have to check each time (parent-child or sibling)?
//TODO: baseline support
//TODO: support non-parent, non-sibling (need to find lowest common ancestor)

//### const item?
//local position
static qreal position(QFxItem *item, QFxAnchorLine::AnchorLine anchorLine)
{
    qreal ret = 0.0;
    switch(anchorLine) {
    case QFxAnchorLine::Left:
        ret = item->x();
        break;
    case QFxAnchorLine::Right:
        ret = item->x() + item->width();
        break;
    case QFxAnchorLine::Top:
        ret = item->y();
        break;
    case QFxAnchorLine::Bottom:
        ret = item->y() + item->height();
        break;
    case QFxAnchorLine::HCenter:
        ret = item->x() + item->width()/2;
        break;
    case QFxAnchorLine::VCenter:
        ret = item->y() + item->height()/2;
        break;
    case QFxAnchorLine::Baseline:
        ret = item->y() + item->baselineOffset();
        break;
    default:
        break;
    }

    return ret;
}

//position when origin is 0,0
static qreal adjustedPosition(QFxItem *item, QFxAnchorLine::AnchorLine anchorLine)
{
    int ret = 0;
    switch(anchorLine) {
    case QFxAnchorLine::Left:
        ret = 0;
        break;
    case QFxAnchorLine::Right:
        ret = item->width();
        break;
    case QFxAnchorLine::Top:
        ret = 0;
        break;
    case QFxAnchorLine::Bottom:
        ret = item->height();
        break;
    case QFxAnchorLine::HCenter:
        ret = item->width()/2;
        break;
    case QFxAnchorLine::VCenter:
        ret = item->height()/2;
        break;
    case QFxAnchorLine::Baseline:
        ret = item->baselineOffset();
        break;
    default:
        break;
    }

    return ret;
}

/*!
    \internal
    \class QFxAnchors
    \ingroup group_layouts
    \brief The QFxAnchors class provides a way to lay out items relative to other items.

    \warning Currently, only anchoring to siblings or parent is supported.
*/

QFxAnchors::QFxAnchors(QObject *parent)
  : QObject(*new QFxAnchorsPrivate(), parent)
{

}

void QFxAnchors::fillChanged()
{
    Q_D(QFxAnchors);
    if (!d->fill)
        return;

    if (d->fill == d->item->itemParent()) {                         //child-parent
        d->item->setPos(QPointF(leftMargin(), topMargin()));
    } else if (d->fill->itemParent() == d->item->itemParent()) {   //siblings
        d->item->setPos(QPointF(d->fill->x()+leftMargin(), d->fill->y()+topMargin()));
    }
    d->item->setWidth(d->fill->width()-leftMargin()-rightMargin());
    d->item->setHeight(d->fill->height()-topMargin()-bottomMargin());
}

/*!
    \property QFxAnchors::fill
    \brief which item the item should fill.

    This is a convenience property. It is the same as anchoring the left, right, top, and bottom
    to another item's left, right, top, and bottom.
*/
QFxItem *QFxAnchors::fill() const
{
    Q_D(const QFxAnchors);
    return d->fill;
}

void QFxAnchors::setFill(QFxItem *f)
{
    Q_D(QFxAnchors);
    if (d->fill) {
        QObject::disconnect(d->fill, SIGNAL(leftChanged()), this, SLOT(fillChanged()));
        QObject::disconnect(d->fill, SIGNAL(topChanged()), this, SLOT(fillChanged()));
        QObject::disconnect(d->fill, SIGNAL(widthChanged()), this, SLOT(fillChanged()));
        QObject::disconnect(d->fill, SIGNAL(heightChanged()), this, SLOT(fillChanged()));
        QObject::disconnect(this, SIGNAL(leftMarginChanged()), this, SLOT(fillChanged()));
        QObject::disconnect(this, SIGNAL(topMarginChanged()), this, SLOT(fillChanged()));
        QObject::disconnect(this, SIGNAL(rightMarginChanged()), this, SLOT(fillChanged()));
        QObject::disconnect(this, SIGNAL(bottomMarginChanged()), this, SLOT(fillChanged()));
    }

    d->fill = f;

    if (d->fill) {
        if (d->fill == d->item->itemParent()) {    //child-parent
            QObject::connect(d->fill, SIGNAL(widthChanged()), this, SLOT(fillChanged()));
            QObject::connect(d->fill, SIGNAL(heightChanged()), this, SLOT(fillChanged()));
        } else if (f->itemParent() == d->item->itemParent()) {   //siblings
            QObject::connect(d->fill, SIGNAL(leftChanged()), this, SLOT(fillChanged()));
            QObject::connect(d->fill, SIGNAL(topChanged()), this, SLOT(fillChanged()));
            QObject::connect(d->fill, SIGNAL(widthChanged()), this, SLOT(fillChanged()));
            QObject::connect(d->fill, SIGNAL(heightChanged()), this, SLOT(fillChanged()));
        } else {
            qmlInfo(d->item) << "Can't anchor to an item that isn't a parent or sibling.";
        }
    }
    QObject::connect(this, SIGNAL(leftMarginChanged()), this, SLOT(fillChanged()));
    QObject::connect(this, SIGNAL(topMarginChanged()), this, SLOT(fillChanged()));
    QObject::connect(this, SIGNAL(rightMarginChanged()), this, SLOT(fillChanged()));
    QObject::connect(this, SIGNAL(bottomMarginChanged()), this, SLOT(fillChanged()));
    fillChanged();  //### can/should we defer until component completion?
}

/*!
    \property QFxAnchors::centeredIn
    \brief which item the item should stay centered in.

    This is a convenience property. It is the same as anchoring the horizontalCenter
    and verticalCenter to another item's horizontalCenter and verticalCenter.
*/
QFxItem *QFxAnchors::centeredIn() const
{
    Q_D(const QFxAnchors);
    return d->centeredIn;
}

void QFxAnchors::setCenteredIn(QFxItem* c)
{
    Q_D(QFxAnchors);
    if (!c){
        qmlInfo(d->item) << "Cannot center in null item.";
        return;
    }
    if (c != d->item->itemParent() && c->itemParent() != d->item->itemParent()){
        qmlInfo(d->item) << "Can't anchor to an item that isn't a parent or sibling.";
        return;
    }
    d->centeredIn = c;
    setHorizontalCenter(c->horizontalCenter());
    setVerticalCenter(c->verticalCenter());
}

void QFxAnchorsPrivate::connectVHelper(const QFxAnchorLine &edge)
{
    //### should we do disconnects first? (will it be called more than once?)
    Q_Q(QFxAnchors);
    if (edge.item == item->itemParent()) {    //child-parent
        switch(edge.anchorLine) {
        case QFxAnchorLine::Bottom:
        case QFxAnchorLine::VCenter:
            QObject::connect(edge.item, SIGNAL(heightChanged()), q, SLOT(updateVerticalAnchors()));
            break;
        case QFxAnchorLine::Top: //no connection needed
        default:
            break;
        }
    } else if (edge.item->itemParent() == item->itemParent()) {   //siblings
        switch(edge.anchorLine) {
        case QFxAnchorLine::Top:
            QObject::connect(edge.item, SIGNAL(topChanged()), q, SLOT(updateVerticalAnchors()));
            break;
        case QFxAnchorLine::Bottom:
            QObject::connect(edge.item, SIGNAL(bottomChanged()), q, SLOT(updateVerticalAnchors()));
            break;
        case QFxAnchorLine::VCenter:
            QObject::connect(edge.item, SIGNAL(vcenterChanged()), q, SLOT(updateVerticalAnchors()));
            break;
        default:
            break;
        }
    } else {
        qmlInfo(item) << "Can't anchor to an item that isn't a parent or sibling.";
    }
}

void QFxAnchors::connectVAnchors()
{
    Q_D(QFxAnchors);
    if (!d->checkVValid())
        return;

    if (d->usedAnchors & HasTopAnchor) {
        //Handle stretching connections (if we have multiple horizontal anchors)
        QFxAnchorLine *edge = 0;
        if (d->usedAnchors & HasBottomAnchor) {
            edge = &d->bottom;
            connect(this, SIGNAL(bottomMarginChanged()), this, SLOT(updateVerticalAnchors()));
        } else if (d->usedAnchors & HasVCenterAnchor) {
            edge = &d->vCenter;
            connect(this, SIGNAL(verticalCenterOffsetChanged()), this, SLOT(updateVerticalAnchors()));
        }
        if (edge) {
            //we need to stretch
            d->connectVHelper(*edge);
        }

        //Handle top
        d->connectVHelper(d->top);
        connect(this, SIGNAL(topMarginChanged()), this, SLOT(updateVerticalAnchors()));
        updateVerticalAnchors();
    } else if (d->usedAnchors & HasBottomAnchor) {
        //Handle stretching connections (if we have multiple horizontal anchors)
        if (d->usedAnchors & HasVCenterAnchor) {
            d->connectVHelper(d->vCenter);
            connect(this, SIGNAL(verticalCenterOffsetChanged()), this, SLOT(updateVerticalAnchors()));
        }

        //Handle bottom
        d->connectVHelper(d->bottom);
        connect(this, SIGNAL(bottomMarginChanged()), this, SLOT(updateVerticalAnchors()));
        updateVerticalAnchors();
    } else if (d->usedAnchors & HasVCenterAnchor) {
        //Handle vCenter
        d->connectVHelper(d->vCenter);
        connect(this, SIGNAL(verticalCenterOffsetChanged()), this, SLOT(updateVerticalAnchors()));
        updateVerticalAnchors();
    }
}

void QFxAnchorsPrivate::connectHHelper(const QFxAnchorLine &edge)
{
    //### should we do disconnects first? (will it be called more than once?)
    Q_Q(QFxAnchors);
    if (edge.item == item->itemParent()) {    //child-parent
        switch(edge.anchorLine) {
        case QFxAnchorLine::Right:
        case QFxAnchorLine::HCenter:
            QObject::connect(edge.item, SIGNAL(widthChanged()), q, SLOT(updateHorizontalAnchors()));
            break;
        case QFxAnchorLine::Left: //no connection needed
        default:
            break;
        }
    } else if (edge.item->itemParent() == item->itemParent()) {   //siblings
        switch(edge.anchorLine) {
        case QFxAnchorLine::Left:
            QObject::connect(edge.item, SIGNAL(leftChanged()), q, SLOT(updateHorizontalAnchors()));
            break;
        case QFxAnchorLine::Right:
            QObject::connect(edge.item, SIGNAL(rightChanged()), q, SLOT(updateHorizontalAnchors()));
            break;
        case QFxAnchorLine::HCenter:
            QObject::connect(edge.item, SIGNAL(hcenterChanged()), q, SLOT(updateHorizontalAnchors()));
            break;
        default:
            break;
        }
    } else {
        qmlInfo(item) << "Can't anchor to an item that isn't a parent or sibling.";
    }
}

void QFxAnchors::connectHAnchors()
{
    Q_D(QFxAnchors);
    if (!d->checkHValid())
        return;

    if (d->usedAnchors & HasLeftAnchor) {
        //Handle stretching connections (if we have multiple horizontal anchors)
        QFxAnchorLine *edge = 0;
        if (d->usedAnchors & HasRightAnchor) {
            edge = &d->right;
            connect(this, SIGNAL(rightMarginChanged()), this, SLOT(updateHorizontalAnchors()));
        } else if (d->usedAnchors & HasHCenterAnchor) {
            edge = &d->hCenter;
            connect(this, SIGNAL(horizontalCenterOffsetChanged()), this, SLOT(updateHorizontalAnchors()));
        }
        if (edge) {
            //we need to stretch
            d->connectHHelper(*edge);
        }

        //Handle left
        d->connectHHelper(d->left);
        connect(this, SIGNAL(leftMarginChanged()), this, SLOT(updateHorizontalAnchors()));
        updateHorizontalAnchors();
    } else if (d->usedAnchors & HasRightAnchor) {
        //Handle stretching connections (if we have multiple horizontal anchors)
        if (d->usedAnchors & HasHCenterAnchor) {
            d->connectHHelper(d->hCenter);
            connect(this, SIGNAL(horizontalCenterOffsetChanged()), this, SLOT(updateHorizontalAnchors()));
        }

        //Handle right
        d->connectHHelper(d->right);
        connect(this, SIGNAL(rightMarginChanged()), this, SLOT(updateHorizontalAnchors()));
        updateHorizontalAnchors();
    } else if (d->usedAnchors & HasHCenterAnchor) {
        //Handle hCenter
        d->connectHHelper(d->hCenter);
        connect(this, SIGNAL(horizontalCenterOffsetChanged()), this, SLOT(updateHorizontalAnchors()));
        updateHorizontalAnchors();
    }
}

bool QFxAnchorsPrivate::calcStretch(const QFxAnchorLine &edge1,
                                    const QFxAnchorLine &edge2,
                                    int offset1,
                                    int offset2,
                                    QFxAnchorLine::AnchorLine line,
                                    int &stretch)
{
    bool edge1IsParent = (edge1.item == item->itemParent());
    bool edge2IsParent = (edge2.item == item->itemParent());
    bool edge1IsSibling = (edge1.item->itemParent() == item->itemParent());
    bool edge2IsSibling = (edge2.item->itemParent() == item->itemParent());

    bool invalid = false;
    if ((edge2IsParent && edge1IsParent) || (edge2IsSibling && edge1IsSibling)) {
        stretch = ((int)position(edge2.item, edge2.anchorLine) + offset2)
                    - ((int)position(edge1.item, edge1.anchorLine) + offset1);
    } else if (edge2IsParent && edge1IsSibling) {
        stretch = ((int)position(edge2.item, edge2.anchorLine) + offset2)
                    - ((int)position(item->itemParent(), line)
                    + (int)position(edge1.item, edge1.anchorLine) + offset1);
    } else if (edge2IsSibling && edge1IsParent) {
        stretch = ((int)position(item->itemParent(), line) + (int)position(edge2.item, edge2.anchorLine) + offset2)
                    - ((int)position(edge1.item, edge1.anchorLine) + offset1);
    } else
        invalid = true;

    return invalid;
}

void QFxAnchors::updateVerticalAnchors()
{
    Q_D(QFxAnchors);
    if (!d->updatingVerticalAnchor) {
        d->updatingVerticalAnchor = true;
        if (d->usedAnchors & HasTopAnchor) {
            //Handle stretching
            bool invalid = true;
            int height = 0;
            if (d->usedAnchors & HasBottomAnchor) {
                invalid = d->calcStretch(d->top, d->bottom, d->topMargin, -d->bottomMargin, QFxAnchorLine::Top, height);
            } else if (d->usedAnchors & HasVCenterAnchor) {
                invalid = d->calcStretch(d->top, d->vCenter, d->topMargin, d->vCenterOffset, QFxAnchorLine::Top, height);
                height *= 2;
            }
            if (!invalid)
                d->item->setHeight(height);

            //Handle top
            if (d->top.item == d->item->itemParent()) {
                d->item->setY(adjustedPosition(d->top.item, d->top.anchorLine) + d->topMargin);
            } else if (d->top.item->itemParent() == d->item->itemParent()) {
                d->item->setY(position(d->top.item, d->top.anchorLine) + d->topMargin);
            }
        } else if (d->usedAnchors & HasBottomAnchor) {
            //Handle stretching (top + bottom case is handled above)
            if (d->usedAnchors & HasVCenterAnchor) {
                int height = 0;
                bool invalid = d->calcStretch(d->vCenter, d->bottom, d->vCenterOffset, -d->bottomMargin,
                                              QFxAnchorLine::Top, height);
                if (!invalid)
                    d->item->setHeight(height*2);
            }

            //Handle bottom
            if (d->bottom.item == d->item->itemParent()) {
                d->item->setY(adjustedPosition(d->bottom.item, d->bottom.anchorLine) - d->item->height() - d->bottomMargin);
            } else if (d->bottom.item->itemParent() == d->item->itemParent()) {
                d->item->setY(position(d->bottom.item, d->bottom.anchorLine) - d->item->height() - d->bottomMargin);
            }


        } else if (d->usedAnchors & HasVCenterAnchor) {
            //(stetching handled above)

            //Handle vCenter
            if (d->vCenter.item == d->item->itemParent()) {
                d->item->setY(adjustedPosition(d->vCenter.item, d->vCenter.anchorLine)
                              - d->item->height()/2 + d->vCenterOffset);
            } else if (d->vCenter.item->itemParent() == d->item->itemParent()) {
                d->item->setY(position(d->vCenter.item, d->vCenter.anchorLine) - d->item->height()/2 + d->vCenterOffset);
            }
        }
        d->updatingVerticalAnchor = false;
    } else {
        qmlInfo(d->item) << "Anchor loop detected on vertical anchor.";
    }
}

void QFxAnchors::updateHorizontalAnchors()
{
    Q_D(QFxAnchors);
    if (!d->updatingHorizontalAnchor) {
        d->updatingHorizontalAnchor = true;

        //alternate implementation (needs performance testing)
        /*switch(d->usedAnchors & QFxAnchors::Horizontal_Mask) {
        case 0x03:              //(HasLeftAnchor | HasRightAnchor)
        {
            int width = 0;
            if (!d->calcStretch(d->left, d->right, d->leftMargin, -d->rightMargin, QFxAnchorLine::Left, width))
                d->item->setWidth(width);
            //fall though
        }
        case 0x11:              //(HasLeftAnchor | HasHCenterAnchor)
        {
            if (d->usedAnchors & HasHCenterAnchor) {
                int width = 0;
                if (!d->calcStretch(d->left, d->hCenter, d->leftMargin, d->hCenterOffset, QFxAnchorLine::Left, width))
                    d->item->setWidth(width*2);
            }
            //fall though
        }
        case HasLeftAnchor:
            if (d->left.item == d->item->itemParent()) {
                d->item->setX(adjustedPosition(d->left.item, d->left.anchorLine) + d->leftMargin);
            } else if (d->left.item->itemParent() == d->item->itemParent()) {
                d->item->setX(position(d->left.item, d->left.anchorLine) + d->leftMargin);
            }
            break;
        case 0x12:              //(HasRightAnchor | HasHCenterAnchor)
        {
            int width = 0;
            if (!d->calcStretch(d->hCenter, d->right, d->hCenterOffset, -d->rightMargin, QFxAnchorLine::Left, width))
                d->item->setWidth(width*2);
            //fall though
        }
        case HasRightAnchor:
            if (d->right.item == d->item->itemParent()) {
                d->item->setX(adjustedPosition(d->right.item, d->right.anchorLine) - d->item->width() - d->rightMargin);
            } else if (d->right.item->itemParent() == d->item->itemParent()) {
                d->item->setX(position(d->right.item, d->right.anchorLine) - d->item->width() - d->rightMargin);
            }
            break;
        case HasHCenterAnchor:
            if (d->hCenter.item == d->item->itemParent()) {
                d->item->setX(adjustedPosition(d->hCenter.item, d->hCenter.anchorLine) - d->item->width()/2 + d->hCenterOffset);
            } else if (d->hCenter.item->itemParent() == d->item->itemParent()) {
                d->item->setX(position(d->hCenter.item, d->hCenter.anchorLine) - d->item->width()/2 + d->hCenterOffset);
            }
            break;
        default:
            break;
        }*/

        if (d->usedAnchors & HasLeftAnchor) {
            //Handle stretching
            bool invalid = true;
            int width = 0;
            if (d->usedAnchors & HasRightAnchor) {
                invalid = d->calcStretch(d->left, d->right, d->leftMargin, -d->rightMargin, QFxAnchorLine::Left, width);
            } else if (d->usedAnchors & HasHCenterAnchor) {
                invalid = d->calcStretch(d->left, d->hCenter, d->leftMargin, d->hCenterOffset, QFxAnchorLine::Left, width);
                width *= 2;
            }
            if (!invalid)
                d->item->setWidth(width);

            //Handle left
            if (d->left.item == d->item->itemParent()) {
                d->item->setX(adjustedPosition(d->left.item, d->left.anchorLine) + d->leftMargin);
            } else if (d->left.item->itemParent() == d->item->itemParent()) {
                d->item->setX(position(d->left.item, d->left.anchorLine) + d->leftMargin);
            }
        } else if (d->usedAnchors & HasRightAnchor) {
            //Handle stretching (left + right case is handled in updateLeftAnchor)
            if (d->usedAnchors & HasHCenterAnchor) {
                int width = 0;
                bool invalid = d->calcStretch(d->hCenter, d->right, d->hCenterOffset, -d->rightMargin,
                                              QFxAnchorLine::Left, width);
                if (!invalid)
                    d->item->setWidth(width*2);
            }

            //Handle right
            if (d->right.item == d->item->itemParent()) {
                d->item->setX(adjustedPosition(d->right.item, d->right.anchorLine) - d->item->width() - d->rightMargin);
            } else if (d->right.item->itemParent() == d->item->itemParent()) {
                d->item->setX(position(d->right.item, d->right.anchorLine) - d->item->width() - d->rightMargin);
            }
        } else if (d->usedAnchors & HasHCenterAnchor) {
            //Handle hCenter
            if (d->hCenter.item == d->item->itemParent()) {
                d->item->setX(adjustedPosition(d->hCenter.item, d->hCenter.anchorLine) - d->item->width()/2 + d->hCenterOffset);
            } else if (d->hCenter.item->itemParent() == d->item->itemParent()) {
                d->item->setX(position(d->hCenter.item, d->hCenter.anchorLine) - d->item->width()/2 + d->hCenterOffset);
            }
        }

        d->updatingHorizontalAnchor = false;
    } else {
        qmlInfo(d->item) << "Anchor loop detected on horizontal anchor.";
    }
}

QFxAnchorLine QFxAnchors::top() const
{
    Q_D(const QFxAnchors);
    return d->top;
}

void QFxAnchors::setTop(const QFxAnchorLine &edge)
{
    Q_D(QFxAnchors);
    if (!d->checkVAnchorValid(edge))
        return;

    d->usedAnchors |= HasTopAnchor;

    d->checkVValid();

    d->top = edge;
}

void QFxAnchors::resetTop()
{
    Q_D(QFxAnchors);

    //update flags
    d->usedAnchors &= ~HasTopAnchor;

    //clear binding
    QmlMetaProperty prop(this, "top");
    prop.binding()->clearExpression();

    //disconnect signal/slot connections as needed
    disconnect(this, SIGNAL(topMarginChanged()), this, SLOT(updateVerticalAnchors()));
    disconnect(d->top.item, 0, this, 0);

    updateVerticalAnchors();
}

QFxAnchorLine QFxAnchors::bottom() const
{
    Q_D(const QFxAnchors);
    return d->bottom;
}

void QFxAnchors::setBottom(const QFxAnchorLine &edge)
{
    Q_D(QFxAnchors);
    if (!d->checkVAnchorValid(edge))
        return;

    d->usedAnchors |= HasBottomAnchor;

    d->checkVValid();

    d->bottom = edge;
}

void QFxAnchors::resetBottom()
{
    Q_D(QFxAnchors);

    //update flags
    d->usedAnchors &= ~HasBottomAnchor;

    //clear binding
    QmlMetaProperty prop(this, "bottom");
    prop.binding()->clearExpression();

    //disconnect signal/slot connections as needed
    disconnect(this, SIGNAL(bottomMarginChanged()), this, SLOT(updateVerticalAnchors()));
    disconnect(d->bottom.item, 0, this, 0);

    updateVerticalAnchors();
}

QFxAnchorLine QFxAnchors::verticalCenter() const
{
    Q_D(const QFxAnchors);
    return d->vCenter;
}

void QFxAnchors::setVerticalCenter(const QFxAnchorLine &edge)
{
    Q_D(QFxAnchors);
    if (!d->checkVAnchorValid(edge))
        return;

    d->usedAnchors |= HasVCenterAnchor;

    d->checkVValid();

    d->vCenter = edge;
}

void QFxAnchors::resetVerticalCenter()
{
    Q_D(QFxAnchors);

    //update flags
    d->usedAnchors &= ~HasVCenterAnchor;

    //clear binding
    QmlMetaProperty prop(this, "verticalCenter");
    prop.binding()->clearExpression();

    //disconnect signal/slot connections as needed
    disconnect(this, SIGNAL(verticalCenterOffsetChanged()), this, SLOT(updateVerticalAnchors()));
    disconnect(d->vCenter.item, 0, this, 0);

    updateVerticalAnchors();
}

QFxAnchorLine QFxAnchors::left() const
{
    Q_D(const QFxAnchors);
    return d->left;
}

void QFxAnchors::setLeft(const QFxAnchorLine &edge)
{
    Q_D(QFxAnchors);
    if (!d->checkHAnchorValid(edge))
        return;

    d->usedAnchors |= HasLeftAnchor;

    d->checkHValid();

    d->left = edge;
}

void QFxAnchors::resetLeft()
{
    Q_D(QFxAnchors);

    //update flags
    d->usedAnchors &= ~HasLeftAnchor;

    //clear binding
    QmlMetaProperty prop(this, "left");
    prop.binding()->clearExpression();

    //disconnect signal/slot connections as needed
    disconnect(this, SIGNAL(leftMarginChanged()), this, SLOT(updateHorizontalAnchors()));
    disconnect(d->left.item, 0, this, 0);

    updateHorizontalAnchors();
}

QFxAnchorLine QFxAnchors::right() const
{
    Q_D(const QFxAnchors);
    return d->right;
}

void QFxAnchors::setRight(const QFxAnchorLine &edge)
{
    Q_D(QFxAnchors);
    if (!d->checkHAnchorValid(edge))
        return;

    d->usedAnchors |= HasRightAnchor;

    d->checkHValid();

    d->right = edge;
}

void QFxAnchors::resetRight()
{
    Q_D(QFxAnchors);

    //update flags
    d->usedAnchors &= ~HasRightAnchor;

    //clear binding
    QmlMetaProperty prop(this, "right");
    prop.binding()->clearExpression();

    //disconnect signal/slot connections as needed
    disconnect(this, SIGNAL(rightMarginChanged()), this, SLOT(updateHorizontalAnchors()));
    disconnect(d->right.item, 0, this, 0);

    updateHorizontalAnchors();
}

QFxAnchorLine QFxAnchors::horizontalCenter() const
{
    Q_D(const QFxAnchors);
    return d->hCenter;
}

void QFxAnchors::setHorizontalCenter(const QFxAnchorLine &edge)
{
    Q_D(QFxAnchors);
    if (!d->checkHAnchorValid(edge))
        return;

    d->usedAnchors |= HasHCenterAnchor;

    d->checkHValid();

    d->hCenter = edge;
}

void QFxAnchors::resetHorizontalCenter()
{
    Q_D(QFxAnchors);

    //update flags
    d->usedAnchors &= ~HasHCenterAnchor;

    //clear binding
    QmlMetaProperty prop(this, "horizontalCenter");
    prop.binding()->clearExpression();

    //disconnect signal/slot connections as needed
    disconnect(this, SIGNAL(horizontalCenterOffsetChanged()), this, SLOT(updateHorizontalAnchors()));
    disconnect(d->hCenter.item, 0, this, 0);

    updateHorizontalAnchors();
}

int QFxAnchors::leftMargin() const
{
    Q_D(const QFxAnchors);
    return d->leftMargin;
}

void QFxAnchors::setLeftMargin(int offset)
{
    Q_D(QFxAnchors);
    if (d->leftMargin == offset)
        return;
    d->leftMargin = offset;
    emit leftMarginChanged();
}

int QFxAnchors::rightMargin() const
{
    Q_D(const QFxAnchors);
    return d->rightMargin;
}

void QFxAnchors::setRightMargin(int offset)
{
    Q_D(QFxAnchors);
    if (d->rightMargin == offset)
        return;
    d->rightMargin = offset;
    emit rightMarginChanged();
}

int QFxAnchors::horizontalCenterOffset() const
{
    Q_D(const QFxAnchors);
    return d->hCenterOffset;
}

void QFxAnchors::setHorizontalCenterOffset(int offset)
{
    Q_D(QFxAnchors);
    if (d->hCenterOffset == offset)
        return;
    d->hCenterOffset = offset;
    emit horizontalCenterOffsetChanged();
}

int QFxAnchors::topMargin() const
{
    Q_D(const QFxAnchors);
    return d->topMargin;
}

void QFxAnchors::setTopMargin(int offset)
{
    Q_D(QFxAnchors);
    if (d->topMargin == offset)
        return;
    d->topMargin = offset;
    emit topMarginChanged();
}

int QFxAnchors::bottomMargin() const
{
    Q_D(const QFxAnchors);
    return d->bottomMargin;
}

void QFxAnchors::setBottomMargin(int offset)
{
    Q_D(QFxAnchors);
    if (d->bottomMargin == offset)
        return;
    d->bottomMargin = offset;
    emit bottomMarginChanged();
}

int QFxAnchors::verticalCenterOffset() const
{
    Q_D(const QFxAnchors);
    return d->vCenterOffset;
}

void QFxAnchors::setVerticalCenterOffset(int offset)
{
    Q_D(QFxAnchors);
    if (d->vCenterOffset == offset)
        return;
    d->vCenterOffset = offset;
    emit verticalCenterOffsetChanged();
}

#if 0
/*!
    \property QFxAnchors::baseline
    \brief what the baseline of the item should be anchored to (aligned with).

    The baseline of a Text item is the imaginary line on which the text sits. Controls containing
    text usually set their baseline to the baseline of their text.

    For non-text items, a default baseline offset of two-thirds of the item's height is used
    to determine the baseline.
*/
int QFxAnchors::baseline() const
{
    return d->item->baseline();
}

void QFxAnchors::setBaseline(int baseline)
{
    d->usedAnchors |= HasBaselineAnchor;

    if (d->usedAnchors & HasTopAnchor && d->usedAnchors & HasBottomAnchor) {
        qmlInfo(d->item) << "Can't specify top, bottom, and baseline anchors";
        return;
    }

    if (d->usedAnchors & HasTopAnchor) {
        int hoffset = baseline - d->item->baseline();
        d->item->setHeight(d->item->height() + hoffset);
    } else {
        if (d->usedAnchors & HasBottomAnchor) {
            int hoffset = d->item->baseline() - baseline;
            d->item->setHeight(d->item->height() + hoffset);
        }

        int boffset = d->item->baseline() - d->item->top();
        QFxItem *parentItem = d->item->itemParent();
        if (parentItem)
            d->item->setY(baseline - boffset - parentItem->top());
        else
            d->item->setY(baseline - boffset);
    }
}
#endif

QFxAnchors::UsedAnchors QFxAnchors::usedAnchors() const
{
    Q_D(const QFxAnchors);
    return d->usedAnchors;
}

void QFxAnchors::setItem(QFxItem *item)
{
    Q_D(QFxAnchors);
    d->item = item;
}

bool QFxAnchorsPrivate::checkHValid() const
{
    if (usedAnchors & QFxAnchors::HasLeftAnchor &&
        usedAnchors & QFxAnchors::HasRightAnchor &&
        usedAnchors & QFxAnchors::HasHCenterAnchor) {
        qmlInfo(item) << "Can't specify left, right, and hcenter anchors";
        return false;
    }

    return true;
}

bool QFxAnchorsPrivate::checkHAnchorValid(QFxAnchorLine anchor) const
{
    if (anchor.anchorLine & QFxAnchorLine::Vertical_Mask) {
        qmlInfo(item) << "Can't anchor a horizontal edge to a vertical edge.";
        return false;
    }else if (anchor.item == item){
        qmlInfo(item) << "Can't anchor item to self.";
        return false;
    }

    return true;
}

bool QFxAnchorsPrivate::checkVValid() const
{
    if (usedAnchors & QFxAnchors::HasTopAnchor &&
        usedAnchors & QFxAnchors::HasBottomAnchor &&
        usedAnchors & QFxAnchors::HasVCenterAnchor) {
        qmlInfo(item) << "Can't specify top, bottom, and vcenter anchors";
        return false;
    }

    return true;
}

bool QFxAnchorsPrivate::checkVAnchorValid(QFxAnchorLine anchor) const
{
    if (anchor.anchorLine & QFxAnchorLine::Horizontal_Mask) {
        qmlInfo(item) << "Can't anchor a vertical edge to a horizontal edge.";
        return false;
    }else if (anchor.item == item){
        qmlInfo(item) << "Can't anchor item to self.";
        return false;
    }

    return true;
}

QT_END_NAMESPACE
