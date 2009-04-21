/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
****************************************************************************/

#include <QtTest/QtTest>

#include "qstate.h"
#include "qstatemachine.h"
#include "qactiontransition.h"
#include "qsignaltransition.h"
#include "qstateaction.h"

// Will try to wait for the condition while allowing event processing
#define QTRY_COMPARE(__expr, __expected) \
    do { \
        const int __step = 50; \
        const int __timeout = 5000; \
        if ((__expr) != (__expected)) { \
            QTest::qWait(0); \
        } \
        for (int __i = 0; __i < __timeout && ((__expr) != (__expected)); __i+=__step) { \
            QTest::qWait(__step); \
        } \
        QCOMPARE(__expr, __expected); \
    } while(0)

//TESTED_CLASS=
//TESTED_FILES=

class tst_QState : public QObject
{
    Q_OBJECT

public:
    tst_QState();
    virtual ~tst_QState();

private slots:
#if 0
    void test();
#endif
    void assignProperty();
    void assignPropertyTwice();
    void historyInitialState();
    void addEntryAction();

private:
    bool functionCalled;
};

tst_QState::tst_QState() : functionCalled(false)
{
}

tst_QState::~tst_QState()
{
}

#if 0
void tst_QState::test()
{
    QStateMachine machine;
    QState *s1 = new QState(machine.rootState());

    QCOMPARE(s1->machine(), &machine);
    QCOMPARE(s1->parentState(), machine.rootState());
    QCOMPARE(s1->initialState(), (QState*)0);
    QVERIFY(s1->childStates().isEmpty());
    QVERIFY(s1->transitions().isEmpty());

    QCOMPARE(s1->isFinal(), false);
    s1->setFinal(true);
    QCOMPARE(s1->isFinal(), true);
    s1->setFinal(false);
    QCOMPARE(s1->isFinal(), false);

    QCOMPARE(s1->isParallel(), false);
    s1->setParallel(true);
    QCOMPARE(s1->isParallel(), true);
    s1->setParallel(false);
    QCOMPARE(s1->isParallel(), false);

    QCOMPARE(s1->isAtomic(), true);
    QCOMPARE(s1->isCompound(), false);
    QCOMPARE(s1->isComplex(), false);

    QState *s11 = new QState(s1);
    QCOMPARE(s11->parentState(), s1);
    QCOMPARE(s11->isAtomic(), true);
    QCOMPARE(s11->isCompound(), false);
    QCOMPARE(s11->isComplex(), false);
    QCOMPARE(s11->machine(), s1->machine());
    QVERIFY(s11->isDescendantOf(s1));

    QCOMPARE(s1->initialState(), (QState*)0);
    QCOMPARE(s1->childStates().size(), 1);
    QCOMPARE(s1->childStates().at(0), s11);

    QCOMPARE(s1->isAtomic(), false);
    QCOMPARE(s1->isCompound(), true);
    QCOMPARE(s1->isComplex(), true);

    s1->setParallel(true);
    QCOMPARE(s1->isAtomic(), false);
    QCOMPARE(s1->isCompound(), false);
    QCOMPARE(s1->isComplex(), true);

    QState *s12 = new QState(s1);
    QCOMPARE(s12->parentState(), s1);
    QCOMPARE(s12->isAtomic(), true);
    QCOMPARE(s12->isCompound(), false);
    QCOMPARE(s12->isComplex(), false);
    QCOMPARE(s12->machine(), s1->machine());
    QVERIFY(s12->isDescendantOf(s1));
    QVERIFY(!s12->isDescendantOf(s11));

    QCOMPARE(s1->initialState(), (QState*)0);
    QCOMPARE(s1->childStates().size(), 2);
    QCOMPARE(s1->childStates().at(0), s11);
    QCOMPARE(s1->childStates().at(1), s12);

    QCOMPARE(s1->isAtomic(), false);
    QCOMPARE(s1->isCompound(), false);
    QCOMPARE(s1->isComplex(), true);

    s1->setParallel(false);
    QCOMPARE(s1->isAtomic(), false);
    QCOMPARE(s1->isCompound(), true);
    QCOMPARE(s1->isComplex(), true);

    s1->setInitialState(s11);
    QCOMPARE(s1->initialState(), s11);

    s1->setInitialState(0);
    QCOMPARE(s1->initialState(), (QState*)0);

    s1->setInitialState(s12);
    QCOMPARE(s1->initialState(), s12);

    QState *s13 = new QState();
    s1->setInitialState(s13);
    QCOMPARE(s13->parentState(), s1);
    QCOMPARE(s1->childStates().size(), 3);
    QCOMPARE(s1->childStates().at(0), s11);
    QCOMPARE(s1->childStates().at(1), s12);
    QCOMPARE(s1->childStates().at(2), s13);
    QVERIFY(s13->isDescendantOf(s1));

    QVERIFY(s12->childStates().isEmpty());

    QState *s121 = new QState(s12);
    QCOMPARE(s121->parentState(), s12);
    QCOMPARE(s121->isAtomic(), true);
    QCOMPARE(s121->isCompound(), false);
    QCOMPARE(s121->isComplex(), false);
    QCOMPARE(s121->machine(), s12->machine());
    QVERIFY(s121->isDescendantOf(s12));
    QVERIFY(s121->isDescendantOf(s1));
    QVERIFY(!s121->isDescendantOf(s11));

    QCOMPARE(s12->childStates().size(), 1);
    QCOMPARE(s12->childStates().at(0), (QState*)s121);

    QCOMPARE(s1->childStates().size(), 3);
    QCOMPARE(s1->childStates().at(0), s11);
    QCOMPARE(s1->childStates().at(1), s12);
    QCOMPARE(s1->childStates().at(2), s13);

    s11->addTransition(s12);
    QCOMPARE(s11->transitions().size(), 1);
    QCOMPARE(s11->transitions().at(0)->sourceState(), s11);
    QCOMPARE(s11->transitions().at(0)->targetStates().size(), 1);
    QCOMPARE(s11->transitions().at(0)->targetStates().at(0), s12);
    QCOMPARE(s11->transitions().at(0)->eventType(), QEvent::None);

    QState *s14 = new QState();
    s12->addTransition(QList<QState*>() << s13 << s14);
    QCOMPARE(s12->transitions().size(), 1);
    QCOMPARE(s12->transitions().at(0)->sourceState(), s12);
    QCOMPARE(s12->transitions().at(0)->targetStates().size(), 2);
    QCOMPARE(s12->transitions().at(0)->targetStates().at(0), s13);
    QCOMPARE(s12->transitions().at(0)->targetStates().at(1), s14);
    QCOMPARE(s12->transitions().at(0)->eventType(), QEvent::None);

    s13->addTransition(this, SIGNAL(destroyed()), s14);
    QCOMPARE(s13->transitions().size(), 1);
    QCOMPARE(s13->transitions().at(0)->sourceState(), s13);
    QCOMPARE(s13->transitions().at(0)->targetStates().size(), 1);
    QCOMPARE(s13->transitions().at(0)->targetStates().at(0), s14);
    QCOMPARE(s13->transitions().at(0)->eventType(), QEvent::Signal);
    QVERIFY(qobject_cast<QSignalTransition*>(s13->transitions().at(0)) != 0);

    delete s13->transitions().at(0);
    QCOMPARE(s13->transitions().size(), 0);

    s12->addTransition(this, SIGNAL(destroyed()), s11);
    QCOMPARE(s12->transitions().size(), 2);
}
#endif

class TestClass: public QObject
{
    Q_OBJECT
public:
    TestClass() : called(false) {}
    bool called;

public slots:
    void slot() { called = true; }

    
};

void tst_QState::addEntryAction()
{
    QStateMachine sm;

    TestClass testObject;

    QState *s0 = new QState(sm.rootState());
    s0->addEntryAction(new QStateInvokeMethodAction(&testObject, "slot"));
    sm.setInitialState(s0);

    sm.start();
    QCoreApplication::processEvents();
    
    QCOMPARE(testObject.called, true);
}

void tst_QState::assignProperty()
{
    QStateMachine machine;

    QObject *object = new QObject();
    object->setProperty("fooBar", 10);

    QState *s1 = new QState(machine.rootState());
    s1->assignProperty(object, "fooBar", 20);
    
    machine.setInitialState(s1);
    machine.start();
    QCoreApplication::processEvents();

    QCOMPARE(object->property("fooBar").toInt(), 20);
}

void tst_QState::assignPropertyTwice()
{
    QStateMachine machine;

    QObject *object = new QObject();
    object->setProperty("fooBar", 10);

    QState *s1 = new QState(machine.rootState());
    s1->assignProperty(object, "fooBar", 20);
    s1->assignProperty(object, "fooBar", 30);
    
    machine.setInitialState(s1);
    machine.start();
    QCoreApplication::processEvents();

    QCOMPARE(object->property("fooBar").toInt(), 30);
}

class EventTestTransition: public QActionTransition
{
public:
    EventTestTransition(QEvent::Type type, QState *targetState) 
        : QActionTransition(QList<QAbstractState*>() << targetState), m_type(type)
    {        
    }

protected:
    bool eventTest(QEvent *e) const
    {
        return e->type() == m_type;
    }

private:
    QEvent::Type m_type;
    
};

void tst_QState::historyInitialState() 
{
    QStateMachine machine;

    QState *s1 = new QState(machine.rootState());
    
    QState *s2 = new QState(machine.rootState());
    QHistoryState *h1 = s2->addHistoryState();
    
    s2->setInitialState(h1);

    QState *s3 = new QState(s2);
    h1->setDefaultState(s3);

    QState *s4 = new QState(s2);

    s1->addTransition(new EventTestTransition(QEvent::User, s2));
    s2->addTransition(new EventTestTransition(QEvent::User, s1));
    s3->addTransition(new EventTestTransition(QEvent::Type(QEvent::User+1), s4));

    machine.setInitialState(s1);
    machine.start();
    QCoreApplication::processEvents();

    QCOMPARE(machine.configuration().size(), 1);
    QVERIFY(machine.configuration().contains(s1));

    machine.postEvent(new QEvent(QEvent::User));
    QCoreApplication::processEvents();

    QCOMPARE(machine.configuration().size(), 2);
    QVERIFY(machine.configuration().contains(s2));
    QVERIFY(machine.configuration().contains(s3));

    machine.postEvent(new QEvent(QEvent::User));
    QCoreApplication::processEvents();

    QCOMPARE(machine.configuration().size(), 1);
    QVERIFY(machine.configuration().contains(s1));

    machine.postEvent(new QEvent(QEvent::User));
    QCoreApplication::processEvents();

    QCOMPARE(machine.configuration().size(), 2);
    QVERIFY(machine.configuration().contains(s2));
    QVERIFY(machine.configuration().contains(s3));

    machine.postEvent(new QEvent(QEvent::Type(QEvent::User+1)));
    QCoreApplication::processEvents();

    QCOMPARE(machine.configuration().size(), 2);
    QVERIFY(machine.configuration().contains(s2));
    QVERIFY(machine.configuration().contains(s4));

    machine.postEvent(new QEvent(QEvent::User));
    QCoreApplication::processEvents();

    QCOMPARE(machine.configuration().size(), 1);
    QVERIFY(machine.configuration().contains(s1));

    machine.postEvent(new QEvent(QEvent::User));
    QCoreApplication::processEvents();

    QCOMPARE(machine.configuration().size(), 2);
    QVERIFY(machine.configuration().contains(s2));
    QVERIFY(machine.configuration().contains(s4));
}


QTEST_MAIN(tst_QState)
#include "tst_qstate.moc"
