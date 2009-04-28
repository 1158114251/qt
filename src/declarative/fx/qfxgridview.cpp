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

#include "qfxvisualitemmodel.h"
#include "qlistmodelinterface.h"
#include "qmlfollow.h"
#include "private/qfxflickable_p.h"
#include "qfxgridview.h"

QT_BEGIN_NAMESPACE

class QFxGridViewAttached : public QObject
{
    Q_OBJECT
public:
    QFxGridViewAttached(QObject *parent)
        : QObject(parent), m_isCurrent(false), m_delayRemove(false) {}
    ~QFxGridViewAttached() {
        attachedProperties.remove(parent());
    }

    Q_PROPERTY(QFxGridView *view READ view);
    QFxGridView *view() { return m_view; }

    Q_PROPERTY(bool isCurrentItem READ isCurrentItem NOTIFY currentItemChanged);
    bool isCurrentItem() const { return m_isCurrent; }
    void setIsCurrentItem(bool c) {
        if (m_isCurrent != c) {
            m_isCurrent = c;
            emit currentItemChanged();
        }
    }

    Q_PROPERTY(bool delayRemove READ delayRemove WRITE setDelayRemove NOTIFY delayRemoveChanged);
    bool delayRemove() const { return m_delayRemove; }
    void setDelayRemove(bool delay) {
        if (m_delayRemove != delay) {
            m_delayRemove = delay;
            emit delayRemoveChanged();
        }
    }

    static QFxGridViewAttached *properties(QObject *obj) {
        QFxGridViewAttached *rv = attachedProperties.value(obj);
        if(!rv) {
            rv = new QFxGridViewAttached(obj);
            attachedProperties.insert(obj, rv);
        }
        return rv;
    }

    void emitAdd() { emit add(); }
    void emitRemove() { emit remove(); }

signals:
    void currentItemChanged();
    void delayRemoveChanged();
    void add();
    void remove();

public:
    QFxGridView *m_view;
    bool m_isCurrent;
    bool m_delayRemove;

    static QHash<QObject*, QFxGridViewAttached*> attachedProperties;
};

QHash<QObject*, QFxGridViewAttached*> QFxGridViewAttached::attachedProperties;


//----------------------------------------------------------------------------

class FxGridItem
{
public:
    FxGridItem(QFxItem *i, QFxGridView *v) : item(i), view(v) {
        attached = QFxGridViewAttached::properties(item);
        attached->m_view = view;
    }
    ~FxGridItem() {}

    qreal rowPos() const { return (view->flow() == QFxGridView::LeftToRight ? item->y() : item->x()); }
    qreal colPos() const { return (view->flow() == QFxGridView::LeftToRight ? item->x() : item->y()); }
    qreal endRowPos() const {
        return (view->flow() == QFxGridView::LeftToRight
                                        ? item->y() + (item->height() > 0 ? item->height() : 1)
                                        : item->x() + (item->width() > 0 ? item->width() : 1)) - 1;
    }
    void setPosition(qreal col, qreal row) {
        if (view->flow() == QFxGridView::LeftToRight) {
            item->setPos(QPointF(col, row));
        } else {
            item->setPos(QPointF(row, col));
        }
    }

    QFxItem *item;
    QFxGridView *view;
    QFxGridViewAttached *attached;
    int index;
};

//----------------------------------------------------------------------------

class QFxGridViewPrivate : public QFxFlickablePrivate
{
    Q_DECLARE_PUBLIC(QFxGridView);

public:
    QFxGridViewPrivate()
    : model(0), currentItem(0), tmpCurrent(0), flow(QFxGridView::LeftToRight)
    , visiblePos(0), visibleIndex(0) , currentIndex(-1)
    , cellWidth(100), cellHeight(100), columns(1)
    , highlightComponent(0), highlight(0), trackedItem(0)
    , moveReason(Other), buffer(0), highlightXAnimator(0), highlightYAnimator(0)
    , keyPressed(false), ownModel(false), wrap(false), autoHighlight(true)
    , fixCurrentVisibility(false) {}

    void init();
    void clear();
    FxGridItem *getItem(int modelIndex);
    FxGridItem *createItem(int modelIndex);
    void releaseItem(FxGridItem *item);
    void refill(qreal from, qreal to);

    void updateGrid();
    void layout(bool removed=false);
    void updateTrackedItem();
    void createHighlight();
    void updateHighlight();
    void updateCurrent(int modelIndex);

    FxGridItem *visibleItem(int modelIndex) const {
        if (modelIndex >= visibleIndex && modelIndex < visibleIndex + visibleItems.count()) {
            for (int i = modelIndex - visibleIndex; i < visibleItems.count(); ++i) {
                FxGridItem *item = visibleItems.at(i);
                if (item->index == modelIndex)
                    return item;
            }
        }
        return 0;
    }

    qreal position() const {
        Q_Q(const QFxGridView);
        return flow == QFxGridView::LeftToRight ? q->yPosition() : q->xPosition();
    }
    void setPosition(qreal pos) {
        Q_Q(QFxGridView);
        if (flow == QFxGridView::LeftToRight)
            q->setYPosition(pos);
        else
            q->setXPosition(pos);
    }
    int size() const {
        Q_Q(const QFxGridView);
        return flow == QFxGridView::LeftToRight ? q->height() : q->width();
    }
    qreal startPosition() const {
        qreal pos = 0;
        if (!visibleItems.isEmpty())
            pos = visibleItems.first()->rowPos() - visibleIndex / columns * rowSize();
        return pos;
    }

    qreal endPosition() const {
        qreal pos = 0;
        if (model && model->count())
            pos = rowPosAt(model->count() - 1) + rowSize();
        return pos;
    }

    bool isValid() const {
        return model && model->count() && (!ownModel || model->delegate());
    }

    int rowSize() const {
        return flow == QFxGridView::LeftToRight ? cellHeight : cellWidth;
    }
    int colSize() const {
        return flow == QFxGridView::LeftToRight ? cellWidth : cellHeight;
    }

    qreal colPosAt(int modelIndex) const {
        if (FxGridItem *item = visibleItem(modelIndex))
            return item->colPos();
        if (!visibleItems.isEmpty()) {
            if (modelIndex < visibleIndex) {
                int count = (visibleIndex - modelIndex) % columns;
                int col = visibleItems.first()->colPos() / colSize();
                col = (columns - count + col) % columns;
                return col * colSize();
            } else {
                int count = columns - 1 - (modelIndex - visibleItems.last()->index - 1) % columns;
                return visibleItems.last()->colPos() - count * colSize();
            }
        }
        return 0;
    }
    qreal rowPosAt(int modelIndex) const {
        if (FxGridItem *item = visibleItem(modelIndex))
            return item->rowPos();
        if (!visibleItems.isEmpty()) {
            if (modelIndex < visibleIndex) {
                int firstCol = visibleItems.first()->colPos() / colSize();
                int col = visibleIndex - modelIndex + (columns - firstCol - 1);
                int rows = col / columns;
                return visibleItems.first()->rowPos() - rows * rowSize();
            } else {
                int count = modelIndex - visibleItems.last()->index;
                int col = visibleItems.last()->colPos() + count * colSize();
                int rows = col / (columns * colSize());
                return visibleItems.last()->rowPos() + rows * rowSize();
            }
        }
        return 0;
    }

    // Map a model index to visibleItems list index.
    // These may differ if removed items are still present in the visible list,
    // e.g. doing a removal animation
    int mapFromModel(int modelIndex) const {
        if (modelIndex < visibleIndex || modelIndex >= visibleIndex + visibleItems.count())
            return -1;
        for (int i = 0; i < visibleItems.count(); ++i) {
            FxGridItem *listItem = visibleItems.at(i);
            if (listItem->index == modelIndex)
                return i + visibleIndex;
            if (listItem->index > modelIndex)
                return -1;
        }
        return -1; // Not in visibleList
    }

    // for debugging only
    void checkVisible() const {
        int skip = 0;
        for (int i = 0; i < visibleItems.count(); ++i) {
            FxGridItem *listItem = visibleItems.at(i);
            if (listItem->index == -1) {
                ++skip;
            } else if (listItem->index != visibleIndex + i - skip) {
                qDebug() << "index" << visibleIndex << i << listItem->index;
                for (int j = 0; j < visibleItems.count(); j++)
                    qDebug() << " index" << j << "item index" << visibleItems.at(j)->index;
                abort();
            }
        }
    }

    QFxVisualItemModel *model;
    QVariant modelVariant;
    QList<FxGridItem*> visibleItems;
    FxGridItem *currentItem;
    QFxItem *tmpCurrent;
    QFxGridView::Flow flow;
    int visiblePos;
    int visibleIndex;
    int currentIndex;
    int cellWidth;
    int cellHeight;
    int columns;
    QmlComponent *highlightComponent;
    FxGridItem *highlight;
    FxGridItem *trackedItem;
    enum MovementReason { Other, Key, Mouse };
    MovementReason moveReason;
    int buffer;
    QmlFollow *highlightXAnimator;
    QmlFollow *highlightYAnimator;

    int keyPressed : 1;
    int ownModel : 1;
    int wrap : 1;
    int autoHighlight : 1;
    int fixCurrentVisibility : 1;
};

void QFxGridViewPrivate::init()
{
    Q_Q(QFxGridView);
    q->setOptions(QFxGridView::IsFocusRealm);
}

void QFxGridViewPrivate::clear()
{
    for (int i = 0; i < visibleItems.count(); ++i)
        releaseItem(visibleItems.at(i));
    visibleItems.clear();
    visiblePos = 0;
    visibleIndex = 0;
    if (currentItem) {
        FxGridItem *tmpItem = currentItem;
        currentItem = 0;
        currentIndex = -1;
        releaseItem(tmpItem);
    }
    createHighlight();
    trackedItem = 0;
}

FxGridItem *QFxGridViewPrivate::getItem(int modelIndex)
{
    if (currentItem && modelIndex == currentIndex)
        return currentItem;
    if (FxGridItem *listItem = visibleItem(modelIndex))
        return listItem;
    return createItem(modelIndex);
}

FxGridItem *QFxGridViewPrivate::createItem(int modelIndex)
{
    Q_Q(QFxGridView);
    // create object
    FxGridItem *listItem = 0;
    if (QFxItem *item = model->item(modelIndex, false)) {
        listItem = new FxGridItem(item, q);
        listItem->index = modelIndex;
        // complete
        model->completeItem();
        listItem->item->setZ(modelIndex + 1);
        listItem->item->setParent(q->viewport());
    }
    return listItem;
}


void QFxGridViewPrivate::releaseItem(FxGridItem *item)
{
    Q_Q(QFxGridView);
    if (item != currentItem) {
        if (trackedItem == item) {
            QObject::disconnect(trackedItem->item, SIGNAL(topChanged()), q, SLOT(trackedPositionChanged()));
            QObject::disconnect(trackedItem->item, SIGNAL(leftChanged()), q, SLOT(trackedPositionChanged()));
            trackedItem = 0;
        }
        model->release(item->item);
        delete item;
    }
}

void QFxGridViewPrivate::refill(qreal from, qreal to)
{
    Q_Q(QFxGridView);
    if (!isValid() || !q->isComponentComplete())
        return;

    from -= buffer;
    to += buffer;
    bool changed = false;

    int colPos = 0;
    int rowPos = 0;
    int modelIndex = 0;
    if (visibleItems.count()) {
        rowPos = visibleItems.last()->rowPos();
        colPos = visibleItems.last()->colPos() + colSize();
        if (colPos > colSize() * (columns-1)) {
            colPos = 0;
            rowPos += rowSize();
        }
        int i = visibleItems.count() - 1;
        while (i > 0 && visibleItems.at(i)->index == -1)
            --i;
        modelIndex = visibleItems.at(i)->index + 1;
    }

    FxGridItem *item = 0;
    while (modelIndex < model->count() && rowPos <= to) {
        //qDebug() << "refill: append item" << modelIndex;
        item = getItem(modelIndex);
        item->setPosition(colPos, rowPos);
        visibleItems.append(item);
        colPos += colSize();
        if (colPos > colSize() * (columns-1)) {
            colPos = 0;
            rowPos += rowSize();
        }
        ++modelIndex;
        changed = true;
    }

    if (visibleItems.count()) {
        rowPos = visibleItems.first()->rowPos();
        colPos = visibleItems.first()->colPos() - colSize();
        if (colPos < 0) {
            colPos = colSize() * (columns - 1);
            rowPos -= rowSize();
        }
    }
    while (visibleIndex > 0 && rowPos + rowSize() - 1 >= from){
        //qDebug() << "refill: prepend item" << visibleIndex-1 << "top pos" << rowPos << colPos;
        item = getItem(visibleIndex-1);
        --visibleIndex;
        item->setPosition(colPos, rowPos);
        visibleItems.prepend(item);
        colPos -= colSize();
        if (colPos < 0) {
            colPos = colSize() * (columns - 1);
            rowPos -= rowSize();
        }
        changed = true;
    }

    while (visibleItems.count() > 1 && (item = visibleItems.first()) && item->endRowPos() < from) {
        if (item->attached->delayRemove())
            break;
        //qDebug() << "refill: remove first" << visibleIndex << "top end pos" << item->endRowPos();
        if (item->index != -1)
            visibleIndex++;
        visibleItems.removeFirst();
        releaseItem(item);
        changed = true;
    }
    while (visibleItems.count() > 1 && (item = visibleItems.last()) && item->rowPos() > to) {
        if (item->attached->delayRemove())
            break;
        //qDebug() << "refill: remove last" << visibleIndex+visibleItems.count()-1;
        visibleItems.removeLast();
        releaseItem(item);
        changed = true;
    }
    if (changed) {
        if (flow == QFxGridView::LeftToRight)
            q->setViewportHeight(endPosition() - startPosition());
        else
            q->setViewportWidth(endPosition() - startPosition());
    }
}

void QFxGridViewPrivate::updateGrid()
{
    Q_Q(QFxGridView);
    columns = (int)qMax((flow == QFxGridView::LeftToRight ? q->width() : q->height()) / colSize(), 1.);
    if (isValid()) {
        if (flow == QFxGridView::LeftToRight)
            q->setViewportHeight(endPosition() - startPosition());
        else
            q->setViewportWidth(endPosition() - startPosition());
    }
}

void QFxGridViewPrivate::layout(bool removed)
{
    Q_Q(QFxGridView);
    if (visibleItems.count()) {
        qreal rowPos = visibleItems.first()->rowPos();
        qreal colPos = visibleItems.first()->colPos();
        if (visibleIndex % columns != 0) {
            if (removed)
                rowPos -= rowSize();
            colPos = (visibleIndex % columns) * colSize();
            visibleItems.first()->setPosition(colPos, rowPos);
        } else if (colPos != 0) {
            colPos = 0;
            visibleItems.first()->setPosition(colPos, rowPos);
        }
        for (int i = 1; i < visibleItems.count(); ++i) {
            FxGridItem *item = visibleItems.at(i);
            colPos += colSize();
            if (colPos > colSize() * (columns-1)) {
                colPos = 0;
                rowPos += rowSize();
            }
            item->setPosition(colPos, rowPos);
        }
    }
    q->refill();
    q->trackedPositionChanged();
    updateHighlight();
    if (flow == QFxGridView::LeftToRight) {
        q->setViewportHeight(endPosition() - startPosition());
        fixupY();
    } else {
        q->setViewportWidth(endPosition() - startPosition());
        fixupX();
    }
}

void QFxGridViewPrivate::updateTrackedItem()
{
    Q_Q(QFxGridView);
    FxGridItem *item = currentItem;
    if (highlight)
        item = highlight;

    if (trackedItem && item != trackedItem) {
        QObject::disconnect(trackedItem->item, SIGNAL(topChanged()), q, SLOT(trackedPositionChanged()));
        QObject::disconnect(trackedItem->item, SIGNAL(leftChanged()), q, SLOT(trackedPositionChanged()));
        trackedItem = 0;
    }

    if (!trackedItem && item) {
        trackedItem = item;
        QObject::connect(trackedItem->item, SIGNAL(topChanged()), q, SLOT(trackedPositionChanged()));
        QObject::connect(trackedItem->item, SIGNAL(leftChanged()), q, SLOT(trackedPositionChanged()));
        q->trackedPositionChanged();
    }
    if (trackedItem)
        q->trackedPositionChanged();
}

void QFxGridViewPrivate::createHighlight()
{
    Q_Q(QFxGridView);
    if (highlight) {
        if (trackedItem == highlight)
            trackedItem = 0;
        delete highlight->item;
        delete highlight;
        highlight = 0;
        delete highlightXAnimator;
        delete highlightYAnimator;
        highlightXAnimator = 0;
        highlightYAnimator = 0;
    }

    if (!highlightComponent)
        return;

    if (currentItem) {
        QmlContext *highlightContext = new QmlContext(qmlContext(q));
        QObject *nobj = highlightComponent->create(highlightContext);
        if (nobj) {
            highlightContext->setParent(nobj);
            QFxItem *item = qobject_cast<QFxItem *>(nobj);
            if (item) {
                item->setParent(q->viewport());
                highlight = new FxGridItem(item, q);
                highlightXAnimator = new QmlFollow(q);
                highlightXAnimator->setTarget(QmlMetaProperty(highlight->item, QLatin1String("x")));
                highlightXAnimator->setSpring(3);
                highlightXAnimator->setDamping(0.3);
                highlightXAnimator->setEnabled(autoHighlight);
                highlightYAnimator = new QmlFollow(q);
                highlightYAnimator->setTarget(QmlMetaProperty(highlight->item, QLatin1String("y")));
                highlightYAnimator->setSpring(3);
                highlightYAnimator->setDamping(0.3);
                highlightYAnimator->setEnabled(autoHighlight);
            } else {
                delete highlightContext;
            }
        }
    }
}

void QFxGridViewPrivate::updateHighlight()
{
    if ((!currentItem && highlight) || (currentItem && !highlight))
        createHighlight();
    updateTrackedItem();
    if (currentItem && autoHighlight && highlight) {
        // auto-update highlight
        highlightXAnimator->setSourceValue(currentItem->item->x());
        highlightYAnimator->setSourceValue(currentItem->item->y());
        highlight->item->setWidth(currentItem->item->width());
        highlight->item->setHeight(currentItem->item->height());
    }
}

void QFxGridViewPrivate::updateCurrent(int modelIndex)
{
    Q_Q(QFxGridView);
    if (!isValid() || modelIndex < 0 || modelIndex >= model->count()) {
        if (currentItem) {
            FxGridItem *item = currentItem;
            currentItem = 0;
            currentIndex = 0;
            updateHighlight();
            releaseItem(item);
            emit q->currentIndexChanged();
        }
        return;
    }

    if (currentItem && currentIndex == modelIndex) {
        updateHighlight();
        return;
    }

    if (tmpCurrent) {
        delete tmpCurrent;
        tmpCurrent = 0;
    }
    int oldCurrentIndex = currentIndex;
    FxGridItem *oldCurrentItem = currentItem;
    currentIndex = -1;
    currentItem = visibleItem(modelIndex);
    if (!currentItem) {
        currentItem = getItem(modelIndex);
        currentItem->setPosition(colPosAt(modelIndex), rowPosAt(modelIndex));
    }
    currentIndex = modelIndex;
    fixCurrentVisibility = true;
    if (oldCurrentItem && oldCurrentItem->item != currentItem->item)
        oldCurrentItem->attached->setIsCurrentItem(false);
    currentItem->item->setFocus(true);
    currentItem->attached->setIsCurrentItem(true);
    updateHighlight();
    emit q->currentIndexChanged();
    // Release the old current item
    if (oldCurrentItem && !visibleItem(oldCurrentIndex))
        releaseItem(oldCurrentItem);
}

//----------------------------------------------------------------------------

/*!
    \qmlclass GridView
    \inherits Flickable
    \brief The GridView element provides a grid view of items provided by a model.

    The model is typically provided by a QAbstractListModel "C++ model object",
    but can also be created directly in XML.

    The items are laid out top to bottom (vertically) or left to right (horizontally)
    and may be flicked to scroll.

    The below example creates a very simple grid, using an XML model.
    \code
    <resources>
        <ListModel id="contactModel">
            <Contact>
                <firstName>John</firstName>
                <lastName>Smith</lastName>
            </Contact>
            <Contact>
                <firstName>Bill</firstName>
                <lastName>Jones</lastName>
            </Contact>
            <Contact>
                <firstName>Jane</firstName>
                <lastName>Doe</lastName>
            </Contact>
            </ListModel>
            <Component id="contactDelegate">
                <Rect pen.color="blue" z="-1" height="20" width="80" color="white" radius="2">
                    <Text id="name" text="{firstName + ' ' + lastName}" font.size="11"/>
                </Rect>
        </Component>
    </resources>

    <GridView id="Grid" width="160" height="240" cellWidth="80" cellHeight="20" clip="true"
                model="{contactModel}" delegate="{contactDelegate}"/>
    \endcode
*/
QFxGridView::QFxGridView(QFxItem *parent)
    : QFxFlickable(*(new QFxGridViewPrivate), parent)
{
    Q_D(QFxGridView);
    d->init();
}

QFxGridView::~QFxGridView()
{
    Q_D(QFxGridView);
    if (d->ownModel)
        delete d->model;
}

/*!
  \qmlproperty model GridView::model
  This property holds the model providing data for the grid.

  The model provides a set of data that is used to create the items for the view.
  For large or dynamic datasets the model is usually provided by a C++ model object.
  The C++ model object must be a \l QListModelInterface subclass, a \l VisualModel,
  or a simple list.

  Models can also be created directly in XML, using the \l ListModel element. For example:
  \code
  <ListModel id="contactModel">
      <Contact>
          <firstName>John</firstName>
          <lastName>Smith</lastName>
      </Contact>
      <Contact>
          <firstName>Bill</firstName>
          <lastName>Jones</lastName>
      </Contact>
      <Contact>
          <firstName>Jane</firstName>
          <lastName>Doe</lastName>
      </Contact>
  </ListModel>

  <GridView model="{contactModel}" .../>
  \endcode
*/
QVariant QFxGridView::model() const
{
    Q_D(const QFxGridView);
    return d->modelVariant;
}

void QFxGridView::setModel(const QVariant &model)
{
    Q_D(QFxGridView);
    if (d->model) {
        disconnect(d->model, SIGNAL(itemsInserted(int,int)), this, SLOT(itemsInserted(int,int)));
        disconnect(d->model, SIGNAL(itemsRemoved(int,int)), this, SLOT(itemsRemoved(int,int)));
    }
    d->clear();
    d->modelVariant = model;
    QObject *object = qvariant_cast<QObject*>(model);
    QFxVisualItemModel *vim = 0;
    if (object && (vim = qobject_cast<QFxVisualItemModel *>(object))) {
        if (d->ownModel) {
            delete d->model;
            d->ownModel = false;
        }
        d->model = vim;
    } else {
        if (!d->ownModel) {
            d->model = new QFxVisualItemModel(qmlContext(this));
            d->ownModel = true;
        }
        d->model->setModel(model);
    }
    if (d->model) {
        if (d->currentIndex >= d->model->count() || d->currentIndex < 0)
            setCurrentIndex(0);
        else
            d->updateCurrent(d->currentIndex);
        connect(d->model, SIGNAL(itemsInserted(int,int)), this, SLOT(itemsInserted(int,int)));
        connect(d->model, SIGNAL(itemsRemoved(int,int)), this, SLOT(itemsRemoved(int,int)));
        refill();
        emit countChanged();
    }
}

/*!
  \qmlproperty component GridView::delegate

  The delegate provides a template describing what each item in the view should look and act like.

  Here is an example delegate:
  \code
  <Component id="contactDelegate">
      <Item id="wrapper">
          <Image id="pic" width="100" height="100" file="{portrait}"/>
          <Text id="name" text="{firstName + ' ' + lastName}"
              anchors.left="{pic.right}" anchors.leftMargin="5"/>
      </Item>
  </Component>
  ...
  <GridView delegate="{contactDelegate}" .../>
  \endcode
*/
QmlComponent *QFxGridView::delegate() const
{
    Q_D(const QFxGridView);
    return d->model ? d->model->delegate() : 0;
}

void QFxGridView::setDelegate(QmlComponent *delegate)
{
    Q_D(QFxGridView);
    if (!d->ownModel) {
        d->model = new QFxVisualItemModel(qmlContext(this));
        d->ownModel = true;
    }
    d->model->setDelegate(delegate);
    d->updateCurrent(d->currentIndex);
    refill();
}

/*!
  \qmlproperty int GridView::currentIndex
  \qmlproperty Item GridView::current

  \c currentIndex holds the index of the current item.
  \c current is the current item.  Note that the position of the current item
  may only be approximate until it becomes visible in the view.
*/
int QFxGridView::currentIndex() const
{
    Q_D(const QFxGridView);
    return d->currentIndex;
}

void QFxGridView::setCurrentIndex(int index)
{
    Q_D(QFxGridView);
    if (d->isValid() && index != d->currentIndex && index < d->model->count() && index >= 0)
        d->updateCurrent(index);
    else
        d->currentIndex = index;
}

QFxItem *QFxGridView::currentItem()
{
    Q_D(QFxGridView);
    if (!d->currentItem) {
        // Always return something valid
        if (!d->tmpCurrent) 
            d->tmpCurrent = new QFxItem(viewport());
        return d->tmpCurrent;
    }
    return d->currentItem->item;
}

/*!
  \qmlproperty int GridView::count
  This property holds the number of items in the view.
*/
int QFxGridView::count() const
{
    Q_D(const QFxGridView);
    if (d->model)
        return d->model->count();
    return 0;
}

/*!
  \qmlproperty component GridView::highlight
  This property holds the component to use as the highlight.

  An instance of the highlight component will be created for each view.
  The geometry of the resultant component instance will be managed by the view
  so as to stay with the current item, unless the autoHighlight property is false.

  The below example demonstrates how to make a simple highlight:
  \code
  <Component id="ListHighlight">
      <Rect color="lightsteelblue" radius="4"/>
  </Component>
  <GridView highlight="{ListHighlight}">
  \endcode

  \sa autoHighlight
*/
QmlComponent *QFxGridView::highlight() const
{
    Q_D(const QFxGridView);
    return d->highlightComponent;
}

void QFxGridView::setHighlight(QmlComponent *highlight)
{
    Q_D(QFxGridView);
    delete d->highlightComponent;
    d->highlightComponent = highlight;
    d->updateCurrent(d->currentIndex);
}

/*!
  \qmlproperty component GridView::autoHighlight
  This property sets whether the highlight is managed by the view.

  If autoHighlight is true, the highlight will be moved smoothly
  to follow the current item.  If autoHighlight is false, the
  highlight will not be moved by the view, and must be implemented
  by the highlight, for example:

  \code
  <Component id="Highlight">
      <Rect id="Wrapper" color="#242424" radius="4" width="320" height="60" >
          <y>
              <Follow source="{Wrapper.GridView.view.current.y}" spring="3" damping="0.2"/>
          </y>
          <x>
              <Follow source="{Wrapper.GridView.view.current.x}" spring="3" damping="0.2"/>
          </x>
      </Rect>
  </Component>
  \endcode
*/
bool QFxGridView::autoHighlight() const
{
    Q_D(const QFxGridView);
    return d->autoHighlight;
}

void QFxGridView::setAutoHighlight(bool autoHighlight)
{
    Q_D(QFxGridView);
    d->autoHighlight = autoHighlight;
    if (d->highlightXAnimator) {
        d->highlightXAnimator->setEnabled(d->autoHighlight);
        d->highlightYAnimator->setEnabled(d->autoHighlight);
    }
    d->updateHighlight();
}

/*!
  \qmlproperty enumeration GridView::flow
  This property holds the flow of the grid.

  Possible values are \c LeftToRight (default) and \c TopToBottom.

  If \a flow is \c LeftToRight, the view will scroll vertically.
  If \a flow is \c TopToBottom, the view will scroll horizontally.
*/
QFxGridView::Flow QFxGridView::flow() const
{
    Q_D(const QFxGridView);
    return d->flow;
}

void QFxGridView::setFlow(Flow flow)
{
    Q_D(QFxGridView);
    if (d->flow != flow) {
        d->flow = flow;
        if (d->flow == LeftToRight)
            setViewportWidth(-1);
        else
            setViewportHeight(-1);
        d->clear();
        d->updateGrid();
        refill();
        d->updateCurrent(d->currentIndex);
    }
}

/*!
  \qmlproperty bool GridView::wrap
  This property holds whether the grid wraps key navigation

  If this property is true then key presses to move off of one end of the grid will cause the
  selection to jump to the other side.
*/
bool QFxGridView::isWrapEnabled() const
{
    Q_D(const QFxGridView);
    return d->wrap;
}

void QFxGridView::setWrapEnabled(bool wrap)
{
    Q_D(QFxGridView);
    d->wrap = wrap;
}

/*!
  \qmlproperty int GridView::cacheBuffer
  This property holds the number of off-screen pixels to cache.

  This property determines the number of pixels above the top of the view
  and below the bottom of the view to cache.  Setting this value can make
  scrolling the view smoother at the expense of additional memory usage.
*/

/*!
  \property QFxGridView::cacheBuffer
  \brief sets the number of off-screen pixels to cache.

  This property determines the number of pixels above the top of the view
  and below the bottom of the view to cache.  Setting this value can make
  scrolling the view smoother at the expense of additional memory usage.
*/
int QFxGridView::cacheBuffer() const
{
    Q_D(const QFxGridView);
    return d->buffer;
}

void QFxGridView::setCacheBuffer(int buffer)
{
    Q_D(QFxGridView);
    if(d->buffer != buffer) {
        d->buffer = buffer;
        if (isComponentComplete())
            refill();
    }
}

/*!
  \qmlproperty int GridView::cellWidth
  This property holds the width of each cell in the grid

  The default cellWidth is 100.
*/
int QFxGridView::cellWidth() const
{
    Q_D(const QFxGridView);
    return d->cellWidth;
}

void QFxGridView::setCellWidth(int cellWidth)
{
    Q_D(QFxGridView);
    if (cellWidth != d->cellWidth && cellWidth > 0) {
        d->cellWidth = qMax(1, cellWidth);
        d->updateGrid();
        emit cellSizeChanged();
        d->layout();
    }
}

/*!
  \qmlproperty int GridView::cellHeight
  This property holds the height of each cell in the grid

  The default cellHeight is 100.
*/
int QFxGridView::cellHeight() const
{
    Q_D(const QFxGridView);
    return d->cellHeight;
}

void QFxGridView::setCellHeight(int cellHeight)
{
    Q_D(QFxGridView);
    if (cellHeight != d->cellHeight && cellHeight > 0) {
        d->cellHeight = qMax(1, cellHeight);
        d->updateGrid();
        emit cellSizeChanged();
        d->layout();
    }
}

/*!
  \reimp
*/
void QFxGridView::setHeight(int height)
{
    Q_D(QFxGridView);
    QFxFlickable::setHeight(height);
    if (isComponentComplete()) {
        d->updateGrid();
        d->layout();
    }
}

/*!
  \reimp
*/
void QFxGridView::setWidth(int width)
{
    Q_D(QFxGridView);
    QFxFlickable::setWidth(width);
    if (isComponentComplete()) {
        d->updateGrid();
        d->layout();
    }
}

/*!
  \reimp
*/
void QFxGridView::viewportMoved()
{
    QFxFlickable::viewportMoved();
    refill();
}

/*!
  \reimp
*/
qreal QFxGridView::minYExtent() const
{
    Q_D(const QFxGridView);
    if (d->flow == QFxGridView::TopToBottom)
        return QFxFlickable::minYExtent();
    return -d->startPosition();
}

/*!
  \reimp
*/
qreal QFxGridView::maxYExtent() const
{
    Q_D(const QFxGridView);
    if (d->flow == QFxGridView::TopToBottom)
        return QFxFlickable::maxYExtent();
    return -(d->endPosition() - height());
}

/*!
  \reimp
*/
qreal QFxGridView::minXExtent() const
{
    Q_D(const QFxGridView);
    if (d->flow == QFxGridView::LeftToRight)
        return QFxFlickable::minXExtent();
    return -d->startPosition();
}

/*!
  \reimp
*/
qreal QFxGridView::maxXExtent() const
{
    Q_D(const QFxGridView);
    if (d->flow == QFxGridView::LeftToRight)
        return QFxFlickable::maxXExtent();
    return -(d->endPosition() - height());
}

/*!
  \reimp
*/
void QFxGridView::keyPressEvent(QKeyEvent *event)
{
    Q_D(QFxGridView);
    if (d->model && d->model->count() && !d->locked) {
        if ((d->flow == QFxGridView::LeftToRight && event->key() == Qt::Key_Up)
            || (d->flow == QFxGridView::TopToBottom && event->key() == Qt::Key_Left)) {
            if (currentIndex() >= d->columns || d->wrap) {
                d->keyPressed = true;
                d->moveReason = QFxGridViewPrivate::Key;
                int index = currentIndex() - d->columns;
                setCurrentIndex(index >= 0 ? index : d->model->count()-1);
                event->accept();
            }
            return;
        } else if ((d->flow == QFxGridView::LeftToRight && event->key() == Qt::Key_Down)
                || (d->flow == QFxGridView::TopToBottom && event->key() == Qt::Key_Right)) {
            if (currentIndex() < d->model->count() - d->columns || d->wrap) {
                d->keyPressed = true;
                d->moveReason = QFxGridViewPrivate::Key;
                int index = currentIndex()+d->columns;
                setCurrentIndex(index < d->model->count() ? index : 0);
                event->accept();
            }
            return;
        } else if ((d->flow == QFxGridView::LeftToRight && event->key() == Qt::Key_Left)
                || (d->flow == QFxGridView::TopToBottom && event->key() == Qt::Key_Up)) {
            if (currentIndex() > 0 || d->wrap) {
                d->keyPressed = true;
                d->moveReason = QFxGridViewPrivate::Key;
                int index = currentIndex() - 1;
                setCurrentIndex(index >= 0 ? index : d->model->count()-1);
                event->accept();
            }
            return;
        } else if ((d->flow == QFxGridView::LeftToRight && event->key() == Qt::Key_Right)
                || (d->flow == QFxGridView::TopToBottom && event->key() == Qt::Key_Down)) {
            if (currentIndex() < d->model->count() - 1 || d->wrap) {
                d->keyPressed = true;
                d->moveReason = QFxGridViewPrivate::Key;
                int index = currentIndex() + 1;
                setCurrentIndex(index < d->model->count() ? index : 0);
                event->accept();
            }
            return;
        }
    }
    d->moveReason = QFxGridViewPrivate::Other;
    QFxFlickable::keyPressEvent(event);
}

/*!
  \reimp
*/
void QFxGridView::keyReleaseEvent(QKeyEvent *event)
{
    Q_D(QFxGridView);
    d->keyPressed = false;
    QFxFlickable::keyReleaseEvent(event);
}

/*!
  \reimp
*/
void QFxGridView::componentComplete()
{
    Q_D(QFxGridView);
    QFxFlickable::componentComplete();
    d->updateGrid();
    if (d->currentIndex < 0)
        d->updateCurrent(0);
    refill();
}

void QFxGridView::trackedPositionChanged()
{
    Q_D(QFxGridView);
    if (!d->trackedItem)
        return;
    if (!isFlicking() && !d->pressed && d->moveReason == QFxGridViewPrivate::Key) {
        if (d->trackedItem->rowPos() < d->position()) {
            d->setPosition(d->trackedItem->rowPos());
        } else if (d->trackedItem->endRowPos() > d->position() + d->size()) {
            qreal pos = d->trackedItem->endRowPos() - d->size();
            if (d->rowSize() > d->size())
                pos = d->trackedItem->rowPos();
            d->setPosition(pos);
        }
    }
}

void QFxGridView::itemsInserted(int modelIndex, int count)
{
    Q_D(QFxGridView);
    if (!d->visibleItems.count() || d->model->count() <= 1) {
        refill();
        d->updateCurrent(qMax(0, qMin(d->currentIndex, d->model->count()-1)));
        emit countChanged();
        return;
    }

    int index = d->mapFromModel(modelIndex);
    if (index == -1) {
        int i = d->visibleItems.count() - 1;
        while (i > 0 && d->visibleItems.at(i)->index == -1)
            --i;
        if (d->visibleItems.at(i)->index + 1 == modelIndex) {
            // Special case of appending an item to the model.
            index = d->visibleIndex + d->visibleItems.count();
        } else {
            if (modelIndex + count - 1 < d->visibleIndex) {
                // Insert before visible items
                d->visibleIndex += count;
                for (int i = 0; i < d->visibleItems.count(); ++i) {
                    FxGridItem *listItem = d->visibleItems.at(i);
                    if (listItem->index != -1 && listItem != d->currentItem)
                        listItem->index += count;
                }
            }
            if (d->currentIndex >= modelIndex) {
                // adjust current item index
                d->currentIndex += count;
                if (d->currentItem)
                    d->currentItem->index = d->currentIndex;
            }
            d->layout();
            emit countChanged();
            return;
        }
    }

    // At least some of the added items will be visible
    int insertCount = count;
    if (index < d->visibleIndex) {
        insertCount -= d->visibleIndex - index;
        index = d->visibleIndex;
        modelIndex = d->visibleIndex;
    }

    index -= d->visibleIndex;
    int to = d->buffer+d->position()+d->size()-1;
    int colPos, rowPos;
    if (index < d->visibleItems.count()) {
        colPos = d->visibleItems.at(index)->colPos();
        rowPos = d->visibleItems.at(index)->rowPos();
    } else {
        // appending items to visible list
        colPos = d->visibleItems.at(index-1)->colPos() + d->colSize();
        rowPos = d->visibleItems.at(index-1)->rowPos();
        if (colPos > d->colSize() * (d->columns-1)) {
            colPos = 0;
            rowPos += d->rowSize();
        }
    }

    QList<FxGridItem*> added;
    int i = 0;
    for (; i < insertCount && rowPos + d->rowSize() - 1 <= to; ++i) {
        int mod = (modelIndex+i) % d->columns;
        while (mod++ < d->columns && modelIndex + i < d->model->count() && i < insertCount) {
            FxGridItem *item = d->createItem(modelIndex + i);
            d->visibleItems.insert(index, item);
            item->setPosition(colPos, rowPos);
            added.append(item);
            colPos += d->colSize();
            if (colPos > d->colSize() * (d->columns-1)) {
                colPos = 0;
                rowPos += d->rowSize();
            }
            ++index;
            ++i;
        }
    }

    if (d->currentIndex >= modelIndex) {
        // adjust current item index
        d->currentIndex += count;
        if (d->currentItem) {
            d->currentItem->index = d->currentIndex;
            d->currentItem->setPosition(d->colPosAt(d->currentIndex), d->rowPosAt(d->currentIndex));
        }
    }
    if (i < insertCount) {
        // We didn't insert all our new items, which means anything
        // beyond the current index is not visible - remove it.
        while (d->visibleItems.count() > index)
            d->releaseItem(d->visibleItems.takeLast());
    } else {
        // Update the indexes of the following visible items.
        for (; index < d->visibleItems.count(); ++index) {
            FxGridItem *listItem = d->visibleItems.at(index);
            if (listItem != d->currentItem) {
                if (listItem->index != -1)
                    listItem->index += count;
            }
        }
    }
    // everything is in order now - emit add() signal
    foreach(FxGridItem *item, added)
        item->attached->emitAdd();
    d->layout();
    emit countChanged();
}

void QFxGridView::itemsRemoved(int modelIndex, int count)
{
    Q_D(QFxGridView);

    int index = d->mapFromModel(modelIndex);
    if (index == -1) {
        if (modelIndex + count - 1 < d->visibleIndex) {
            // Items removed before our visible items.
            d->visibleIndex -= count;
            for (int i = 0; i < d->visibleItems.count(); ++i) {
                FxGridItem *listItem = d->visibleItems.at(i);
                if (listItem->index != -1 && listItem != d->currentItem)
                    listItem->index -= count;
            }
        }
        if (d->currentIndex >= modelIndex + count) {
            d->currentIndex -= count;
            if (d->currentItem)
                d->currentItem->index -= count;
        } else if (d->currentIndex >= modelIndex && d->currentIndex < modelIndex + count) {
            // current item has been removed.
            if (d->currentItem) {
                FxGridItem *item = d->currentItem;
                d->currentItem = 0;
                d->releaseItem(item);
            }
            d->currentIndex = -1;
            d->updateCurrent(qMin(modelIndex, d->model->count()-1));
        }
        d->layout(true);
        emit countChanged();
        return;
    }

    // Remove the items from the visible list, skipping anything already marked for removal
    QList<FxGridItem*>::Iterator it = d->visibleItems.begin();
    while (it != d->visibleItems.end()) {
        FxGridItem *item = *it;
        if (item->index == -1 || item->index < modelIndex) {
            // already removed, or before removed items
            ++it;
        } else if (item->index >= modelIndex + count) {
            // after removed items
            if (item != d->currentItem)
                item->index -= count;
            ++it;
        } else {
            // removed item
            item->attached->emitRemove();
            if (item->attached->delayRemove()) {
                item->index = -1;
                connect(item->attached, SIGNAL(delayRemoveChanged()), this, SLOT(destroyRemoved()), Qt::QueuedConnection);
                ++it;
            } else {
                it = d->visibleItems.erase(it);
                d->releaseItem(item);
            }
        }
    }

    // fix current
    if (d->currentIndex >= modelIndex + count) {
        d->currentIndex -= count;
        if (d->currentItem)
            d->currentItem->index -= count;
    } else if (d->currentIndex >= modelIndex && d->currentIndex < modelIndex + count) {
        // current item has been removed.
        if (d->currentItem && !d->currentItem->attached->delayRemove()) {
            FxGridItem *item = d->currentItem;
            d->currentItem = 0;
            d->releaseItem(item);
        }
        d->currentItem = 0;
        d->currentIndex = -1;
        d->updateCurrent(qMin(modelIndex, d->model->count()-1));
    }

    // update visibleIndex
    for (it = d->visibleItems.begin(); it != d->visibleItems.end(); ++it) {
        if ((*it)->index != -1) {
            d->visibleIndex = (*it)->index;
            break;
        }
    }

    if (d->visibleItems.isEmpty()) {
        d->visibleIndex = 0;
        d->setPosition(0);
        refill();
    } else {
        // Correct the positioning of the items
        d->layout();
    }
    emit countChanged();
}

void QFxGridView::destroyRemoved()
{
    Q_D(QFxGridView);
    for (QList<FxGridItem*>::Iterator it = d->visibleItems.begin();
            it != d->visibleItems.end();) {
        FxGridItem *listItem = *it;
        if (listItem->index == -1 && listItem->attached->delayRemove() == false) {
            d->releaseItem(listItem);
            it = d->visibleItems.erase(it);
        } else {
            ++it;
        }
    }

    // Correct the positioning of the items
    d->layout();
}

void QFxGridView::refill()
{
    Q_D(QFxGridView);
    d->refill(d->position(), d->position()+d->size()-1);
}


QObject *QFxGridView::qmlAttachedProperties(QObject *obj)
{
    return QFxGridViewAttached::properties(obj);
}

QML_DEFINE_TYPE(QFxGridView,GridView);

QT_END_NAMESPACE

#include "qfxgridview.moc"
