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

#include <QDebug>
#include <QCoreApplication>
#include "qml.h"
#include "qmlstate.h"
#include "qmlstategroup.h"
#include "qmlstateoperations.h"
#include "private/qfxperf_p.h"
#include "qfxpositioners.h"
#include "qfxpositioners_p.h"


QT_BEGIN_NAMESPACE

/*!
    \internal
    \class QmlGraphicsBasePositioner
    \ingroup group_layouts
    \brief The QmlGraphicsBasePositioner class provides a base for QmlGraphics layouts.

    To create a QmlGraphics Positioner, simply subclass QmlGraphicsBasePositioner and implement
    doLayout(), which is automatically called when the layout might need
    updating.

    It is strongly recommended that in your implementation of doLayout()
    that you use the move, remove and add transitions when those conditions
    arise. You can use the applyAdd, applyMove and applyRemove functions
    to do this easily.

    Note also that the subclass is responsible for adding the
    spacing in between items.
*/
QmlGraphicsBasePositioner::QmlGraphicsBasePositioner(AutoUpdateType at, QmlGraphicsItem *parent)
    : QmlGraphicsItem(*(new QmlGraphicsBasePositionerPrivate), parent)
{
    Q_D(QmlGraphicsBasePositioner);
    d->init(at);
}

QmlGraphicsBasePositioner::QmlGraphicsBasePositioner(QmlGraphicsBasePositionerPrivate &dd, AutoUpdateType at, QmlGraphicsItem *parent)
    : QmlGraphicsItem(dd, parent)
{
    Q_D(QmlGraphicsBasePositioner);
    d->init(at);
}

int QmlGraphicsBasePositioner::spacing() const
{
    Q_D(const QmlGraphicsBasePositioner);
    return d->_spacing;
}

void QmlGraphicsBasePositioner::setSpacing(int s)
{
    Q_D(QmlGraphicsBasePositioner);
    if (s==d->_spacing)
        return;
    d->_spacing = s;
    prePositioning();
    emit spacingChanged();
}

QmlTransition *QmlGraphicsBasePositioner::move() const
{
    Q_D(const QmlGraphicsBasePositioner);
    return d->moveTransition;
}

void QmlGraphicsBasePositioner::setMove(QmlTransition *mt)
{
    Q_D(QmlGraphicsBasePositioner);
    d->moveTransition = mt;
}

QmlTransition *QmlGraphicsBasePositioner::add() const
{
    Q_D(const QmlGraphicsBasePositioner);
    return d->addTransition;
}

void QmlGraphicsBasePositioner::setAdd(QmlTransition *add)
{
    Q_D(QmlGraphicsBasePositioner);
    d->addTransition = add;
}

QmlTransition *QmlGraphicsBasePositioner::remove() const
{
    Q_D(const QmlGraphicsBasePositioner);
    return d->removeTransition;
}

void QmlGraphicsBasePositioner::setRemove(QmlTransition *remove)
{
    Q_D(QmlGraphicsBasePositioner);
    d->removeTransition = remove;
}

void QmlGraphicsBasePositioner::componentComplete()
{
    QmlGraphicsItem::componentComplete();
#ifdef Q_ENABLE_PERFORMANCE_LOG
    QmlPerfTimer<QmlPerf::BasepositionerComponentComplete> cc;
#endif
    prePositioning();
}

QVariant QmlGraphicsBasePositioner::itemChange(GraphicsItemChange change,
                                       const QVariant &value)
{
    if (change == ItemChildAddedChange ||
               change == ItemChildRemovedChange) {
        prePositioning();
    }

    return QmlGraphicsItem::itemChange(change, value);
}

bool QmlGraphicsBasePositioner::event(QEvent *e)
{
    Q_D(QmlGraphicsBasePositioner);
    if (e->type() == QEvent::User) {
        d->_ep = false;
        d->_stableItems += d->_newItems;
        d->_leavingItems.clear();
        d->_newItems.clear();
        return true;
    }
    return QmlGraphicsItem::event(e);
}

/*!
  Items that have just been added to the positioner. This includes invisible items
  that have turned visible.
*/
QSet<QmlGraphicsItem *>* QmlGraphicsBasePositioner::newItems()
{
    Q_D(QmlGraphicsBasePositioner);
    return &d->_newItems;
}

/*!
  Items that are visible in the positioner, not including ones that have just been added.
*/
QSet<QmlGraphicsItem *>* QmlGraphicsBasePositioner::items()
{
    Q_D(QmlGraphicsBasePositioner);
    return &d->_stableItems;
}

/*!
  Items that have just left the positioner. This includes visible items
  that have turned invisible.
*/
QSet<QmlGraphicsItem *>* QmlGraphicsBasePositioner::leavingItems()
{
    Q_D(QmlGraphicsBasePositioner);
    return &d->_leavingItems;
}

void QmlGraphicsBasePositioner::prePositioning()
{
    Q_D(QmlGraphicsBasePositioner);
    if (!isComponentComplete() || d->_movingItem)
        return;

    if (!d->_ep) {
        d->_ep = true;
        QCoreApplication::postEvent(this, new QEvent(QEvent::User));
    }
    QSet<QmlGraphicsItem *> allItems;
    QList<QGraphicsItem *> children = childItems();
    for (int ii = 0; ii < children.count(); ++ii) {
        QmlGraphicsItem *child = qobject_cast<QmlGraphicsItem *>(children.at(ii));
        if (!child)
            continue;
        if (!d->_items.contains(child)){
            QObject::connect(child, SIGNAL(visibleChanged()),
                             this, SLOT(prePositioning()));
            QObject::connect(child, SIGNAL(opacityChanged()),
                             this, SLOT(prePositioning()));
            QObject::connect(child, SIGNAL(heightChanged()),
                             this, SLOT(prePositioning()));
            QObject::connect(child, SIGNAL(widthChanged()),
                             this, SLOT(prePositioning()));
            d->_items += child;
        }
        if (child->opacity() == 0.0){
            if (d->_stableItems.contains(child)){
                d->_leavingItems += child;
                d->_stableItems -= child;
            }
        }else if (!d->_stableItems.contains(child)){
            d->_newItems+=child;
        }
        allItems += child;
    }
    QSet<QmlGraphicsItem *> deletedItems = d->_items - allItems;
    foreach(QmlGraphicsItem *child, d->_items){
        if (!allItems.contains(child)){
            if (!deletedItems.contains(child)) {
                QObject::disconnect(child, SIGNAL(opacityChanged()),
                                 this, SLOT(prePositioning()));
                QObject::disconnect(child, SIGNAL(heightChanged()),
                                 this, SLOT(prePositioning()));
                QObject::disconnect(child, SIGNAL(widthChanged()),
                                 this, SLOT(prePositioning()));
            }
            d->_items -= child;
        }
    }
    d->_animated.clear();
    doPositioning();
    finishApplyTransitions();
    //Set implicit size to the size of its children
    //###To keep this valid, do we need to update on pos change as well?
    qreal h = 0.0f;
    qreal w = 0.0f;
    foreach(QmlGraphicsItem *child, d->_items){
        if(!child->isVisible() || child->opacity() <= 0)
            continue;
        h = qMax(h, child->y() + child->height());
        w = qMax(w, child->x() + child->width());
    }
    setImplicitHeight(h);
    setImplicitWidth(w);
}

void QmlGraphicsBasePositioner::applyTransition(const QList<QPair<QString, QVariant> >& changes, QmlGraphicsItem* target, QmlStateOperation::ActionList &actions)
{
    Q_D(QmlGraphicsBasePositioner);
    if (!target)
        return;

    for (int ii=0; ii<changes.size(); ++ii){

        QVariant val = changes[ii].second;
        actions << Action(target, changes[ii].first, val);

    }

    d->_animated << target;
}

void QmlGraphicsBasePositioner::finishApplyTransitions()
{
    Q_D(QmlGraphicsBasePositioner);
    // Note that if a transition is not set the transition manager will
    // apply the changes directly, in the case someone uses applyAdd/Move/Remove
    // without testing add()/move()/remove().
    d->addTransitionManager.transition(d->addActions, d->addTransition);
    d->moveTransitionManager.transition(d->moveActions, d->moveTransition);
    d->removeTransitionManager.transition(d->removeActions, d->removeTransition);
    d->addActions.clear();
    d->moveActions.clear();
    d->removeActions.clear();
}
void QmlGraphicsBasePositioner::setMovingItem(QmlGraphicsItem *i)
{
    Q_D(QmlGraphicsBasePositioner);
    d->_movingItem = i;
}

/*!
  Applies the positioner's add transition to the \a target item.\a changes is a list of property,value
  pairs which will be changed on the target using the add transition.
*/
void QmlGraphicsBasePositioner::applyAdd(const QList<QPair<QString, QVariant> >& changes, QmlGraphicsItem* target)
{
    Q_D(QmlGraphicsBasePositioner);
    applyTransition(changes,target, d->addActions);
}

/*!
  Applies the positioner's move transition to the \a target.\a changes is a list of property,value pairs
  which will be changed on the target using the move transition.
*/
void QmlGraphicsBasePositioner::applyMove(const QList<QPair<QString, QVariant> >& changes, QmlGraphicsItem* target)
{
    Q_D(QmlGraphicsBasePositioner);
    applyTransition(changes,target, d->moveActions);
}

/*!
  Applies the positioner's remove transition to the \a target item.\a changes is a list of
  property,value pairs which will be changed on the target using the remove transition.
*/
void QmlGraphicsBasePositioner::applyRemove(const QList<QPair<QString, QVariant> >& changes, QmlGraphicsItem* target)
{
    Q_D(QmlGraphicsBasePositioner);
    applyTransition(changes,target, d->removeActions);
}

QML_DEFINE_TYPE(Qt,4,6,(QT_VERSION&0x00ff00)>>8,Column,QmlGraphicsColumn)
/*!
  \qmlclass Column
  \brief The Column item lines up its children vertically.
  \inherits Item

  The Column item positions its child items so that they are vertically
    aligned and not overlapping. Spacing between items can be added.

  The below example positions differently shaped rectangles using a Column.
  \table
  \row
  \o \image verticalpositioner_example.png
  \o
  \qml
Column {
    spacing: 2
    Rectangle { color: "red"; width: 50; height: 50 }
    Rectangle { color: "green"; width: 20; height: 50 }
    Rectangle { color: "blue"; width: 50; height: 20 }
}
  \endqml
  \endtable

  Column also provides for transitions to be set when items are added, moved,
  or removed in the positioner. Adding and removing apply both to items which are deleted
  or have their position in the document changed so as to no longer be children of the positioner,
  as well as to items which have their opacity set to or from zero so as to appear or disappear.

  \table
  \row
  \o \image verticalpositioner_transition.gif
  \o
  \qml
Column {
    spacing: 2
    remove: ...
    add: ...
    move: ...
    ...
}
  \endqml
  \endtable

  Note that the positioner assumes that the x and y positions of its children
  will not change. If you manually change the x or y properties in script, bind
  the x or y properties, or use anchors on a child of a positioner, then the
  positioner may exhibit strange behaviour.

*/
/*!
    \qmlproperty Transition Column::remove
    This property holds the transition to apply when removing an item from the positioner. The transition is only applied to the removed items.

    Removed can mean that either the object has been deleted or reparented, and thus is now longer a child of the positioner, or that the object has had its opacity set to zero, and thus is no longer visible.

    Note that if the item counts as removed because its opacity is zero it will not be visible during the transition unless you set the opacity in the transition, like in the below example.

    \table
    \row
    \o \image positioner-remove.gif
    \o
    \qml
Column {
    remove: Transition {
        NumberAnimation {
            properties: "opacity"
            from: 1
            to: 0
            duration: 500
        }
    }
}
    \endqml
    \endtable

*/
/*!
    \qmlproperty Transition Column::add
    This property holds the transition to be applied when adding an item to the positioner. The transition will only be applied to the added item(s).

    Added can mean that either the object has been created or reparented, and thus is now a child or the positioner, or that the object has had its opacity increased from zero, and thus is now visible.

    \table
    \row
    \o \image positioner-add.gif
    \o
    \qml
Column {
    add: Transition {
        NumberAnimation {
            properties: "opacity"
            from: 0
            to: 1
            duration: 500
        }
    }
}
    \endqml
    \endtable

*/
/*!
    \qmlproperty Transition Column::move
    This property holds the transition to apply when moving an item within the positioner.

    This can happen when other items are added or removed from the positioner, or when items resize themselves.

    \table
    \row
    \o \image positioner-move.gif
    \o
    \qml
Column {
    move: Transition {
        NumberAnimation {
            properties: "y"
            ease: "easeOutBounce"
        }
    }
}
    \endqml
    \endtable
*/
/*!
  \qmlproperty int Column::spacing

  spacing is the amount in pixels left empty between each adjacent
  item, and defaults to 0.

  The below example places a Grid containing a red, a blue and a
  green rectangle on a gray background. The area the grid positioner
  occupies is colored white. The top positioner has the default of no spacing,
  and the bottom positioner has its spacing set to 2.

  \image spacing_a.png
  \image spacing_b.png

*/
/*!
    \internal
    \class QmlGraphicsColumn
    \brief The QmlGraphicsColumn class lines up items vertically.
    \ingroup group_positioners
*/
QmlGraphicsColumn::QmlGraphicsColumn(QmlGraphicsItem *parent)
: QmlGraphicsBasePositioner(Vertical, parent)
{
}

void QmlGraphicsColumn::doPositioning()
{
    int voffset = 0;

    foreach(QmlGraphicsItem* item, *leavingItems()){
        if (remove()){
            QList<QPair<QString,QVariant> > changes;
            applyRemove(changes, item);
        }
    }

    QList<QGraphicsItem *> children = childItems();
    for (int ii = 0; ii < children.count(); ++ii) {
        QmlGraphicsItem *child = qobject_cast<QmlGraphicsItem *>(children.at(ii));
        if (!child || child->opacity() == 0.0)
            continue;

        bool needMove = (child->y() != voffset || child->x());

        QList<QPair<QString, QVariant> > changes;
        changes << qMakePair(QString(QLatin1String("y")),QVariant(voffset));
        changes << qMakePair(QString(QLatin1String("x")),QVariant(0));
        if (needMove && items()->contains(child) && move()) {
            applyMove(changes,child);
        } else if (!items()->contains(child) && add()) {
            applyAdd(changes,child);
        } else if (needMove) {
            setMovingItem(child);
            child->setY(voffset);
            setMovingItem(0);
        }
        voffset += child->height();
        voffset += spacing();
    }
}

QML_DEFINE_TYPE(Qt,4,6,(QT_VERSION&0x00ff00)>>8,Row,QmlGraphicsRow)
/*!
  \qmlclass Row
  \brief The Row item lines up its children horizontally.
  \inherits Item

  The Row item positions its child items so that they are
  horizontally aligned and not overlapping. Spacing can be added between the
  items, and a margin around all items can also be added. It also provides for
  transitions to be set when items are added, moved, or removed in the
  positioner. Adding and removing apply both to items which are deleted or have
  their position in the document changed so as to no longer be children of the
  positioner, as well as to items which have their opacity set to or from zero
  so as to appear or disappear.

  The below example lays out differently shaped rectangles using a Row.
  \qml
Row {
    spacing: 2
    Rectangle { color: "red"; width: 50; height: 50 }
    Rectangle { color: "green"; width: 20; height: 50 }
    Rectangle { color: "blue"; width: 50; height: 20 }
}
  \endqml
  \image horizontalpositioner_example.png

  Note that the positioner assumes that the x and y positions of its children
  will not change. If you manually change the x or y properties in script, bind
  the x or y properties, or use anchors on a child of a positioner, then the
  positioner may exhibit strange behaviour.

*/
/*!
    \qmlproperty Transition Row::remove
    This property holds the transition to apply when removing an item from the positioner.
    The transition will only be applied to the removed item(s).

    Removed can mean that either the object has been deleted or reparented, and thus is now longer a child of the positioner, or that the object has had its opacity set to zero, and thus is no longer visible.

    Note that if the item counts as removed because its opacity is zero it will not be visible during the transition unless you set the opacity in the transition, like in the below example.

    \qml
Row {
    remove: Transition {
        NumberAnimation {
            properties: "opacity"
            from: 1
            to: 0
            duration: 500
        }
    }
}
    \endqml

*/
/*!
    \qmlproperty Transition Row::add
    This property holds the transition to apply when adding an item to the positioner.
    The transition will only be applied to the added item(s).

    Added can mean that either the object has been created or reparented, and thus is now a child or the positioner, or that the object has had its opacity increased from zero, and thus is now visible.

    \qml
Row {
    add: Transition {
        NumberAnimation {
            properties: "opacity"
            from: 0
            to: 1
            duration: 500
        }
    }
}
    \endqml

*/
/*!
    \qmlproperty Transition Row::move
    This property holds the transition to apply when moving an item within the positioner.

    This can happen when other items are added or removed from the positioner, or when items resize themselves.

    \qml
Row {
    id: positioner
    move: Transition {
        NumberAnimation {
            properties: "x"
            ease: "easeOutBounce"
        }
    }
}
    \endqml

*/
/*!
  \qmlproperty int Row::spacing

  spacing is the amount in pixels left empty between each adjacent
  item, and defaults to 0.

  The below example places a Grid containing a red, a blue and a
  green rectangle on a gray background. The area the grid positioner
  occupies is colored white. The top positioner has the default of no spacing,
  and the bottom positioner has its spacing set to 2.

  \image spacing_a.png
  \image spacing_b.png

*/
/*!
    \internal
    \class QmlGraphicsRow
    \brief The QmlGraphicsRow class lines up items horizontally.
    \ingroup group_positioners
*/
QmlGraphicsRow::QmlGraphicsRow(QmlGraphicsItem *parent)
: QmlGraphicsBasePositioner(Horizontal, parent)
{
}

void QmlGraphicsRow::doPositioning()
{
    int hoffset = 0;

    foreach(QmlGraphicsItem* item, *leavingItems()){
        if (remove()){
            QList<QPair<QString,QVariant> > changes;
            applyRemove(changes, item);
        }
    }
    QList<QGraphicsItem *> children = childItems();
    for (int ii = 0; ii < children.count(); ++ii) {
        QmlGraphicsItem *child = qobject_cast<QmlGraphicsItem *>(children.at(ii));
        if (!child || child->opacity() == 0.0)
            continue;

        bool needMove = (child->x() != hoffset || child->y());

        QList<QPair<QString, QVariant> > changes;
        changes << qMakePair(QString(QLatin1String("x")),QVariant(hoffset));
        changes << qMakePair(QString(QLatin1String("y")),QVariant(0));
        if (needMove && items()->contains(child) && move()) {
            applyMove(changes,child);
        } else if (!items()->contains(child) && add()) {
            applyAdd(changes,child);
        } else if (needMove) {
            setMovingItem(child);
            child->setX(hoffset);
            setMovingItem(0);
        }
        if(child->width() && child->height()){//don't advance for invisible children
            hoffset += child->width();
            hoffset += spacing();
        }
    }
}

QML_DEFINE_TYPE(Qt,4,6,(QT_VERSION&0x00ff00)>>8,Grid,QmlGraphicsGrid)

/*!
  \qmlclass Grid QmlGraphicsGrid
  \brief The Grid item positions its children in a grid.
  \inherits Item

  The Grid item positions its child items so that they are
  aligned in a grid and are not overlapping. Spacing can be added
  between the items. It also provides for transitions to be set when items are
  added, moved, or removed in the positioner. Adding and removing apply
  both to items which are deleted or have their position in the
  document changed so as to no longer be children of the positioner, as
  well as to items which have their opacity set to or from zero so
  as to appear or disappear.

  The Grid defaults to using four columns, and as many rows as
  are necessary to fit all the child items. The number of rows
  and/or the number of columns can be constrained by setting the rows
  or columns properties. The grid positioner calculates a grid with
  rectangular cells of sufficient size to hold all items, and then
  places the items in the cells, going across then down, and
  positioning each item at the (0,0) corner of the cell. The below
  example demonstrates this.

  \table
  \row
  \o \image gridLayout_example.png
  \o
  \qml
Grid {
    columns: 3
    spacing: 2
    Rectangle { color: "red"; width: 50; height: 50 }
    Rectangle { color: "green"; width: 20; height: 50 }
    Rectangle { color: "blue"; width: 50; height: 20 }
    Rectangle { color: "cyan"; width: 50; height: 50 }
    Rectangle { color: "magenta"; width: 10; height: 10 }
}
  \endqml
  \endtable

  Note that the positioner assumes that the x and y positions of its children
  will not change. If you manually change the x or y properties in script, bind
  the x or y properties, or use anchors on a child of a positioner, then the
  positioner may exhibit strange behaviour.
*/
/*!
    \qmlproperty Transition Grid::remove
    This property holds the transition to apply when removing an item from the positioner.
    The transition is only applied to the removed item(s).

    Removed can mean that either the object has been deleted or
    reparented, and thus is now longer a child of the positioner, or that
    the object has had its opacity set to zero, and thus is no longer
    visible.

    Note that if the item counts as removed because its opacity is
    zero it will not be visible during the transition unless you set
    the opacity in the transition, like in the below example.

    \qml
Grid {
    remove: Transition {
        NumberAnimation {
            properties: "opacity"
            from: 1
            to: 0
            duration: 500
        }
    }
}
    \endqml

*/
/*!
    \qmlproperty Transition Grid::add
    This property holds the transition to apply when adding an item to the positioner.
    The transition is only applied to the added item(s).

    Added can mean that either the object has been created or
    reparented, and thus is now a child or the positioner, or that the
    object has had its opacity increased from zero, and thus is now
    visible.

    \qml
Grid {
    add: Transition {
        NumberAnimation {
            properties: "opacity"
            from: 0
            to: 1
            duration: 500
        }
    }
}
    \endqml

*/
/*!
    \qmlproperty Transition Grid::move
    This property holds the transition to apply when moving an item within the positioner.

    This can happen when other items are added or removed from the positioner, or
    when items resize themselves.

    \qml
Grid {
    move: Transition {
        NumberAnimation {
            properties: "x,y"
            ease: "easeOutBounce"
        }
    }
}
    \endqml

*/
/*!
  \qmlproperty int Grid::spacing

  spacing is the amount in pixels left empty between each adjacent
  item, and defaults to 0.

  The below example places a Grid containing a red, a blue and a
  green rectangle on a gray background. The area the grid positioner
  occupies is colored white. The top positioner has the default of no spacing,
  and the bottom positioner has its spacing set to 2.

  \image spacing_a.png
  \image spacing_b.png

*/
/*!
    \internal
    \class QmlGraphicsGrid
    \brief The QmlGraphicsGrid class lays out items in a grid.
    \ingroup group_layouts

*/
QmlGraphicsGrid::QmlGraphicsGrid(QmlGraphicsItem *parent) :
    QmlGraphicsBasePositioner(Both, parent)
{
    _columns=-1;
    _rows=-1;
}

/*!
    \qmlproperty int Grid::columns
    This property holds the number of columns in the grid.

    When the columns property is set the Grid will always have
    that many columns. Note that if you do not have enough items to
    fill this many columns some columns will be of zero width.
*/

/*!
    \qmlproperty int Grid::rows
    This property holds the number of rows in the grid.

    When the rows property is set the Grid will always have that
    many rows. Note that if you do not have enough items to fill this
    many rows some rows will be of zero width.
*/

void QmlGraphicsGrid::doPositioning()
{
    int c=_columns,r=_rows;//Actual number of rows/columns
    int numVisible = items()->size() + newItems()->size();
    if (_columns==-1 && _rows==-1){
        c = 4;
        r = (numVisible+3)/4;
    }else if (_rows==-1){
        r = (numVisible+(_columns-1))/_columns;
    }else if (_columns==-1){
        c = (numVisible+(_rows-1))/_rows;
    }

    QList<int> maxColWidth;
    QList<int> maxRowHeight;
    int childIndex =0;
    QList<QGraphicsItem *> children = childItems();
    for (int i=0; i<r; i++){
        for (int j=0; j<c; j++){
            if (j==0)
                maxRowHeight << 0;
            if (i==0)
                maxColWidth << 0;

            if (childIndex == children.count())
                continue;
            QmlGraphicsItem *child = qobject_cast<QmlGraphicsItem *>(children.at(childIndex++));
            if (!child || child->opacity() == 0.0)
                continue;
            if (child->width() > maxColWidth[j])
                maxColWidth[j] = child->width();
            if (child->height() > maxRowHeight[i])
                maxRowHeight[i] = child->height();
        }
    }

    int xoffset=0;
    int yoffset=0;
    int curRow =0;
    int curCol =0;
    foreach(QmlGraphicsItem* item, *leavingItems()){
        if (remove()){
            QList<QPair<QString,QVariant> > changes;
            applyRemove(changes, item);
        }
    }
    foreach(QGraphicsItem* schild, children){
        QmlGraphicsItem *child = qobject_cast<QmlGraphicsItem *>(schild);
        if (!child || child->opacity() == 0.0)
            continue;
        bool needMove = (child->x()!=xoffset)||(child->y()!=yoffset);
        QList<QPair<QString, QVariant> > changes;
        changes << qMakePair(QString(QLatin1String("x")),QVariant(xoffset));
        changes << qMakePair(QString(QLatin1String("y")),QVariant(yoffset));
        if (newItems()->contains(child) && add()) {
            applyAdd(changes,child);
        } else if (needMove) {
            if (move()){
                applyMove(changes,child);
            }else{
                setMovingItem(child);
                child->setPos(QPointF(xoffset, yoffset));
                setMovingItem(0);
            }
        }
        xoffset+=maxColWidth[curCol]+spacing();
        curCol++;
        curCol%=c;
        if (!curCol){
            yoffset+=maxRowHeight[curRow]+spacing();
            xoffset=0;
            curRow++;
            if (curRow>=r)
                break;
        }
    }
}

QT_END_NAMESPACE
