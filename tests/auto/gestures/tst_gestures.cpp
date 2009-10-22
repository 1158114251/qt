/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the test suite of the Qt Toolkit.
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


#include <QtTest/QtTest>
#include "../../shared/util.h"

#include <qevent.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qgesture.h>
#include <qgesturerecognizer.h>
#include <qgraphicsitem.h>
#include <qgraphicsview.h>

#include <qdebug.h>

//TESTED_CLASS=
//TESTED_FILES=

static QPointF mapToGlobal(const QPointF &pt, QGraphicsItem *item, QGraphicsView *view)
{
    return view->mapToGlobal(view->mapFromScene(item->mapToScene(pt)));
}

class CustomGesture : public QGesture
{
    Q_OBJECT
public:
    static Qt::GestureType GestureType;

    CustomGesture(QObject *parent = 0)
        : QGesture(parent), serial(0)
    {
    }

    int serial;

    static const int SerialMaybeThreshold;
    static const int SerialStartedThreshold;
    static const int SerialFinishedThreshold;
};
Qt::GestureType CustomGesture::GestureType = Qt::CustomGesture;
const int CustomGesture::SerialMaybeThreshold = 1;
const int CustomGesture::SerialStartedThreshold = 3;
const int CustomGesture::SerialFinishedThreshold = 6;

class CustomEvent : public QEvent
{
public:
    static int EventType;

    CustomEvent(int serial_ = 0)
        : QEvent(QEvent::Type(CustomEvent::EventType)),
          serial(serial_), hasHotSpot(false)
    {
    }

    int serial;
    QPointF hotSpot;
    bool hasHotSpot;
};
int CustomEvent::EventType = 0;

class CustomGestureRecognizer : public QGestureRecognizer
{
public:
    CustomGestureRecognizer()
    {
        CustomEvent::EventType = QEvent::registerEventType();
        eventsCounter = 0;
    }

    QGesture* createGesture(QObject *)
    {
        return new CustomGesture;
    }

    QGestureRecognizer::Result filterEvent(QGesture *state, QObject*, QEvent *event)
    {
        if (event->type() == CustomEvent::EventType) {
            QGestureRecognizer::Result result = QGestureRecognizer::ConsumeEventHint;
            CustomGesture *g = static_cast<CustomGesture*>(state);
            CustomEvent *e = static_cast<CustomEvent*>(event);
            g->serial = e->serial;
            if (e->hasHotSpot)
                g->setHotSpot(e->hotSpot);
            ++eventsCounter;
            if (g->serial >= CustomGesture::SerialFinishedThreshold)
                result |= QGestureRecognizer::GestureFinished;
            else if (g->serial >= CustomGesture::SerialStartedThreshold)
                result |= QGestureRecognizer::GestureTriggered;
            else if (g->serial >= CustomGesture::SerialMaybeThreshold)
                result |= QGestureRecognizer::MaybeGesture;
            else
                result = QGestureRecognizer::NotGesture;
            return result;
        }
        return QGestureRecognizer::Ignore;
    }

    void reset(QGesture *state)
    {
        CustomGesture *g = static_cast<CustomGesture*>(state);
        g->serial = 0;
        QGestureRecognizer::reset(state);
    }

    int eventsCounter;
    QString name;
};

class GestureWidget : public QWidget
{
    Q_OBJECT
public:
    GestureWidget(const char *name = 0)
    {
        if (name)
            setObjectName(QLatin1String(name));
        reset();
        acceptGestureOverride = false;
    }
    void reset()
    {
        customEventsReceived = 0;
        gestureEventsReceived = 0;
        gestureOverrideEventsReceived = 0;
        events.clear();
        overrideEvents.clear();
    }

    int customEventsReceived;
    int gestureEventsReceived;
    int gestureOverrideEventsReceived;
    struct Events
    {
        QList<Qt::GestureType> all;
        QList<Qt::GestureType> started;
        QList<Qt::GestureType> updated;
        QList<Qt::GestureType> finished;
        QList<Qt::GestureType> canceled;

        void clear()
        {
            all.clear();
            started.clear();
            updated.clear();
            finished.clear();
            canceled.clear();
        }
    } events, overrideEvents;

    bool acceptGestureOverride;

protected:
    bool event(QEvent *event)
    {
        Events *eventsPtr = 0;
        if (event->type() == QEvent::Gesture) {
            ++gestureEventsReceived;
            eventsPtr = &events;
        } else if (event->type() == QEvent::GestureOverride) {
            ++gestureOverrideEventsReceived;
            eventsPtr = &overrideEvents;
            if (acceptGestureOverride)
                event->accept();
        }
        if (eventsPtr) {
            QGestureEvent *e = static_cast<QGestureEvent*>(event);
            QList<QGesture*> gestures = e->allGestures();
            foreach(QGesture *g, gestures) {
                eventsPtr->all << g->gestureType();
                switch(g->state()) {
                case Qt::GestureStarted:
                    eventsPtr->started << g->gestureType();
                    break;
                case Qt::GestureUpdated:
                    eventsPtr->updated << g->gestureType();
                    break;
                case Qt::GestureFinished:
                    eventsPtr->finished << g->gestureType();
                    break;
                case Qt::GestureCanceled:
                    eventsPtr->canceled << g->gestureType();
                    break;
                default:
                    Q_ASSERT(false);
                }
            }
        } else if (event->type() == CustomEvent::EventType) {
            ++customEventsReceived;
        } else {
            return QWidget::event(event);
        }
        return true;
    }
};

static void sendCustomGesture(CustomEvent *event, QObject *object, QGraphicsScene *scene = 0)
{
    for (int i = CustomGesture::SerialMaybeThreshold;
         i <= CustomGesture::SerialFinishedThreshold; ++i) {
        event->serial = i;
        if (scene)
            scene->sendEvent(qobject_cast<QGraphicsObject *>(object), event);
        else
            QApplication::sendEvent(object, event);
    }
}

class tst_Gestures : public QObject
{
Q_OBJECT

public:
    tst_Gestures();
    virtual ~tst_Gestures();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void customGesture();
    void autoCancelingGestures();
    void gestureOverChild();
    void multipleWidgetOnlyGestureInTree();
    void conflictingGestures();
    void finishedWithoutStarted();
    void unknownGesture();
    void graphicsItemGesture();
    void explicitGraphicsObjectTarget();
    void gestureOverChildGraphicsItem();
};

tst_Gestures::tst_Gestures()
{
}

tst_Gestures::~tst_Gestures()
{
}

void tst_Gestures::initTestCase()
{
    CustomGesture::GestureType = qApp->registerGestureRecognizer(new CustomGestureRecognizer);
    QVERIFY(CustomGesture::GestureType != Qt::GestureType(0));
    QVERIFY(CustomGesture::GestureType != Qt::CustomGesture);
}

void tst_Gestures::cleanupTestCase()
{
}

void tst_Gestures::init()
{
}

void tst_Gestures::cleanup()
{
}

void tst_Gestures::customGesture()
{
    GestureWidget widget;
    widget.grabGesture(CustomGesture::GestureType, Qt::WidgetGesture);
    CustomEvent event;
    sendCustomGesture(&event, &widget);

    static const int TotalGestureEventsCount = CustomGesture::SerialFinishedThreshold - CustomGesture::SerialStartedThreshold + 1;
    static const int TotalCustomEventsCount = CustomGesture::SerialFinishedThreshold - CustomGesture::SerialMaybeThreshold + 1;
    QCOMPARE(widget.customEventsReceived, TotalCustomEventsCount);
    QCOMPARE(widget.gestureEventsReceived, TotalGestureEventsCount);
    QCOMPARE(widget.gestureOverrideEventsReceived, 0);
    QCOMPARE(widget.events.all.size(), TotalGestureEventsCount);
    for(int i = 0; i < widget.events.all.size(); ++i)
        QCOMPARE(widget.events.all.at(i), CustomGesture::GestureType);
    QCOMPARE(widget.events.started.size(), 1);
    QCOMPARE(widget.events.updated.size(), TotalGestureEventsCount - 2);
    QCOMPARE(widget.events.finished.size(), 1);
    QCOMPARE(widget.events.canceled.size(), 0);
}

void tst_Gestures::autoCancelingGestures()
{
    GestureWidget widget;
    widget.grabGesture(CustomGesture::GestureType, Qt::WidgetGesture);
    // send partial gesture. The gesture will be in the "maybe" state, but will
    // never get enough events to fire, so Qt will have to kill it.
    CustomEvent ev;
    for (int i = CustomGesture::SerialMaybeThreshold;
         i < CustomGesture::SerialStartedThreshold; ++i) {
        ev.serial = i;
        QApplication::sendEvent(&widget, &ev);
    }
    // wait long enough so the gesture manager will cancel the gesture
    QTest::qWait(5000);
    QCOMPARE(widget.customEventsReceived, CustomGesture::SerialStartedThreshold - CustomGesture::SerialMaybeThreshold);
    QCOMPARE(widget.gestureEventsReceived, 0);
    QCOMPARE(widget.gestureOverrideEventsReceived, 0);
    QCOMPARE(widget.events.all.size(), 0);
}

void tst_Gestures::gestureOverChild()
{
    GestureWidget widget("widget");
    QVBoxLayout *l = new QVBoxLayout(&widget);
    GestureWidget *child = new GestureWidget("child");
    l->addWidget(child);

    widget.grabGesture(CustomGesture::GestureType, Qt::WidgetGesture);

    CustomEvent event;
    sendCustomGesture(&event, child);

    static const int TotalGestureEventsCount = CustomGesture::SerialFinishedThreshold - CustomGesture::SerialStartedThreshold + 1;
    static const int TotalCustomEventsCount = CustomGesture::SerialFinishedThreshold - CustomGesture::SerialMaybeThreshold + 1;

    QCOMPARE(child->customEventsReceived, TotalCustomEventsCount);
    QCOMPARE(widget.customEventsReceived, 0);
    QCOMPARE(child->gestureEventsReceived, 0);
    QCOMPARE(child->gestureOverrideEventsReceived, 0);
    QCOMPARE(widget.gestureEventsReceived, 0);
    QCOMPARE(widget.gestureOverrideEventsReceived, 0);

    // enable gestures over the children
    widget.grabGesture(CustomGesture::GestureType, Qt::WidgetWithChildrenGesture);

    widget.reset();
    child->reset();

    sendCustomGesture(&event, child);

    QCOMPARE(child->customEventsReceived, TotalCustomEventsCount);
    QCOMPARE(widget.customEventsReceived, 0);

    QCOMPARE(child->gestureEventsReceived, 0);
    QCOMPARE(child->gestureOverrideEventsReceived, 0);
    QCOMPARE(widget.gestureEventsReceived, TotalGestureEventsCount);
    QCOMPARE(widget.gestureOverrideEventsReceived, 0);
    for(int i = 0; i < widget.events.all.size(); ++i)
        QCOMPARE(widget.events.all.at(i), CustomGesture::GestureType);
    QCOMPARE(widget.events.started.size(), 1);
    QCOMPARE(widget.events.updated.size(), TotalGestureEventsCount - 2);
    QCOMPARE(widget.events.finished.size(), 1);
    QCOMPARE(widget.events.canceled.size(), 0);
}

void tst_Gestures::multipleWidgetOnlyGestureInTree()
{
    GestureWidget parent("parent");
    QVBoxLayout *l = new QVBoxLayout(&parent);
    GestureWidget *child = new GestureWidget("child");
    l->addWidget(child);

    parent.grabGesture(CustomGesture::GestureType, Qt::WidgetGesture);
    child->grabGesture(CustomGesture::GestureType, Qt::WidgetGesture);

    static const int TotalGestureEventsCount = CustomGesture::SerialFinishedThreshold - CustomGesture::SerialStartedThreshold + 1;
    static const int TotalCustomEventsCount = CustomGesture::SerialFinishedThreshold - CustomGesture::SerialMaybeThreshold + 1;

    // sending events to the child and making sure there is no conflict
    CustomEvent event;
    sendCustomGesture(&event, child);

    QCOMPARE(child->customEventsReceived, TotalCustomEventsCount);
    QCOMPARE(parent.customEventsReceived, 0);
    QCOMPARE(child->gestureEventsReceived, TotalGestureEventsCount);
    QCOMPARE(child->gestureOverrideEventsReceived, 0);
    QCOMPARE(parent.gestureEventsReceived, 0);
    QCOMPARE(parent.gestureOverrideEventsReceived, 0);

    parent.reset();
    child->reset();

    // same for the parent widget
    sendCustomGesture(&event, &parent);

    QCOMPARE(child->customEventsReceived, 0);
    QCOMPARE(parent.customEventsReceived, TotalCustomEventsCount);
    QCOMPARE(child->gestureEventsReceived, 0);
    QCOMPARE(child->gestureOverrideEventsReceived, 0);
    QCOMPARE(parent.gestureEventsReceived, TotalGestureEventsCount);
    QCOMPARE(parent.gestureOverrideEventsReceived, 0);
}

void tst_Gestures::conflictingGestures()
{
    GestureWidget parent("parent");
    QVBoxLayout *l = new QVBoxLayout(&parent);
    GestureWidget *child = new GestureWidget("child");
    l->addWidget(child);

    parent.grabGesture(CustomGesture::GestureType, Qt::WidgetWithChildrenGesture);
    child->grabGesture(CustomGesture::GestureType, Qt::WidgetWithChildrenGesture);

    static const int TotalGestureEventsCount = CustomGesture::SerialFinishedThreshold - CustomGesture::SerialStartedThreshold + 1;

    // child accepts the override, parent will not receive anything
    parent.acceptGestureOverride = false;
    child->acceptGestureOverride = true;

    // sending events to the child and making sure there is no conflict
    CustomEvent event;
    sendCustomGesture(&event, child);

    QCOMPARE(child->gestureOverrideEventsReceived, TotalGestureEventsCount);
    QCOMPARE(child->gestureEventsReceived, 0);
    QCOMPARE(parent.gestureOverrideEventsReceived, 0);
    QCOMPARE(parent.gestureEventsReceived, 0);

    parent.reset();
    child->reset();

    // parent accepts the override
    parent.acceptGestureOverride = true;
    child->acceptGestureOverride = false;

    // sending events to the child and making sure there is no conflict
    sendCustomGesture(&event, child);

    QCOMPARE(child->gestureOverrideEventsReceived, TotalGestureEventsCount);
    QCOMPARE(child->gestureEventsReceived, 0);
    QCOMPARE(parent.gestureOverrideEventsReceived, TotalGestureEventsCount);
    QCOMPARE(parent.gestureEventsReceived, 0);

    parent.reset();
    child->reset();

    // nobody accepts the override, we will send normal events to the closest context (to the child)
    parent.acceptGestureOverride = false;
    child->acceptGestureOverride = false;

    // sending events to the child and making sure there is no conflict
    sendCustomGesture(&event, child);

    QCOMPARE(child->gestureOverrideEventsReceived, TotalGestureEventsCount);
    QCOMPARE(child->gestureEventsReceived, TotalGestureEventsCount);
    QCOMPARE(parent.gestureOverrideEventsReceived, TotalGestureEventsCount);
    QCOMPARE(parent.gestureEventsReceived, 0);
}

void tst_Gestures::finishedWithoutStarted()
{
    GestureWidget widget;
    widget.grabGesture(CustomGesture::GestureType, Qt::WidgetGesture);

    // the gesture will claim it finished, but it was never started.
    CustomEvent ev;
    ev.serial = CustomGesture::SerialFinishedThreshold;
    QApplication::sendEvent(&widget, &ev);

    QCOMPARE(widget.customEventsReceived, 1);
    QCOMPARE(widget.gestureEventsReceived, 2);
    QCOMPARE(widget.gestureOverrideEventsReceived, 0);
    QCOMPARE(widget.events.all.size(), 2);
    QCOMPARE(widget.events.started.size(), 1);
    QCOMPARE(widget.events.updated.size(), 0);
    QCOMPARE(widget.events.finished.size(), 1);
    QCOMPARE(widget.events.canceled.size(), 0);
}

void tst_Gestures::unknownGesture()
{
    GestureWidget widget;
    widget.grabGesture(CustomGesture::GestureType, Qt::WidgetGesture);
    widget.grabGesture(Qt::CustomGesture, Qt::WidgetGesture);
    widget.grabGesture(Qt::GestureType(Qt::PanGesture+512), Qt::WidgetGesture);

    CustomEvent event;
    sendCustomGesture(&event, &widget);

    static const int TotalGestureEventsCount = CustomGesture::SerialFinishedThreshold - CustomGesture::SerialStartedThreshold + 1;

    QCOMPARE(widget.gestureEventsReceived, TotalGestureEventsCount);
}

class GestureItem : public QGraphicsObject
{
public:
    GestureItem()
    {
        size = QRectF(0, 0, 100, 100);
        customEventsReceived = 0;
        gestureEventsReceived = 0;
        gestureOverrideEventsReceived = 0;
        events.clear();
        overrideEvents.clear();
        acceptGestureOverride = false;
    }

    int customEventsReceived;
    int gestureEventsReceived;
    int gestureOverrideEventsReceived;
    struct Events
    {
        QList<Qt::GestureType> all;
        QList<Qt::GestureType> started;
        QList<Qt::GestureType> updated;
        QList<Qt::GestureType> finished;
        QList<Qt::GestureType> canceled;

        void clear()
        {
            all.clear();
            started.clear();
            updated.clear();
            finished.clear();
            canceled.clear();
        }
    } events, overrideEvents;

    bool acceptGestureOverride;

    QRectF size;

    void reset()
    {
        customEventsReceived = 0;
        gestureEventsReceived = 0;
        gestureOverrideEventsReceived = 0;
        events.clear();
        overrideEvents.clear();
    }

protected:
    QRectF boundingRect() const
    {
        return size;
    }
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *)
    {
        p->fillRect(boundingRect(), Qt::blue);
    }

    bool event(QEvent *event)
    {
        Events *eventsPtr = 0;
        if (event->type() == QEvent::Gesture) {
            ++gestureEventsReceived;
            eventsPtr = &events;
        } else if (event->type() == QEvent::GestureOverride) {
            ++gestureOverrideEventsReceived;
            eventsPtr = &overrideEvents;
            if (acceptGestureOverride)
                event->accept();
        }
        if (eventsPtr) {
            QGestureEvent *e = static_cast<QGestureEvent*>(event);
            QList<QGesture*> gestures = e->allGestures();
            foreach(QGesture *g, gestures) {
                eventsPtr->all << g->gestureType();
                switch(g->state()) {
                case Qt::GestureStarted:
                    eventsPtr->started << g->gestureType();
                    break;
                case Qt::GestureUpdated:
                    eventsPtr->updated << g->gestureType();
                    break;
                case Qt::GestureFinished:
                    eventsPtr->finished << g->gestureType();
                    break;
                case Qt::GestureCanceled:
                    eventsPtr->canceled << g->gestureType();
                    break;
                default:
                    Q_ASSERT(false);
                }
            }
        } else if (event->type() == CustomEvent::EventType) {
            ++customEventsReceived;
        } else {
            return QGraphicsObject::event(event);
        }
        return true;
    }
};

void tst_Gestures::graphicsItemGesture()
{
    QGraphicsScene scene;
    QGraphicsView view(&scene);

    GestureItem *item = new GestureItem;
    scene.addItem(item);
    item->setPos(100, 100);

    view.show();
    QTest::qWaitForWindowShown(&view);
    view.ensureVisible(scene.sceneRect());

    view.viewport()->grabGesture(CustomGesture::GestureType, Qt::WidgetGesture);
    item->grabGesture(CustomGesture::GestureType);

    static const int TotalGestureEventsCount = CustomGesture::SerialFinishedThreshold - CustomGesture::SerialStartedThreshold + 1;
    static const int TotalCustomEventsCount = CustomGesture::SerialFinishedThreshold - CustomGesture::SerialMaybeThreshold + 1;

    CustomEvent event;
    sendCustomGesture(&event, item, &scene);

    QCOMPARE(item->customEventsReceived, TotalCustomEventsCount);
    QCOMPARE(item->gestureEventsReceived, TotalGestureEventsCount);
    QCOMPARE(item->gestureOverrideEventsReceived, 0);
    QCOMPARE(item->events.all.size(), TotalGestureEventsCount);
    for(int i = 0; i < item->events.all.size(); ++i)
        QCOMPARE(item->events.all.at(i), CustomGesture::GestureType);
    QCOMPARE(item->events.started.size(), 1);
    QCOMPARE(item->events.updated.size(), TotalGestureEventsCount - 2);
    QCOMPARE(item->events.finished.size(), 1);
    QCOMPARE(item->events.canceled.size(), 0);

    item->reset();

    // make sure the event is properly delivered if only the hotspot is set.
    event.hotSpot = mapToGlobal(QPointF(10, 10), item, &view);
    event.hasHotSpot = true;
    sendCustomGesture(&event, item, &scene);

    QCOMPARE(item->customEventsReceived, TotalCustomEventsCount);
    QCOMPARE(item->gestureEventsReceived, TotalGestureEventsCount);
    QCOMPARE(item->gestureOverrideEventsReceived, 0);
    QCOMPARE(item->events.all.size(), TotalGestureEventsCount);
    for(int i = 0; i < item->events.all.size(); ++i)
        QCOMPARE(item->events.all.at(i), CustomGesture::GestureType);
    QCOMPARE(item->events.started.size(), 1);
    QCOMPARE(item->events.updated.size(), TotalGestureEventsCount - 2);
    QCOMPARE(item->events.finished.size(), 1);
    QCOMPARE(item->events.canceled.size(), 0);
}

void tst_Gestures::explicitGraphicsObjectTarget()
{
    QGraphicsScene scene;
    QGraphicsView view(&scene);

    GestureItem *item1 = new GestureItem;
    scene.addItem(item1);
    item1->setPos(100, 100);

    GestureItem *item2 = new GestureItem;
    scene.addItem(item2);
    item2->setPos(100, 100);

    GestureItem *item2_child1 = new GestureItem;
    scene.addItem(item2_child1);
    item2_child1->setParentItem(item2);
    item2_child1->setPos(10, 10);

    view.show();
    QTest::qWaitForWindowShown(&view);
    view.ensureVisible(scene.sceneRect());

    view.viewport()->grabGesture(CustomGesture::GestureType, Qt::WidgetGesture);
    item1->grabGesture(CustomGesture::GestureType, Qt::ItemGesture);
    item2->grabGesture(CustomGesture::GestureType, Qt::ItemGesture);
    item2_child1->grabGesture(CustomGesture::GestureType, Qt::ItemGesture);

    static const int TotalGestureEventsCount = CustomGesture::SerialFinishedThreshold - CustomGesture::SerialStartedThreshold + 1;

    // sending events to item1, but the hotSpot is set to item2
    CustomEvent event;
    event.hotSpot = mapToGlobal(QPointF(15, 15), item2, &view);
    event.hasHotSpot = true;

    sendCustomGesture(&event, item1, &scene);

    QCOMPARE(item1->gestureEventsReceived, 0);
    QCOMPARE(item1->gestureOverrideEventsReceived, 0);
    QCOMPARE(item2_child1->gestureEventsReceived, TotalGestureEventsCount);
    QCOMPARE(item2_child1->gestureOverrideEventsReceived, 0);
    QCOMPARE(item2_child1->events.all.size(), TotalGestureEventsCount);
    for(int i = 0; i < item2_child1->events.all.size(); ++i)
        QCOMPARE(item2_child1->events.all.at(i), CustomGesture::GestureType);
    QCOMPARE(item2_child1->events.started.size(), 1);
    QCOMPARE(item2_child1->events.updated.size(), TotalGestureEventsCount - 2);
    QCOMPARE(item2_child1->events.finished.size(), 1);
    QCOMPARE(item2_child1->events.canceled.size(), 0);
    QCOMPARE(item2->gestureEventsReceived, 0);
    QCOMPARE(item2->gestureOverrideEventsReceived, 0);
}

void tst_Gestures::gestureOverChildGraphicsItem()
{
    QGraphicsScene scene;
    QGraphicsView view(&scene);

    GestureItem *item0 = new GestureItem;
    scene.addItem(item0);
    item0->setPos(0, 0);

    GestureItem *item1 = new GestureItem;
    scene.addItem(item1);
    item1->setPos(100, 100);

    GestureItem *item2 = new GestureItem;
    scene.addItem(item2);
    item2->setPos(100, 100);

    GestureItem *item2_child1 = new GestureItem;
    scene.addItem(item2_child1);
    item2_child1->setParentItem(item2);
    item2_child1->setPos(0, 0);

    view.show();
    QTest::qWaitForWindowShown(&view);
    view.ensureVisible(scene.sceneRect());

    view.viewport()->grabGesture(CustomGesture::GestureType, Qt::WidgetGesture);
    item1->grabGesture(CustomGesture::GestureType);

    static const int TotalGestureEventsCount = CustomGesture::SerialFinishedThreshold - CustomGesture::SerialStartedThreshold + 1;
    static const int TotalCustomEventsCount = CustomGesture::SerialFinishedThreshold - CustomGesture::SerialMaybeThreshold + 1;

    CustomEvent event;
    event.hotSpot = mapToGlobal(QPointF(10, 10), item2_child1, &view);
    event.hasHotSpot = true;
    sendCustomGesture(&event, item0, &scene);

    QCOMPARE(item0->customEventsReceived, TotalCustomEventsCount);
    QCOMPARE(item2_child1->gestureEventsReceived, 0);
    QCOMPARE(item2_child1->gestureOverrideEventsReceived, 0);
    QCOMPARE(item2->gestureEventsReceived, 0);
    QCOMPARE(item2->gestureOverrideEventsReceived, 0);
    QEXPECT_FAIL("", "need to fix gesture event propagation inside graphicsview", Continue);
    QCOMPARE(item1->gestureEventsReceived, TotalGestureEventsCount);
    QCOMPARE(item1->gestureOverrideEventsReceived, 0);

    item0->reset(); item1->reset(); item2->reset(); item2_child1->reset();
    item2->grabGesture(CustomGesture::GestureType);

    event.hotSpot = mapToGlobal(QPointF(10, 10), item2_child1, &view);
    event.hasHotSpot = true;
    sendCustomGesture(&event, item0, &scene);

    QCOMPARE(item0->customEventsReceived, TotalCustomEventsCount);
    QCOMPARE(item2_child1->gestureEventsReceived, 0);
    QCOMPARE(item2_child1->gestureOverrideEventsReceived, 0);
    QEXPECT_FAIL("", "need to fix gesture event propagation inside graphicsview", Continue);
    QCOMPARE(item2->gestureEventsReceived, TotalGestureEventsCount);
    QEXPECT_FAIL("", "need to fix gesture event propagation inside graphicsview", Continue);
    QCOMPARE(item2->gestureOverrideEventsReceived, TotalGestureEventsCount);
    QCOMPARE(item1->gestureEventsReceived, 0);
    QEXPECT_FAIL("", "need to fix gesture event propagation inside graphicsview", Continue);
    QCOMPARE(item1->gestureOverrideEventsReceived, TotalGestureEventsCount);
}

QTEST_MAIN(tst_Gestures)
#include "tst_gestures.moc"
