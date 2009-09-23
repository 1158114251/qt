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

#include "qfxrepeater.h"
#include "qfxrepeater_p.h"
#include "qmllistaccessor.h"
#include "qfxvisualitemmodel.h"
#include <qlistmodelinterface.h>


QT_BEGIN_NAMESPACE
QFxRepeaterPrivate::QFxRepeaterPrivate()
: model(0), ownModel(false)
{
}

QFxRepeaterPrivate::~QFxRepeaterPrivate()
{
}

QML_DEFINE_TYPE(Qt,4,6,(QT_VERSION&0x00ff00)>>8,Repeater,QFxRepeater)

/*!
    \qmlclass Repeater
    \inherits Item

    \brief The Repeater item allows you to repeat a component based on a model.

    The Repeater item is used when you want to create a large number of
    similar items.  For each entry in the model, an item is instantiated
    in a context seeded with data from the model.  If the repeater will
    be instantiating a large number of instances, it may be more efficient to
    use one of Qt Declarative's \l {xmlViews}{view items}.

    The model may be either an object list, a string list, a number or a Qt model.
    In each case, the data element and the index is exposed to each instantiated
    component.  The index is always exposed as an accessible \c index property.
    In the case of an object or string list, the data element (of type string
    or object) is available as the \c modelData property.  In the case of a Qt model,
    all roles are available as named properties just like in the view classes.

    Items instantiated by the Repeater are inserted, in order, as
    children of the Repeater's parent.  The insertion starts immediately after
    the repeater's position in its parent stacking list.  This is to allow
    you to use a Repeater inside a layout.  The following QML example shows how
    the instantiated items would visually appear stacked between the red and
    blue rectangles.

    \snippet doc/src/snippets/declarative/repeater.qml 0

    \image repeater.png

    The repeater instance continues to own all items it instantiates, even
    if they are otherwise manipulated.  It is illegal to manually remove an item
    created by the Repeater.
 */

/*!
    \internal
    \class QFxRepeater
    \qmlclass Repeater

    XXX Repeater is very conservative in how it instatiates/deletes items.  Also
    new model entries will not be created and old ones will not be removed.
 */

/*!
    Create a new QFxRepeater instance.
 */
QFxRepeater::QFxRepeater(QFxItem *parent)
  : QFxItem(*(new QFxRepeaterPrivate), parent)
{
}

/*!
    \internal
 */
QFxRepeater::QFxRepeater(QFxRepeaterPrivate &dd, QFxItem *parent)
  : QFxItem(dd, parent)
{
}

/*!
    Destroy the repeater instance.  All items it instantiated are also
    destroyed.
 */
QFxRepeater::~QFxRepeater()
{
}

/*!
    \qmlproperty any Repeater::model

    The model providing data for the repeater.

    The model may be either an object list, a string list, a number or a Qt model.
    In each case, the data element and the index is exposed to each instantiated
    component.  The index is always exposed as an accessible \c index property.
    In the case of an object or string list, the data element (of type string
    or object) is available as the \c modelData property.  In the case of a Qt model,
    all roles are available as named properties just like in the view classes.

    As a special case the model can also be merely a number. In this case it will
    create that many instances of the component. They will also be assigned an index
    based on the order they are created.

    Models can also be created directly in QML, using a \l{ListModel} or \l{XmlListModel}.
*/
QVariant QFxRepeater::model() const
{
    Q_D(const QFxRepeater);
    return d->dataSource;
}

void QFxRepeater::setModel(const QVariant &model)
{
    Q_D(QFxRepeater);
    clear();
    /*
    if (d->model) {
        disconnect(d->model, SIGNAL(itemsInserted(int,int)), this, SLOT(itemsInserted(int,int)));
        disconnect(d->model, SIGNAL(itemsRemoved(int,int)), this, SLOT(itemsRemoved(int,int)));
        disconnect(d->model, SIGNAL(itemsMoved(int,int,int)), this, SLOT(itemsMoved(int,int,int)));
        disconnect(d->model, SIGNAL(createdItem(int, QFxItem*)), this, SLOT(createdItem(int,QFxItem*)));
        disconnect(d->model, SIGNAL(destroyingItem(QFxItem*)), this, SLOT(destroyingItem(QFxItem*)));
    }
    */
    d->dataSource = model;
    QObject *object = qvariant_cast<QObject*>(model);
    QFxVisualModel *vim = 0;
    if (object && (vim = qobject_cast<QFxVisualModel *>(object))) {
        if (d->ownModel) {
            delete d->model;
            d->ownModel = false;
        }
        d->model = vim;
    } else {
        if (!d->ownModel) {
            d->model = new QFxVisualDataModel(qmlContext(this));
            d->ownModel = true;
        }
        if (QFxVisualDataModel *dataModel = qobject_cast<QFxVisualDataModel*>(d->model))
            dataModel->setModel(model);
    }
    if (d->model) {
        /*
        connect(d->model, SIGNAL(itemsInserted(int,int)), this, SLOT(itemsInserted(int,int)));
        connect(d->model, SIGNAL(itemsRemoved(int,int)), this, SLOT(itemsRemoved(int,int)));
        connect(d->model, SIGNAL(itemsMoved(int,int,int)), this, SLOT(itemsMoved(int,int,int)));
        connect(d->model, SIGNAL(createdItem(int, QFxItem*)), this, SLOT(createdItem(int,QFxItem*)));
        connect(d->model, SIGNAL(destroyingItem(QFxItem*)), this, SLOT(destroyingItem(QFxItem*)));
        */
        regenerate();
        emit countChanged();
    }
}

/*!
    \qmlproperty Component Repeater::delegate
    \default

    The delegate provides a template describing what each item instantiated by the repeater should look and act like.
 */
QmlComponent *QFxRepeater::delegate() const
{
    Q_D(const QFxRepeater);
    if (d->model) {
        if (QFxVisualDataModel *dataModel = qobject_cast<QFxVisualDataModel*>(d->model))
            return dataModel->delegate();
    }

    return 0;
}

void QFxRepeater::setDelegate(QmlComponent *delegate)
{
    Q_D(QFxRepeater);
    if (!d->ownModel) {
        d->model = new QFxVisualDataModel(qmlContext(this));
        d->ownModel = true;
    }
    if (QFxVisualDataModel *dataModel = qobject_cast<QFxVisualDataModel*>(d->model)) {
        dataModel->setDelegate(delegate);
        regenerate();
    }
}

/*!
    \qmlproperty int Repeater::count

    This property holds the number of items in the repeater.
*/
int QFxRepeater::count() const
{
    Q_D(const QFxRepeater);
    if (d->model)
        return d->model->count();
    return 0;
}


/*!
    \internal
 */
void QFxRepeater::componentComplete()
{
    QFxItem::componentComplete();
    regenerate();
}

/*!
    \internal
 */
QVariant QFxRepeater::itemChange(GraphicsItemChange change,
                                       const QVariant &value)
{
    QVariant rv = QFxItem::itemChange(change, value);
    if (change == ItemParentHasChanged) {
        regenerate();
    }

    return rv;
}

void QFxRepeater::clear()
{
    Q_D(QFxRepeater);
    if (d->model) {
        foreach (QFxItem *item, d->deletables)
            d->model->release(item);
    }
    d->deletables.clear();
}

/*!
    \internal
 */
void QFxRepeater::regenerate()
{
    Q_D(QFxRepeater);

    clear();

    if (!d->model || !d->model->count() || !d->model->isValid() || !parentItem() || !isComponentComplete())
        return;

    //In order to do the insertion like the examples, we have to be at the
    //same point in the childItems() list. Temporary measure until we think of something better
    int pos = parentItem()->childItems().indexOf(this);
    Q_ASSERT(pos != -1);
    QList<QGraphicsItem*> otherChildren;
    for (int ii = pos+1; ii < parentItem()->childItems().count(); ii++){
        QGraphicsItem* otherChild = parentItem()->childItems()[ii];
        otherChildren << otherChild;
        otherChild->setParentItem(0);
    }

    for (int ii = 0; ii < count(); ++ii) {
        QFxItem *item = d->model->item(ii);
        if (item) {
            item->setParent(parentItem());
            d->deletables << item;
        }
    }

    foreach(QGraphicsItem* other, otherChildren)
        other->setParentItem(parentItem());
}
QT_END_NAMESPACE
