/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
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

#include "private/qmlgraphicsflickable_p_p.h"
#include <private/qmleasefollow_p.h>
#include <private/qlistmodelinterface_p.h>
#include "qmlgraphicsvisualitemmodel_p.h"
#include "qmlgraphicslistview_p.h"
#include <qmlexpression.h>

#include <QKeyEvent>

QT_BEGIN_NAMESPACE
class QmlGraphicsListViewAttached : public QObject
{
    Q_OBJECT
public:
    QmlGraphicsListViewAttached(QObject *parent)
        : QObject(parent), m_view(0), m_isCurrent(false), m_delayRemove(false) {}
    ~QmlGraphicsListViewAttached() {
        attachedProperties.remove(parent());
    }

    Q_PROPERTY(QmlGraphicsListView *view READ view CONSTANT)
    QmlGraphicsListView *view() { return m_view; }

    Q_PROPERTY(bool isCurrentItem READ isCurrentItem NOTIFY currentItemChanged)
    bool isCurrentItem() const { return m_isCurrent; }
    void setIsCurrentItem(bool c) {
        if (m_isCurrent != c) {
            m_isCurrent = c;
            emit currentItemChanged();
        }
    }

    Q_PROPERTY(QString prevSection READ prevSection NOTIFY prevSectionChanged)
    QString prevSection() const { return m_prevSection; }
    void setPrevSection(const QString &sect) {
        if (m_prevSection != sect) {
            m_prevSection = sect;
            emit prevSectionChanged();
        }
    }

    Q_PROPERTY(QString section READ section NOTIFY sectionChanged)
    QString section() const { return m_section; }
    void setSection(const QString &sect) {
        if (m_section != sect) {
            m_section = sect;
            emit sectionChanged();
        }
    }

    Q_PROPERTY(bool delayRemove READ delayRemove WRITE setDelayRemove NOTIFY delayRemoveChanged)
    bool delayRemove() const { return m_delayRemove; }
    void setDelayRemove(bool delay) {
        if (m_delayRemove != delay) {
            m_delayRemove = delay;
            emit delayRemoveChanged();
        }
    }

    static QmlGraphicsListViewAttached *properties(QObject *obj) {
        QmlGraphicsListViewAttached *rv = attachedProperties.value(obj);
        if (!rv) {
            rv = new QmlGraphicsListViewAttached(obj);
            attachedProperties.insert(obj, rv);
        }
        return rv;
    }

    void emitAdd() { emit add(); }
    void emitRemove() { emit remove(); }

Q_SIGNALS:
    void currentItemChanged();
    void sectionChanged();
    void prevSectionChanged();
    void delayRemoveChanged();
    void add();
    void remove();

public:
    QmlGraphicsListView *m_view;
    bool m_isCurrent;
    mutable QString m_section;
    QString m_prevSection;
    bool m_delayRemove;

    static QHash<QObject*, QmlGraphicsListViewAttached*> attachedProperties;
};

QHash<QObject*, QmlGraphicsListViewAttached*> QmlGraphicsListViewAttached::attachedProperties;

//----------------------------------------------------------------------------

class FxListItem
{
public:
    FxListItem(QmlGraphicsItem *i, QmlGraphicsListView *v) : item(i), view(v) {
        attached = QmlGraphicsListViewAttached::properties(item);
        attached->m_view = view;
    }
    ~FxListItem() {}

    qreal position() const { return (view->orientation() == QmlGraphicsListView::Vertical ? item->y() : item->x()); }
    int size() const { return (view->orientation() == QmlGraphicsListView::Vertical ? item->height() : item->width()); }
    qreal endPosition() const {
        return (view->orientation() == QmlGraphicsListView::Vertical
                                        ? item->y() + (item->height() > 0 ? item->height() : 1)
                                        : item->x() + (item->width() > 0 ? item->width() : 1)) - 1;
    }
    void setPosition(qreal pos) {
        if (view->orientation() == QmlGraphicsListView::Vertical) {
            item->setY(pos);
        } else {
            item->setX(pos);
        }
    }

    QmlGraphicsItem *item;
    QmlGraphicsListView *view;
    QmlGraphicsListViewAttached *attached;
    int index;
};

//----------------------------------------------------------------------------

class QmlGraphicsListViewPrivate : public QmlGraphicsFlickablePrivate
{
    Q_DECLARE_PUBLIC(QmlGraphicsListView)

public:
    QmlGraphicsListViewPrivate()
        : model(0), currentItem(0), orient(QmlGraphicsListView::Vertical)
        , visiblePos(0), visibleIndex(0)
        , averageSize(100.0), currentIndex(-1), requestedIndex(-1)
        , highlightRangeStart(0), highlightRangeEnd(0)
        , highlightComponent(0), highlight(0), trackedItem(0)
        , moveReason(Other), buffer(0), highlightPosAnimator(0), highlightSizeAnimator(0), spacing(0.0)
        , highlightMoveSpeed(400), highlightResizeSpeed(400), highlightRange(QmlGraphicsListView::NoHighlightRange)
        , ownModel(false), wrap(false), autoHighlight(true)
        , haveHighlightRange(false)
    {}

    void init();
    void clear();
    FxListItem *createItem(int modelIndex);
    void releaseItem(FxListItem *item);

    FxListItem *visibleItem(int modelIndex) const {
        if (modelIndex >= visibleIndex && modelIndex < visibleIndex + visibleItems.count()) {
            for (int i = modelIndex - visibleIndex; i < visibleItems.count(); ++i) {
                FxListItem *item = visibleItems.at(i);
                if (item->index == modelIndex)
                    return item;
            }
        }
        return 0;
    }

    qreal position() const {
        Q_Q(const QmlGraphicsListView);
        return orient == QmlGraphicsListView::Vertical ? q->viewportY() : q->viewportX();
    }
    void setPosition(qreal pos) {
        Q_Q(QmlGraphicsListView);
        if (orient == QmlGraphicsListView::Vertical)
            q->setViewportY(pos);
        else
            q->setViewportX(pos);
    }
    qreal size() const {
        Q_Q(const QmlGraphicsListView);
        return orient == QmlGraphicsListView::Vertical ? q->height() : q->width();
    }

    qreal startPosition() const {
        qreal pos = 0;
        if (!visibleItems.isEmpty()) {
            pos = visibleItems.first()->position();
            if (visibleIndex > 0)
                pos -= visibleIndex * (averageSize + spacing) - spacing;
        }
        return pos;
    }

    qreal endPosition() const {
        qreal pos = 0;
        if (!visibleItems.isEmpty()) {
            int invisibleCount = visibleItems.count() - visibleIndex;
            for (int i = visibleItems.count()-1; i >= 0; --i) {
                if (visibleItems.at(i)->index != -1) {
                    invisibleCount = model->count() - visibleItems.at(i)->index - 1;
                    break;
                }
            }
            pos = visibleItems.last()->endPosition() + invisibleCount * (averageSize + spacing);
        }
        return pos;
    }

    qreal positionAt(int modelIndex) const {
        if (FxListItem *item = visibleItem(modelIndex))
            return item->position();
        if (!visibleItems.isEmpty()) {
            if (modelIndex < visibleIndex) {
                int count = visibleIndex - modelIndex;
                return visibleItems.first()->position() - count * (averageSize + spacing);
            } else {
                int idx = visibleItems.count() - 1;
                while (idx >= 0 && visibleItems.at(idx)->index == -1)
                    --idx;
                if (idx < 0)
                    idx = visibleIndex;
                else
                    idx = visibleItems.at(idx)->index;
                int count = modelIndex - idx - 1;
                return visibleItems.last()->endPosition() + spacing + count * (averageSize + spacing) + 1;
            }
        }
        return 0;
    }

    QString sectionAt(int modelIndex) {
        Q_Q(QmlGraphicsListView);
        if (FxListItem *item = visibleItem(modelIndex))
            return item->attached->section();
        QString section;
        if (!sectionExpression.isEmpty())
            section = model->evaluate(modelIndex, sectionExpression, q).toString();
        return section;
    }

    bool isValid() const {
        return model && model->count() && model->isValid();
    }

    int snapIndex() {
        int index = currentIndex;
        for (int i = 0; i < visibleItems.count(); ++i) {
            FxListItem *item = visibleItems[i];
            if (item->index == -1)
                continue;
            qreal itemTop = item->position();
            if (itemTop >= highlight->position()-item->size()/2 && itemTop < highlight->position()+item->size()/2)
                return item->index;
        }
        return index;
    }

    //XXX Rough.  Only works for fixed size items.
    qreal snapPosAt(qreal pos) {
        return qRound((pos - startPosition()) / averageSize) * averageSize + startPosition();
    }

    int lastVisibleIndex() const {
        int lastIndex = -1;
        for (int i = visibleItems.count()-1; i >= 0; --i) {
            FxListItem *listItem = visibleItems.at(i);
            if (listItem->index != -1) {
                lastIndex = listItem->index;
                break;
            }
        }
        return lastIndex;
    }

    // map a model index to visibleItems index.
    // These may differ if removed items are still present in the visible list,
    // e.g. doing a removal animation
    int mapFromModel(int modelIndex) const {
        if (modelIndex < visibleIndex || modelIndex >= visibleIndex + visibleItems.count())
            return -1;
        for (int i = 0; i < visibleItems.count(); ++i) {
            FxListItem *listItem = visibleItems.at(i);
            if (listItem->index == modelIndex)
                return i + visibleIndex;
            if (listItem->index > modelIndex)
                return -1;
        }
        return -1; // Not in visibleList
    }

    bool mapRangeFromModel(int &index, int &count) const {
        if (index + count < visibleIndex)
            return false;

        int lastIndex = -1;
        for (int i = visibleItems.count()-1; i >= 0; --i) {
            FxListItem *listItem = visibleItems.at(i);
            if (listItem->index != -1) {
                lastIndex = listItem->index;
                break;
            }
        }

        if (index > lastIndex)
            return false;

        int last = qMin(index + count - 1, lastIndex);
        index = qMax(index, visibleIndex);
        count = last - index + 1;

        return true;
    }

    void updateViewport() {
        Q_Q(QmlGraphicsListView);
        if (orient == QmlGraphicsListView::Vertical)
            q->setViewportHeight(endPosition() - startPosition());
        else
            q->setViewportWidth(endPosition() - startPosition());
    }


    // for debugging only
    void checkVisible() const {
        int skip = 0;
        for (int i = 0; i < visibleItems.count(); ++i) {
            FxListItem *listItem = visibleItems.at(i);
            if (listItem->index == -1) {
                ++skip;
            } else if (listItem->index != visibleIndex + i - skip) {
                qFatal("index %d %d %d", visibleIndex, i, listItem->index);
            }
        }
    }

    void refill(qreal from, qreal to);
    void layout();
    void updateUnrequestedIndexes();
    void updateUnrequestedPositions();
    void updateTrackedItem();
    void createHighlight();
    void updateHighlight();
    void updateSections();
    void updateCurrentSection();
    void updateCurrent(int);
    void updateAverage();
    void fixupPosition();
    virtual void fixupY();
    virtual void fixupX();
    virtual void flickX(qreal velocity);
    virtual void flickY(qreal velocity);

    QmlGraphicsVisualModel *model;
    QVariant modelVariant;
    QList<FxListItem*> visibleItems;
    QHash<QmlGraphicsItem*,int> unrequestedItems;
    FxListItem *currentItem;
    QmlGraphicsListView::Orientation orient;
    int visiblePos;
    int visibleIndex;
    qreal averageSize;
    int currentIndex;
    int requestedIndex;
    qreal highlightRangeStart;
    qreal highlightRangeEnd;
    QmlComponent *highlightComponent;
    FxListItem *highlight;
    FxListItem *trackedItem;
    enum MovementReason { Other, Key, Mouse };
    MovementReason moveReason;
    int buffer;
    QmlEaseFollow *highlightPosAnimator;
    QmlEaseFollow *highlightSizeAnimator;
    QString sectionExpression;
    QString currentSection;
    qreal spacing;
    qreal highlightMoveSpeed;
    qreal highlightResizeSpeed;
    QmlGraphicsListView::HighlightRangeMode highlightRange;

    bool ownModel : 1;
    bool wrap : 1;
    bool autoHighlight : 1;
    bool haveHighlightRange : 1;
};

void QmlGraphicsListViewPrivate::init()
{
    Q_Q(QmlGraphicsListView);
    q->setFlag(QGraphicsItem::ItemIsFocusScope);
    QObject::connect(q, SIGNAL(heightChanged()), q, SLOT(refill()));
    QObject::connect(q, SIGNAL(widthChanged()), q, SLOT(refill()));
}

void QmlGraphicsListViewPrivate::clear()
{
    for (int i = 0; i < visibleItems.count(); ++i)
        releaseItem(visibleItems.at(i));
    visibleItems.clear();
    visiblePos = 0;
    visibleIndex = 0;
    releaseItem(currentItem);
    currentItem = 0;
    currentIndex = -1;
    createHighlight();
    trackedItem = 0;
}

FxListItem *QmlGraphicsListViewPrivate::createItem(int modelIndex)
{
    Q_Q(QmlGraphicsListView);
    // create object
    requestedIndex = modelIndex;
    FxListItem *listItem = 0;
    if (QmlGraphicsItem *item = model->item(modelIndex, false)) {
        listItem = new FxListItem(item, q);
        listItem->index = modelIndex;
        // initialise attached properties
        if (!sectionExpression.isEmpty()) {
            QmlExpression e(qmlContext(listItem->item), sectionExpression, q);
            e.setTrackChange(false);
            listItem->attached->m_section = e.value().toString();
            if (modelIndex > 0) {
                if (FxListItem *item = visibleItem(modelIndex-1))
                    listItem->attached->m_prevSection = item->attached->section();
                else
                    listItem->attached->m_prevSection = sectionAt(modelIndex-1);
            }
        }
        // complete
        model->completeItem();
        listItem->item->setZValue(1);
        listItem->item->setParent(q->viewport());
        if (orient == QmlGraphicsListView::Vertical)
            QObject::connect(listItem->item, SIGNAL(heightChanged()), q, SLOT(itemResized()));
        else
            QObject::connect(listItem->item, SIGNAL(widthChanged()), q, SLOT(itemResized()));
    }
    requestedIndex = -1;

    return listItem;
}

void QmlGraphicsListViewPrivate::releaseItem(FxListItem *item)
{
    Q_Q(QmlGraphicsListView);
    if (!item)
        return;
    if (trackedItem == item) {
        const char *notifier1 = orient == QmlGraphicsListView::Vertical ? SIGNAL(yChanged()) : SIGNAL(xChanged());
        const char *notifier2 = orient == QmlGraphicsListView::Vertical ? SIGNAL(heightChanged()) : SIGNAL(widthChanged());
        QObject::disconnect(trackedItem->item, notifier1, q, SLOT(trackedPositionChanged()));
        QObject::disconnect(trackedItem->item, notifier2, q, SLOT(trackedPositionChanged()));
        trackedItem = 0;
    }
    if (model->release(item->item) == 0) {
        // item was not destroyed, and we no longer reference it.
        unrequestedItems.insert(item->item, model->indexOf(item->item, q));
        if (orient == QmlGraphicsListView::Vertical)
            QObject::disconnect(item->item, SIGNAL(heightChanged()), q, SLOT(itemResized()));
        else
            QObject::disconnect(item->item, SIGNAL(widthChanged()), q, SLOT(itemResized()));
    }
    delete item;
}

void QmlGraphicsListViewPrivate::refill(qreal from, qreal to)
{
    Q_Q(QmlGraphicsListView);
    if (!isValid() || !q->isComponentComplete())
        return;
    from -= buffer;
    to += buffer;
    int modelIndex = 0;
    qreal itemEnd = visiblePos-1;
    if (!visibleItems.isEmpty()) {
        visiblePos = visibleItems.first()->position();
        itemEnd = visibleItems.last()->endPosition() + spacing;
        int i = visibleItems.count() - 1;
        while (i > 0 && visibleItems.at(i)->index == -1)
            --i;
        modelIndex = visibleItems.at(i)->index + 1;
    }

    bool changed = false;
    FxListItem *item = 0;
    int pos = itemEnd + 1;
    while (modelIndex < model->count() && pos <= to) {
        //qDebug() << "refill: append item" << modelIndex;
        if (!(item = createItem(modelIndex)))
            break;
        item->setPosition(pos);
        pos += item->size() + spacing;
        visibleItems.append(item);
        ++modelIndex;
        changed = true;
    }
    while (visibleIndex > 0 && visibleIndex <= model->count() && visiblePos > from) {
        //qDebug() << "refill: prepend item" << visibleIndex-1 << "current top pos" << visiblePos;
        if (!(item = createItem(visibleIndex-1)))
            break;
        --visibleIndex;
        visiblePos -= item->size() + spacing;
        item->setPosition(visiblePos);
        visibleItems.prepend(item);
        changed = true;
    }

    while (visibleItems.count() > 1 && (item = visibleItems.first()) && item->endPosition() < from) {
        if (item->attached->delayRemove())
            break;
        //qDebug() << "refill: remove first" << visibleIndex << "top end pos" << item->endPosition();
        if (item->index != -1)
            visibleIndex++;
        visibleItems.removeFirst();
        releaseItem(item);
        changed = true;
    }
    while (visibleItems.count() > 1 && (item = visibleItems.last()) && item->position() > to) {
        if (item->attached->delayRemove())
            break;
        //qDebug() << "refill: remove last" << visibleIndex+visibleItems.count()-1;
        visibleItems.removeLast();
        releaseItem(item);
        changed = true;
    }
    if (changed) {
        if (visibleItems.count())
            visiblePos = visibleItems.first()->position();
        updateAverage();
        if (!sectionExpression.isEmpty())
            updateCurrentSection();
        updateViewport();
    }
}

void QmlGraphicsListViewPrivate::layout()
{
    Q_Q(QmlGraphicsListView);
    if (!visibleItems.isEmpty()) {
        int oldEnd = visibleItems.last()->endPosition();
        int pos = visibleItems.first()->endPosition() + spacing + 1;
        for (int i=1; i < visibleItems.count(); ++i) {
            FxListItem *item = visibleItems.at(i);
            item->setPosition(pos);
            pos += item->size() + spacing;
        }
        // move current item if it is after the visible items.
        if (currentItem && currentIndex > lastVisibleIndex())
            currentItem->setPosition(currentItem->position() + (visibleItems.last()->endPosition() - oldEnd));
    }
    if (!isValid())
        return;
    q->refill();
    updateHighlight();
    fixupPosition();
    updateUnrequestedPositions();
    updateViewport();
}

void QmlGraphicsListViewPrivate::updateUnrequestedIndexes()
{
    Q_Q(QmlGraphicsListView);
    QHash<QmlGraphicsItem*,int>::iterator it;
    for (it = unrequestedItems.begin(); it != unrequestedItems.end(); ++it)
        *it = model->indexOf(it.key(), q);
}

void QmlGraphicsListViewPrivate::updateUnrequestedPositions()
{
    QHash<QmlGraphicsItem*,int>::const_iterator it;
    for (it = unrequestedItems.begin(); it != unrequestedItems.end(); ++it) {
        if (visibleItem(*it))
            continue;
        if (orient == QmlGraphicsListView::Vertical)
            it.key()->setY(positionAt(*it));
        else
            it.key()->setX(positionAt(*it));
    }
}

void QmlGraphicsListViewPrivate::updateTrackedItem()
{
    Q_Q(QmlGraphicsListView);
    FxListItem *item = currentItem;
    if (highlight)
        item = highlight;

    FxListItem *oldTracked = trackedItem;

    const char *notifier1 = orient == QmlGraphicsListView::Vertical ? SIGNAL(yChanged()) : SIGNAL(xChanged());
    const char *notifier2 = orient == QmlGraphicsListView::Vertical ? SIGNAL(heightChanged()) : SIGNAL(widthChanged());

    if (trackedItem && item != trackedItem) {
        QObject::disconnect(trackedItem->item, notifier1, q, SLOT(trackedPositionChanged()));
        QObject::disconnect(trackedItem->item, notifier2, q, SLOT(trackedPositionChanged()));
        trackedItem = 0;
    }

    if (!trackedItem && item) {
        trackedItem = item;
        QObject::connect(trackedItem->item, notifier1, q, SLOT(trackedPositionChanged()));
        QObject::connect(trackedItem->item, notifier2, q, SLOT(trackedPositionChanged()));
    }
    if (trackedItem && trackedItem != oldTracked)
        q->trackedPositionChanged();
}

void QmlGraphicsListViewPrivate::createHighlight()
{
    Q_Q(QmlGraphicsListView);
    if (highlight) {
        if (trackedItem == highlight)
            trackedItem = 0;
        delete highlight->item;
        delete highlight;
        highlight = 0;
        delete highlightPosAnimator;
        delete highlightSizeAnimator;
        highlightPosAnimator = 0;
        highlightSizeAnimator = 0;
    }

    if (currentItem) {
        QmlGraphicsItem *item = 0;
        if (highlightComponent) {
            QmlContext *highlightContext = new QmlContext(qmlContext(q));
            QObject *nobj = highlightComponent->create(highlightContext);
            if (nobj) {
                highlightContext->setParent(nobj);
                item = qobject_cast<QmlGraphicsItem *>(nobj);
                if (!item) {
                    delete nobj;
                } else {
                    item->setParent(q->viewport());
                }
            } else {
                delete highlightContext;
            }
        } else {
            item = new QmlGraphicsItem;
            item->setParent(q->viewport());
        }
        if (item) {
            item->setZValue(0);
            highlight = new FxListItem(item, q);
            if (orient == QmlGraphicsListView::Vertical)
                highlight->item->setHeight(currentItem->item->height());
            else
                highlight->item->setWidth(currentItem->item->width());
            const QLatin1String posProp(orient == QmlGraphicsListView::Vertical ? "y" : "x");
            highlightPosAnimator = new QmlEaseFollow(q);
            highlightPosAnimator->setTarget(QmlMetaProperty(highlight->item, posProp));
            highlightPosAnimator->setVelocity(highlightMoveSpeed);
            highlightPosAnimator->setEnabled(autoHighlight);
            const QLatin1String sizeProp(orient == QmlGraphicsListView::Vertical ? "height" : "width");
            highlightSizeAnimator = new QmlEaseFollow(q);
            highlightSizeAnimator->setVelocity(highlightResizeSpeed);
            highlightSizeAnimator->setTarget(QmlMetaProperty(highlight->item, sizeProp));
            highlightSizeAnimator->setEnabled(autoHighlight);
        }
    }
}

void QmlGraphicsListViewPrivate::updateHighlight()
{
    if ((!currentItem && highlight) || (currentItem && !highlight))
        createHighlight();
    if (currentItem && autoHighlight && highlight && !moving) {
        // auto-update highlight
        highlightPosAnimator->setSourceValue(currentItem->position());
        highlightSizeAnimator->setSourceValue(currentItem->size());
        if (orient == QmlGraphicsListView::Vertical) {
            if (highlight->item->width() == 0)
                highlight->item->setWidth(currentItem->item->width());
        } else {
            if (highlight->item->height() == 0)
                highlight->item->setHeight(currentItem->item->height());
        }
    }
    updateTrackedItem();
}

void QmlGraphicsListViewPrivate::updateSections()
{
    if (!sectionExpression.isEmpty()) {
        QString prevSection;
        if (visibleIndex > 0)
            prevSection = sectionAt(visibleIndex-1);
        for (int i = 0; i < visibleItems.count(); ++i) {
            if (visibleItems.at(i)->index != -1) {
                QmlGraphicsListViewAttached *attached = visibleItems.at(i)->attached;
                attached->setPrevSection(prevSection);
                prevSection = attached->section();
            }
        }
    }
}

void QmlGraphicsListViewPrivate::updateCurrentSection()
{
    if (sectionExpression.isEmpty() || visibleItems.isEmpty()) {
        currentSection = QString();
        return;
    }
    int index = 0;
    while (visibleItems.at(index)->endPosition() < position() && index < visibleItems.count())
        ++index;

    if (index < visibleItems.count())
        currentSection = visibleItems.at(index)->attached->section();
    else
        currentSection = visibleItems.first()->attached->section();
}

void QmlGraphicsListViewPrivate::updateCurrent(int modelIndex)
{
    Q_Q(QmlGraphicsListView);
    if (!isValid() || modelIndex < 0 || modelIndex >= model->count()) {
        if (currentItem) {
            currentItem->attached->setIsCurrentItem(false);
            releaseItem(currentItem);
            currentItem = 0;
            currentIndex = -1;
            updateHighlight();
            emit q->currentIndexChanged();
        }
        return;
    }

    if (currentItem && currentIndex == modelIndex) {
        updateHighlight();
        return;
    }
    FxListItem *oldCurrentItem = currentItem;
    currentIndex = modelIndex;
    currentItem = createItem(modelIndex);
    if (oldCurrentItem && (!currentItem || oldCurrentItem->item != currentItem->item))
        oldCurrentItem->attached->setIsCurrentItem(false);
    if (currentItem) {
        if (modelIndex == visibleIndex - 1) {
            // We can calculate exact postion in this case
            currentItem->setPosition(visibleItems.first()->position() - currentItem->size() - spacing);
        } else {
            // Create current item now and position as best we can.
            // Its position will be corrected when it becomes visible.
            currentItem->setPosition(positionAt(modelIndex));
        }
        currentItem->item->setFocus(true);
        currentItem->attached->setIsCurrentItem(true);
    }
    updateHighlight();
    emit q->currentIndexChanged();
    // Release the old current item
    releaseItem(oldCurrentItem);
}

void QmlGraphicsListViewPrivate::updateAverage()
{
    if (!visibleItems.count())
        return;
    qreal sum = 0.0;
    for (int i = 0; i < visibleItems.count(); ++i)
        sum += visibleItems.at(i)->size();
    averageSize = sum / visibleItems.count();
}

void QmlGraphicsListViewPrivate::fixupPosition()
{
    if (orient == QmlGraphicsListView::Vertical)
        fixupY();
    else
        fixupX();
}

void QmlGraphicsListViewPrivate::fixupY()
{
    QmlGraphicsFlickablePrivate::fixupY();
    if (orient == QmlGraphicsListView::Horizontal)
        return;

    if (haveHighlightRange && highlightRange == QmlGraphicsListView::StrictlyEnforceRange) {
        if (currentItem && highlight && currentItem->position() != highlight->position()) {
            moveReason = Mouse;
            timeline.clear();
            timeline.move(_moveY, -(currentItem->position() - highlightRangeStart), QEasingCurve(QEasingCurve::InOutQuad), 200);
        }
    }
}

void QmlGraphicsListViewPrivate::fixupX()
{
    QmlGraphicsFlickablePrivate::fixupX();
    if (orient == QmlGraphicsListView::Vertical)
        return;

    if (haveHighlightRange && highlightRange == QmlGraphicsListView::StrictlyEnforceRange) {
        if (currentItem && highlight && currentItem->position() != highlight->position()) {
            moveReason = Mouse;
            timeline.clear();
            timeline.move(_moveX, -(currentItem->position() - highlightRangeStart), QEasingCurve(QEasingCurve::InOutQuad), 200);
        }
    }
}

void QmlGraphicsListViewPrivate::flickX(qreal velocity)
{
    Q_Q(QmlGraphicsListView);

    if (!haveHighlightRange || highlightRange != QmlGraphicsListView::StrictlyEnforceRange) {
        QmlGraphicsFlickablePrivate::flickX(velocity);
        return;
    }

    qreal maxDistance = -1;
    // -ve velocity means list is moving up
    if (velocity > 0) {
        if (_moveX.value() < q->minXExtent())
            maxDistance = qAbs(q->minXExtent() -_moveX.value() + (overShoot?30:0));
        flickTargetX = q->minXExtent();
    } else {
        if (_moveX.value() > q->maxXExtent())
            maxDistance = qAbs(q->maxXExtent() - _moveX.value()) + (overShoot?30:0);
        flickTargetX = q->maxXExtent();
    }
    if (maxDistance > 0) {
        qreal v = velocity;
        if (maxVelocity != -1 && maxVelocity < qAbs(v)) {
            if (v < 0)
                v = -maxVelocity;
            else
                v = maxVelocity;
        }
        qreal accel = deceleration;
        qreal v2 = v * v;
        qreal maxAccel = v2 / (2.0f * maxDistance);
        if (maxAccel < accel) {
            // If we are not flicking to the end then attempt to stop exactly on an item boundary
            qreal dist = v2 / accel / 2.0;
            if (v > 0)
                dist = -dist;
            dist = -_moveX.value() - snapPosAt(-(_moveX.value() - highlightRangeStart) + dist) + highlightRangeStart;
            if (v < 0 && dist >= 0 || v > 0 && dist <= 0) {
                timeline.reset(_moveX);
                fixupX();
                return;
            }
            accel = v2 / (2.0f * qAbs(dist));
        }
        timeline.reset(_moveX);
        timeline.accel(_moveX, v, accel, maxDistance);
        timeline.execute(fixupXEvent);
        if (!flicked) {
            flicked = true;
            emit q->flickingChanged();
            emit q->flickStarted();
        }
    } else {
        timeline.reset(_moveX);
        fixupX();
    }
}

void QmlGraphicsListViewPrivate::flickY(qreal velocity)
{
    Q_Q(QmlGraphicsListView);

    if (!haveHighlightRange || highlightRange != QmlGraphicsListView::StrictlyEnforceRange) {
        QmlGraphicsFlickablePrivate::flickY(velocity);
        return;
    }

    qreal maxDistance = -1;
    // -ve velocity means list is moving up
    if (velocity > 0) {
        if (_moveY.value() < q->minYExtent())
            maxDistance = qAbs(q->minYExtent() -_moveY.value() + (overShoot?30:0));
        flickTargetY = q->minYExtent();
    } else {
        if (_moveY.value() > q->maxYExtent())
            maxDistance = qAbs(q->maxYExtent() - _moveY.value()) + (overShoot?30:0);
        flickTargetY = q->maxYExtent();
    }
    if (maxDistance > 0) {
        qreal v = velocity;
        if (maxVelocity != -1 && maxVelocity < qAbs(v)) {
            if (v < 0)
                v = -maxVelocity;
            else
                v = maxVelocity;
        }
        qreal accel = deceleration;
        qreal v2 = v * v;
        qreal maxAccel = v2 / (2.0f * maxDistance);
        if (maxAccel < accel) {
            // If we are not flicking to the end then attempt to stop exactly on an item boundary
            qreal dist = v2 / accel / 2.0;
            if (v > 0)
                dist = -dist;
            dist = -_moveY.value() - snapPosAt(-(_moveY.value() - highlightRangeStart) + dist) + highlightRangeStart;
            if (v < 0 && dist >= 0 || v > 0 && dist <= 0) {
                timeline.reset(_moveY);
                fixupY();
                return;
            }
            accel = v2 / (2.0f * qAbs(dist));
        }
        timeline.reset(_moveY);
        timeline.accel(_moveY, v, accel, maxDistance);
        timeline.execute(fixupYEvent);
        if (!flicked) {
            flicked = true;
            emit q->flickingChanged();
            emit q->flickStarted();
        }
    } else {
        timeline.reset(_moveY);
        fixupY();
    }
}

//----------------------------------------------------------------------------

/*!
    \qmlclass ListView QmlGraphicsListView
    \inherits Flickable
    \brief The ListView item provides a list view of items provided by a model.

    The model is typically provided by a QAbstractListModel "C++ model object",
    but can also be created directly in QML. The items are laid out vertically
    or horizontally and may be flicked to scroll.

    The below example creates a very simple vertical list, using a QML model.
    \image trivialListView.png

    The user interface defines a delegate to display an item, a highlight,
    and the ListView which uses the above.

    \snippet doc/src/snippets/declarative/listview/listview.qml 3

    The model is defined as a ListModel using QML:
    \quotefile doc/src/snippets/declarative/listview/dummydata/ContactModel.qml

    In this case ListModel is a handy way for us to test our UI.  In practice
    the model would be implemented in C++, or perhaps via a SQL data source.
*/

QmlGraphicsListView::QmlGraphicsListView(QmlGraphicsItem *parent)
    : QmlGraphicsFlickable(*(new QmlGraphicsListViewPrivate), parent)
{
    Q_D(QmlGraphicsListView);
    d->init();
}

QmlGraphicsListView::~QmlGraphicsListView()
{
    Q_D(QmlGraphicsListView);
    d->clear();
    if (d->ownModel)
        delete d->model;
}

/*!
    \qmlattachedproperty bool ListView::isCurrentItem
    This attached property is true if this delegate is the current item; otherwise false.

    It is attached to each instance of the delegate.

    This property may be used to adjust the appearance of the current item, for example:

    \snippet doc/src/snippets/declarative/listview/highlight.qml 0
*/

/*!
    \qmlattachedproperty ListView ListView::view
    This attached property holds the view that manages this delegate instance.

    It is attached to each instance of the delegate.
*/

/*!
    \qmlattachedproperty string ListView::prevSection
    This attached property holds the section of the previous element.

    It is attached to each instance of the delegate.

    The section is evaluated using the \l {ListView::sectionExpression}{sectionExpression} property.
*/

/*!
    \qmlattachedproperty string ListView::section
    This attached property holds the section of this element.

    It is attached to each instance of the delegate.

    The section is evaluated using the \l {ListView::sectionExpression}{sectionExpression} property.
*/

/*!
    \qmlattachedproperty bool ListView::delayRemove
    This attached property holds whether the delegate may be destroyed.

    It is attached to each instance of the delegate.

    It is sometimes necessary to delay the destruction of an item
    until an animation completes.

    The example below ensures that the animation completes before
    the item is removed from the list.

    \code
    Component {
        id: myDelegate
        Item {
            id: wrapper
            ListView.onRemove: SequentialAnimation {
                PropertyAction { target: wrapper.ListView; property: "delayRemove"; value: true }
                NumberAnimation { target: wrapper; property: "scale"; to: 0; duration: 250; easing: "easeInOutQuad" }
                PropertyAction { target: wrapper.ListView; property: "delayRemove"; value: false }
            }
        }
    }
    \endcode
*/

/*!
    \qmlattachedsignal ListView::onAdd()
    This attached handler is called immediately after an item is added to the view.
*/

/*!
    \qmlattachedsignal ListView::onRemove()
    This attached handler is called immediately before an item is removed from the view.
*/

/*!
    \qmlproperty model ListView::model
    This property holds the model providing data for the list.

    The model provides a set of data that is used to create the items
    for the view.  For large or dynamic datasets the model is usually
    provided by a C++ model object.  The C++ model object must be a \l
    {QAbstractItemModel} subclass or a simple list.

    Models can also be created directly in QML, using a \l{ListModel},
    \l{XmlListModel} or \l{VisualItemModel}.

    \sa {qmlmodels}{Data Models}
*/
QVariant QmlGraphicsListView::model() const
{
    Q_D(const QmlGraphicsListView);
    return d->modelVariant;
}

void QmlGraphicsListView::setModel(const QVariant &model)
{
    Q_D(QmlGraphicsListView);
    if (d->model) {
        disconnect(d->model, SIGNAL(itemsInserted(int,int)), this, SLOT(itemsInserted(int,int)));
        disconnect(d->model, SIGNAL(itemsRemoved(int,int)), this, SLOT(itemsRemoved(int,int)));
        disconnect(d->model, SIGNAL(itemsMoved(int,int,int)), this, SLOT(itemsMoved(int,int,int)));
        disconnect(d->model, SIGNAL(createdItem(int, QmlGraphicsItem*)), this, SLOT(createdItem(int,QmlGraphicsItem*)));
        disconnect(d->model, SIGNAL(destroyingItem(QmlGraphicsItem*)), this, SLOT(destroyingItem(QmlGraphicsItem*)));
    }
    d->clear();
    d->modelVariant = model;
    QObject *object = qvariant_cast<QObject*>(model);
    QmlGraphicsVisualModel *vim = 0;
    if (object && (vim = qobject_cast<QmlGraphicsVisualModel *>(object))) {
        if (d->ownModel) {
            delete d->model;
            d->ownModel = false;
        }
        d->model = vim;
    } else {
        if (!d->ownModel) {
            d->model = new QmlGraphicsVisualDataModel(qmlContext(this));
            d->ownModel = true;
        }
        if (QmlGraphicsVisualDataModel *dataModel = qobject_cast<QmlGraphicsVisualDataModel*>(d->model))
            dataModel->setModel(model);
    }
    if (d->model) {
        if (d->currentIndex >= d->model->count() || d->currentIndex < 0)
            setCurrentIndex(0);
        else
            d->updateCurrent(d->currentIndex);
        connect(d->model, SIGNAL(itemsInserted(int,int)), this, SLOT(itemsInserted(int,int)));
        connect(d->model, SIGNAL(itemsRemoved(int,int)), this, SLOT(itemsRemoved(int,int)));
        connect(d->model, SIGNAL(itemsMoved(int,int,int)), this, SLOT(itemsMoved(int,int,int)));
        connect(d->model, SIGNAL(createdItem(int, QmlGraphicsItem*)), this, SLOT(createdItem(int,QmlGraphicsItem*)));
        connect(d->model, SIGNAL(destroyingItem(QmlGraphicsItem*)), this, SLOT(destroyingItem(QmlGraphicsItem*)));
        refill();
        emit countChanged();
    }
}

/*!
    \qmlproperty component ListView::delegate

    The delegate provides a template describing what each item in the view should look and act like.

    Here is an example delegate:
    \snippet doc/src/snippets/declarative/listview/listview.qml 0
*/
QmlComponent *QmlGraphicsListView::delegate() const
{
    Q_D(const QmlGraphicsListView);
    if (d->model) {
        if (QmlGraphicsVisualDataModel *dataModel = qobject_cast<QmlGraphicsVisualDataModel*>(d->model))
            return dataModel->delegate();
    }

    return 0;
}

void QmlGraphicsListView::setDelegate(QmlComponent *delegate)
{
    Q_D(QmlGraphicsListView);
    if (!d->ownModel) {
        d->model = new QmlGraphicsVisualDataModel(qmlContext(this));
        d->ownModel = true;
    }
    if (QmlGraphicsVisualDataModel *dataModel = qobject_cast<QmlGraphicsVisualDataModel*>(d->model)) {
        dataModel->setDelegate(delegate);
        d->updateCurrent(d->currentIndex);
        refill();
    }
}

/*!
    \qmlproperty int ListView::currentIndex
    \qmlproperty Item ListView::currentItem

    \c currentIndex holds the index of the current item.
    \c currentItem is the current item.  Note that the position of the current item
    may only be approximate until it becomes visible in the view.
*/
int QmlGraphicsListView::currentIndex() const
{
    Q_D(const QmlGraphicsListView);
    return d->currentIndex;
}

void QmlGraphicsListView::setCurrentIndex(int index)
{
    Q_D(QmlGraphicsListView);
    d->moveReason = QmlGraphicsListViewPrivate::Other;
    if (d->isValid() && index != d->currentIndex && index < d->model->count() && index >= 0) {
        cancelFlick();
        d->updateCurrent(index);
    } else {
        d->currentIndex = index;
    }
}

QmlGraphicsItem *QmlGraphicsListView::currentItem()
{
    Q_D(QmlGraphicsListView);
    if (!d->currentItem)
        return 0;
    return d->currentItem->item;
}

/*!
  \qmlproperty int ListView::count
  This property holds the number of items in the view.
*/
int QmlGraphicsListView::count() const
{
    Q_D(const QmlGraphicsListView);
    if (d->model)
        return d->model->count();
    return 0;
}

/*!
    \qmlproperty component ListView::highlight
    This property holds the component to use as the highlight.

    An instance of the highlight component will be created for each list.
    The geometry of the resultant component instance will be managed by the list
    so as to stay with the current item, unless the highlightFollowsCurrentItem
    property is false.

    The below example demonstrates how to make a simple highlight
    for a vertical list.

    \snippet doc/src/snippets/declarative/listview/listview.qml 1
    \image trivialListView.png

    \sa highlightFollowsCurrentItem
*/
QmlComponent *QmlGraphicsListView::highlight() const
{
    Q_D(const QmlGraphicsListView);
    return d->highlightComponent;
}

void QmlGraphicsListView::setHighlight(QmlComponent *highlight)
{
    Q_D(QmlGraphicsListView);
    delete d->highlightComponent;
    d->highlightComponent = highlight;
    d->updateCurrent(d->currentIndex);
}

/*!
    \qmlproperty bool ListView::highlightFollowsCurrentItem
    This property holds whether the highlight is managed by the view.

    If highlightFollowsCurrentItem is true, the highlight will be moved smoothly
    to follow the current item.  If highlightFollowsCurrentItem is false, the
    highlight will not be moved by the view, and must be implemented
    by the highlight.  The following example creates a highlight with
    its motion defined by the spring \l {SpringFollow}:

    \snippet doc/src/snippets/declarative/listview/highlight.qml 1

    Note that the highlight animation also affects the way that the view
    is scrolled.  This is because the view moves to maintain the
    highlight within the preferred highlight range (or visible viewport).

    \sa highlight
*/
bool QmlGraphicsListView::highlightFollowsCurrentItem() const
{
    Q_D(const QmlGraphicsListView);
    return d->autoHighlight;
}

void QmlGraphicsListView::setHighlightFollowsCurrentItem(bool autoHighlight)
{
    Q_D(QmlGraphicsListView);
    d->autoHighlight = autoHighlight;
    if (d->highlightPosAnimator) {
        d->highlightPosAnimator->setEnabled(d->autoHighlight);
        d->highlightSizeAnimator->setEnabled(d->autoHighlight);
    }
    d->updateHighlight();
}

/*!
    \qmlproperty real ListView::preferredHighlightBegin
    \qmlproperty real ListView::preferredHighlightEnd
    \qmlproperty bool ListView::highlightRangeMode

    These properties set the preferred range of the highlight (current item)
    within the view.

    If highlightRangeMode is set to \e ApplyRange the view will
    attempt to maintain the highlight within the range, however
    the highlight can move outside of the range at the ends of the list
    or due to a mouse interaction.

    If highlightRangeMode is set to \e StrictlyEnforceRange the highlight will never
    move outside of the range.  This means that the current item will change
    if a keyboard or mouse action would cause the highlight to move
    outside of the range.

    The default value is \e NoHighlightRange.

    Note that a valid range requires preferredHighlightEnd to be greater
    than or equal to preferredHighlightBegin.
*/
qreal QmlGraphicsListView::preferredHighlightBegin() const
{
    Q_D(const QmlGraphicsListView);
    return d->highlightRangeStart;
}

void QmlGraphicsListView::setPreferredHighlightBegin(qreal start)
{
    Q_D(QmlGraphicsListView);
    d->highlightRangeStart = start;
    d->haveHighlightRange = d->highlightRange != NoHighlightRange && d->highlightRangeStart <= d->highlightRangeEnd;
}

qreal QmlGraphicsListView::preferredHighlightEnd() const
{
    Q_D(const QmlGraphicsListView);
    return d->highlightRangeEnd;
}

void QmlGraphicsListView::setPreferredHighlightEnd(qreal end)
{
    Q_D(QmlGraphicsListView);
    d->highlightRangeEnd = end;
    d->haveHighlightRange = d->highlightRange != NoHighlightRange && d->highlightRangeStart <= d->highlightRangeEnd;
}

QmlGraphicsListView::HighlightRangeMode QmlGraphicsListView::highlightRangeMode() const
{
    Q_D(const QmlGraphicsListView);
    return d->highlightRange;
}

void QmlGraphicsListView::setHighlightRangeMode(HighlightRangeMode mode)
{
    Q_D(QmlGraphicsListView);
    d->highlightRange = mode;
    d->haveHighlightRange = d->highlightRange != NoHighlightRange && d->highlightRangeStart <= d->highlightRangeEnd;
}

/*!
    \qmlproperty real ListView::spacing

    This property holds the spacing to leave between items.
*/
qreal QmlGraphicsListView::spacing() const
{
    Q_D(const QmlGraphicsListView);
    return d->spacing;
}

void QmlGraphicsListView::setSpacing(qreal spacing)
{
    Q_D(QmlGraphicsListView);
    if (spacing != d->spacing) {
        d->spacing = spacing;
        d->layout();
        emit spacingChanged();
    }
}

/*!
    \qmlproperty enumeration ListView::orientation
    This property holds the orientation of the list.

    Possible values are \c Vertical (default) and \c Horizontal.

    Vertical Example:
    \image trivialListView.png
    Horizontal Example:
    \image ListViewHorizontal.png
*/
QmlGraphicsListView::Orientation QmlGraphicsListView::orientation() const
{
    Q_D(const QmlGraphicsListView);
    return d->orient;
}

void QmlGraphicsListView::setOrientation(QmlGraphicsListView::Orientation orientation)
{
    Q_D(QmlGraphicsListView);
    if (d->orient != orientation) {
        d->orient = orientation;
        if (d->orient == QmlGraphicsListView::Vertical)
            setViewportWidth(-1);
        else
            setViewportHeight(-1);
        d->clear();
        refill();
        emit orientationChanged();
        d->updateCurrent(d->currentIndex);
    }
}

/*!
    \qmlproperty bool ListView::keyNavigationWraps
    This property holds whether the list wraps key navigation

    If this property is true then key presses to move off of one end of the list will cause the
    current item to jump to the other end.
*/
bool QmlGraphicsListView::isWrapEnabled() const
{
    Q_D(const QmlGraphicsListView);
    return d->wrap;
}

void QmlGraphicsListView::setWrapEnabled(bool wrap)
{
    Q_D(QmlGraphicsListView);
    d->wrap = wrap;
}

/*!
    \qmlproperty int ListView::cacheBuffer
    This property holds the number of off-screen pixels to cache.

    This property determines the number of pixels above the top of the list
    and below the bottom of the list to cache.  Setting this value can make
    scrolling the list smoother at the expense of additional memory usage.
*/
int QmlGraphicsListView::cacheBuffer() const
{
    Q_D(const QmlGraphicsListView);
    return d->buffer;
}

void QmlGraphicsListView::setCacheBuffer(int b)
{
    Q_D(QmlGraphicsListView);
    if (d->buffer != b) {
        d->buffer = b;
        if (isComponentComplete())
            refill();
    }
}

/*!
    \qmlproperty string ListView::sectionExpression
    This property holds the expression to be evaluated for the section attached property.

    Each item in the list has attached properties named \c ListView.section and
    \c ListView.prevSection.  These may be used to place a section header for
    related items.  The example below assumes that the model is sorted by size of
    pet.  The section expression is the size property.  If \c ListView.section and
    \c ListView.prevSection differ, the item will display a section header.

    \snippet examples/declarative/listview/sections.qml 0

    \image ListViewSections.png
*/
QString QmlGraphicsListView::sectionExpression() const
{
    Q_D(const QmlGraphicsListView);
    return d->sectionExpression;
}

void QmlGraphicsListView::setSectionExpression(const QString &expression)
{
    Q_D(QmlGraphicsListView);
    if (d->sectionExpression != expression) {
        d->sectionExpression = expression;
        emit sectionExpressionChanged();
    }
}

QString QmlGraphicsListView::currentSection() const
{
    Q_D(const QmlGraphicsListView);
    return d->currentSection;
}

/*!
    \qmlproperty real ListView::highlightMoveSpeed

    This property holds the moving animation speed of the highlight delegate.
*/
qreal QmlGraphicsListView::highlightMoveSpeed() const
{
    Q_D(const QmlGraphicsListView);\
    return d->highlightMoveSpeed;
}

void QmlGraphicsListView::setHighlightMoveSpeed(qreal speed)
{
    Q_D(QmlGraphicsListView);\
    if (d->highlightMoveSpeed != speed)
    {
        d->highlightMoveSpeed = speed;
        emit highlightMoveSpeedChanged();
    }
}

/*!
    \qmlproperty real ListView::highlightResizeSpeed

    This property holds the resizing animation speed of the highlight delegate.
*/
qreal QmlGraphicsListView::highlightResizeSpeed() const
{
    Q_D(const QmlGraphicsListView);\
    return d->highlightResizeSpeed;
}

void QmlGraphicsListView::setHighlightResizeSpeed(qreal speed)
{
    Q_D(QmlGraphicsListView);\
    if (d->highlightResizeSpeed != speed)
    {
        d->highlightResizeSpeed = speed;
        emit highlightResizeSpeedChanged();
    }
}

void QmlGraphicsListView::viewportMoved()
{
    Q_D(QmlGraphicsListView);
    QmlGraphicsFlickable::viewportMoved();
    refill();
    if (isFlicking() || d->moving)
        d->moveReason = QmlGraphicsListViewPrivate::Mouse;
    if (d->moveReason == QmlGraphicsListViewPrivate::Mouse) {
        if (d->haveHighlightRange && d->highlightRange == StrictlyEnforceRange && d->highlight) {
            int idx = d->snapIndex();
            if (idx >= 0 && idx != d->currentIndex)
                d->updateCurrent(idx);

            qreal pos = d->currentItem->position();
            if (pos > d->position() + d->highlightRangeEnd - 1 - d->highlight->size())
                pos = d->position() + d->highlightRangeEnd - 1 - d->highlight->size();
            if (pos < d->position() + d->highlightRangeStart)
                pos = d->position() + d->highlightRangeStart;
            d->highlight->setPosition(pos);
        }
    }
}

qreal QmlGraphicsListView::minYExtent() const
{
    Q_D(const QmlGraphicsListView);
    if (d->orient == QmlGraphicsListView::Horizontal)
        return QmlGraphicsFlickable::minYExtent();
    qreal extent = -d->startPosition();
    if (d->haveHighlightRange && d->highlightRange == StrictlyEnforceRange)
        extent += d->highlightRangeStart;

    return extent;
}

qreal QmlGraphicsListView::maxYExtent() const
{
    Q_D(const QmlGraphicsListView);
    if (d->orient == QmlGraphicsListView::Horizontal)
        return QmlGraphicsFlickable::maxYExtent();
    qreal extent;
    if (d->haveHighlightRange && d->highlightRange == StrictlyEnforceRange)
        extent = -(d->positionAt(count()-1) - d->highlightRangeEnd);
    else
        extent = -(d->endPosition() - height());
    qreal minY = minYExtent();
    if (extent > minY)
        extent = minY;
    return extent;
}

qreal QmlGraphicsListView::minXExtent() const
{
    Q_D(const QmlGraphicsListView);
    if (d->orient == QmlGraphicsListView::Vertical)
        return QmlGraphicsFlickable::minXExtent();
    qreal extent = -d->startPosition();
    if (d->haveHighlightRange && d->highlightRange == StrictlyEnforceRange)
        extent += d->highlightRangeStart;

    return extent;
}

qreal QmlGraphicsListView::maxXExtent() const
{
    Q_D(const QmlGraphicsListView);
    if (d->orient == QmlGraphicsListView::Vertical)
        return QmlGraphicsFlickable::maxXExtent();
    qreal extent;
    if (d->haveHighlightRange && d->highlightRange == StrictlyEnforceRange)
        extent = -(d->positionAt(count()-1) - d->highlightRangeEnd);
    else
        extent = -(d->endPosition() - width());
    qreal minX = minXExtent();
    if (extent > minX)
        extent = minX;
    return extent;
}

void QmlGraphicsListView::keyPressEvent(QKeyEvent *event)
{
    Q_D(QmlGraphicsListView);
    QmlGraphicsFlickable::keyPressEvent(event);
    if (event->isAccepted())
        return;

    if (d->model && d->model->count() && d->interactive) {
        if ((d->orient == QmlGraphicsListView::Horizontal && event->key() == Qt::Key_Left)
                    || (d->orient == QmlGraphicsListView::Vertical && event->key() == Qt::Key_Up)) {
            if (currentIndex() > 0 || (d->wrap && !event->isAutoRepeat())) {
                d->moveReason = QmlGraphicsListViewPrivate::Key;
                decrementCurrentIndex();
                event->accept();
                return;
            } else if (d->wrap) {
                event->accept();
                return;
            }
        } else if ((d->orient == QmlGraphicsListView::Horizontal && event->key() == Qt::Key_Right)
                    || (d->orient == QmlGraphicsListView::Vertical && event->key() == Qt::Key_Down)) {
            if (currentIndex() < d->model->count() - 1 || (d->wrap && !event->isAutoRepeat())) {
                d->moveReason = QmlGraphicsListViewPrivate::Key;
                incrementCurrentIndex();
                event->accept();
                return;
            } else if (d->wrap) {
                event->accept();
                return;
            }
        }
    }
    d->moveReason = QmlGraphicsListViewPrivate::Other;
    event->ignore();
}

/*!
    \qmlmethod ListView::incrementCurrentIndex

    Increments the current index.  The current index will wrap
    if keyNavigationWraps is true and it is currently at the end.
*/
void QmlGraphicsListView::incrementCurrentIndex()
{
    Q_D(QmlGraphicsListView);
    if (currentIndex() < d->model->count() - 1 || d->wrap) {
        int index = currentIndex()+1;
        cancelFlick();
        d->updateCurrent(index < d->model->count() ? index : 0);
    }
}

/*!
    \qmlmethod ListView::decrementCurrentIndex

    Decrements the current index.  The current index will wrap
    if keyNavigationWraps is true and it is currently at the beginning.
*/
void QmlGraphicsListView::decrementCurrentIndex()
{
    Q_D(QmlGraphicsListView);
    if (currentIndex() > 0 || d->wrap) {
        int index = currentIndex()-1;
        cancelFlick();
        d->updateCurrent(index >= 0 ? index : d->model->count()-1);
    }
}

void QmlGraphicsListView::componentComplete()
{
    Q_D(QmlGraphicsListView);
    QmlGraphicsFlickable::componentComplete();
    if (d->currentIndex < 0)
        d->updateCurrent(0);
    refill();
    d->fixupPosition();
}

void QmlGraphicsListView::refill()
{
    Q_D(QmlGraphicsListView);
    d->refill(d->position(), d->position()+d->size()-1);
}

void QmlGraphicsListView::trackedPositionChanged()
{
    Q_D(QmlGraphicsListView);
    if (!d->trackedItem)
        return;
    if (!isFlicking() && !d->moving && d->moveReason != QmlGraphicsListViewPrivate::Mouse) {
        const qreal trackedPos = d->trackedItem->position();
        const qreal viewPos = d->position();
        if (d->haveHighlightRange) {
            if (d->highlightRange == StrictlyEnforceRange) {
                qreal pos = viewPos;
                if (trackedPos > pos + d->highlightRangeEnd - d->trackedItem->size())
                    pos = trackedPos - d->highlightRangeEnd + d->trackedItem->size();
                if (trackedPos < pos + d->highlightRangeStart)
                    pos = trackedPos - d->highlightRangeStart;
                d->setPosition(pos);
            } else {
                qreal pos = viewPos;
                if (trackedPos < d->startPosition() + d->highlightRangeStart) {
                    pos = d->startPosition();
                } else if (d->trackedItem->endPosition() > d->endPosition() - d->size() + d->highlightRangeEnd) {
                    pos = d->endPosition() - d->size();
                } else {
                    if (trackedPos < viewPos + d->highlightRangeStart) {
                        pos = trackedPos - d->highlightRangeStart;
                    } else if (trackedPos > viewPos + d->highlightRangeEnd - d->trackedItem->size()) {
                        pos = trackedPos - d->highlightRangeEnd + d->trackedItem->size();
                    }
                }
                d->setPosition(pos);
            }
        } else {
            if (trackedPos < viewPos && d->currentItem->position() < viewPos) {
                d->setPosition(d->currentItem->position() < trackedPos ? trackedPos : d->currentItem->position());
                d->fixupPosition();
            } else if (d->trackedItem->endPosition() > viewPos + d->size()
                        && d->currentItem->endPosition() > viewPos + d->size()) {
                qreal pos;
                if (d->trackedItem->endPosition() < d->currentItem->endPosition()) {
                    pos = d->trackedItem->endPosition() - d->size();
                    if (d->trackedItem->size() > d->size())
                        pos = trackedPos;
                } else {
                    pos = d->currentItem->endPosition() - d->size();
                    if (d->currentItem->size() > d->size())
                        pos = d->currentItem->position();
                }
                d->setPosition(pos);
                d->fixupPosition();
            }
        }
    }
}

void QmlGraphicsListView::itemResized()
{
    Q_D(QmlGraphicsListView);
    QmlGraphicsItem *item = qobject_cast<QmlGraphicsItem*>(sender());
    if (item) {
        d->layout();
        d->fixupPosition();
    }
}

void QmlGraphicsListView::itemsInserted(int modelIndex, int count)
{
    Q_D(QmlGraphicsListView);
    d->updateUnrequestedIndexes();
    if (!d->visibleItems.count() || d->model->count() <= 1) {
        d->layout();
        d->updateCurrent(qMax(0, qMin(d->currentIndex, d->model->count()-1)));
        emit countChanged();
        return;
    }

    if (!d->mapRangeFromModel(modelIndex, count)) {
        int i = d->visibleItems.count() - 1;
        while (i > 0 && d->visibleItems.at(i)->index == -1)
            --i;
        if (d->visibleItems.at(i)->index + 1 == modelIndex) {
            // Special case of appending an item to the model.
            modelIndex = d->visibleIndex + d->visibleItems.count();
        } else {
            if (modelIndex + count - 1 < d->visibleIndex) {
                // Insert before visible items
                d->visibleIndex += count;
                for (int i = 0; i < d->visibleItems.count(); ++i) {
                    FxListItem *listItem = d->visibleItems.at(i);
                    if (listItem->index != -1)
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

    int index = modelIndex - d->visibleIndex;
    int to = d->buffer+d->position()+d->size()-1;
    // index can be the next item past the end of the visible items list (i.e. appended)
    int pos = index < d->visibleItems.count() ? d->visibleItems.at(index)->position()
                                                : d->visibleItems.at(index-1)->endPosition()+d->spacing+1;
    int initialPos = pos;
    QList<FxListItem*> added;
    for (int i = 0; i < count && pos <= to; ++i) {
        FxListItem *item = d->createItem(modelIndex + i);
        d->visibleItems.insert(index, item);
        item->setPosition(pos);
        added.append(item);
        pos += item->size() + d->spacing;
        ++index;
    }
    if (d->currentIndex >= modelIndex) {
        // adjust current item index
        d->currentIndex += count;
        if (d->currentItem) {
            d->currentItem->index = d->currentIndex;
            d->currentItem->setPosition(d->currentItem->position() + (pos - initialPos));
        }
    }
    if (pos > to) {
        // We didn't insert all our new items, which means anything
        // beyond the current index is not visible - remove it.
        while (d->visibleItems.count() > index)
            d->releaseItem(d->visibleItems.takeLast());
    } else {
        // Update the indexes of the following visible items.
        for (; index < d->visibleItems.count(); ++index) {
            FxListItem *listItem = d->visibleItems.at(index);
            if (listItem->item != d->currentItem->item)
                listItem->setPosition(listItem->position() + (pos - initialPos));
            if (listItem->index != -1)
                listItem->index += count;
        }
    }
    // everything is in order now - emit add() signal
    for (int j = 0; j < added.count(); ++j)
        added.at(j)->attached->emitAdd();
    d->updateUnrequestedPositions();
    d->updateViewport();
    emit countChanged();
}

void QmlGraphicsListView::itemsRemoved(int modelIndex, int count)
{
    Q_D(QmlGraphicsListView);
    d->updateUnrequestedIndexes();
    bool currentRemoved = d->currentIndex >= modelIndex && d->currentIndex < modelIndex + count;
    if (!d->mapRangeFromModel(modelIndex, count)) {
        if (modelIndex + count - 1 < d->visibleIndex) {
            // Items removed before our visible items.
            d->visibleIndex -= count;
            for (int i = 0; i < d->visibleItems.count(); ++i) {
                FxListItem *listItem = d->visibleItems.at(i);
                if (listItem->index != -1)
                    listItem->index -= count;
            }
        }
        if (d->currentIndex >= modelIndex + count) {
            d->currentIndex -= count;
            if (d->currentItem)
                d->currentItem->index -= count;
        } else if (currentRemoved) {
            // current item has been removed.
            d->releaseItem(d->currentItem);
            d->currentItem = 0;
            d->currentIndex = -1;
            d->updateCurrent(qMin(modelIndex, d->model->count()-1));
        }
        d->layout();
        d->updateSections();
        emit countChanged();
        return;
    }

    // Remove the items from the visible list, skipping anything already marked for removal
    QList<FxListItem*>::Iterator it = d->visibleItems.begin();
    while (it != d->visibleItems.end()) {
        FxListItem *item = *it;
        if (item->index == -1 || item->index < modelIndex) {
            // already removed, or before removed items
            ++it;
        } else if (item->index >= modelIndex + count) {
            // after removed items
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
    } else if (currentRemoved) {
        // current item has been removed.
        d->currentItem->attached->setIsCurrentItem(false);
        d->releaseItem(d->currentItem);
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
        d->visiblePos = 0;
        d->timeline.clear();
        d->setPosition(0);
        if (d->model->count() == 0)
            update();
        else
            refill();
    } else {
        // Correct the positioning of the items
        d->layout();
        d->updateSections();
    }

    emit countChanged();
}

void QmlGraphicsListView::destroyRemoved()
{
    Q_D(QmlGraphicsListView);
    for (QList<FxListItem*>::Iterator it = d->visibleItems.begin();
            it != d->visibleItems.end();) {
        FxListItem *listItem = *it;
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

void QmlGraphicsListView::itemsMoved(int from, int to, int count)
{
    Q_D(QmlGraphicsListView);
    qreal firstItemPos = d->visibleItems.first()->position();
    QHash<int,FxListItem*> moved;
    int moveBy = 0;

    QList<FxListItem*>::Iterator it = d->visibleItems.begin();
    while (it != d->visibleItems.end()) {
        FxListItem *item = *it;
        if (item->index >= from && item->index < from + count) {
            // take the items that are moving
            item->index += (to-from);
            moved.insert(item->index, item);
            moveBy += item->size();
            it = d->visibleItems.erase(it);
        } else {
            // move everything after the moved items.
            if (item->index > from && item->index != -1)
                item->index -= count;
            ++it;
        }
    }

    int remaining = count;
    int endIndex = d->visibleIndex;
    it = d->visibleItems.begin();
    while (it != d->visibleItems.end()) {
        FxListItem *item = *it;
        if (remaining && item->index >= to && item->index < to + count) {
            // place items in the target position, reusing any existing items
            FxListItem *movedItem = moved.take(item->index);
            if (!movedItem)
                movedItem = d->createItem(item->index);
            it = d->visibleItems.insert(it, movedItem);
            ++it;
            --remaining;
        } else {
            if (item->index != -1) {
                if (item->index >= to) {
                    // update everything after the moved items.
                    item->index += count;
                }
                endIndex = item->index;
            }
            ++it;
        }
    }

    // If we have moved items to the end of the visible items
    // then add any existing moved items that we have
    while (FxListItem *item = moved.take(endIndex+1)) {
        d->visibleItems.append(item);
        ++endIndex;
    }

    // Whatever moved items remain are no longer visible items.
    while (moved.count())
        d->releaseItem(moved.take(moved.begin().key()));

    // Ensure we don't cause an ugly list scroll.
    d->visibleItems.first()->setPosition(firstItemPos);

    d->layout();
}

void QmlGraphicsListView::createdItem(int index, QmlGraphicsItem *item)
{
    Q_D(QmlGraphicsListView);
    if (d->requestedIndex != index) {
        item->setParentItem(viewport());
        d->unrequestedItems.insert(item, index);
        if (d->orient == QmlGraphicsListView::Vertical)
            item->setY(d->positionAt(index));
        else
            item->setX(d->positionAt(index));
    }
}

void QmlGraphicsListView::destroyingItem(QmlGraphicsItem *item)
{
    Q_D(QmlGraphicsListView);
    d->unrequestedItems.remove(item);
}

QmlGraphicsListViewAttached *QmlGraphicsListView::qmlAttachedProperties(QObject *obj)
{
    return QmlGraphicsListViewAttached::properties(obj);
}

QML_DEFINE_TYPE(Qt,4,6,(QT_VERSION&0x00ff00)>>8,ListView,QmlGraphicsListView)

QT_END_NAMESPACE

#include "qmlgraphicslistview.moc"
