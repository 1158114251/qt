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
#include <qtest.h>
#include <QtDeclarative/qmlengine.h>
#include <QtDeclarative/qmlcomponent.h>
#include <private/qmlgraphicsanchors_p_p.h>
#include <private/qmlgraphicsrectangle_p.h>
#include <private/qmlpropertychanges_p.h>

class tst_states : public QObject
{
    Q_OBJECT
public:
    tst_states() {}

private slots:
    void basicChanges();
    void basicExtension();
    void basicBinding();
    void signalOverride();
    void signalOverrideCrash();
    void parentChange();
    void parentChangeErrors();
    void anchorChanges();
    void anchorChanges2();
    void anchorChanges3();
    void anchorChanges4();
    void anchorChanges5();
    void script();
    void restoreEntryValues();
    void explicitChanges();
    void propertyErrors();
};

void tst_states::basicChanges()
{
    QmlEngine engine;

    {
        QmlComponent rectComponent(&engine, SRCDIR "/data/basicChanges.qml");
        QmlGraphicsRectangle *rect = qobject_cast<QmlGraphicsRectangle*>(rectComponent.create());
        QVERIFY(rect != 0);

        QCOMPARE(rect->color(),QColor("red"));

        rect->setState("blue");
        QCOMPARE(rect->color(),QColor("blue"));

        rect->setState("");
        QCOMPARE(rect->color(),QColor("red"));
    }

    {
        QmlComponent rectComponent(&engine, SRCDIR "/data/basicChanges2.qml");
        QmlGraphicsRectangle *rect = qobject_cast<QmlGraphicsRectangle*>(rectComponent.create());
        QVERIFY(rect != 0);

        QCOMPARE(rect->color(),QColor("red"));

        rect->setState("blue");
        QCOMPARE(rect->color(),QColor("blue"));

        rect->setState("green");
        QCOMPARE(rect->color(),QColor("green"));

        rect->setState("");
        QCOMPARE(rect->color(),QColor("red"));

        rect->setState("green");
        QCOMPARE(rect->color(),QColor("green"));
    }

    {
        QmlComponent rectComponent(&engine, SRCDIR "/data/basicChanges3.qml");
        QmlGraphicsRectangle *rect = qobject_cast<QmlGraphicsRectangle*>(rectComponent.create());
        QVERIFY(rect != 0);

        QCOMPARE(rect->color(),QColor("red"));
        QCOMPARE(rect->border()->width(),1);

        rect->setState("blue");
        QCOMPARE(rect->color(),QColor("blue"));
        QCOMPARE(rect->border()->width(),1);

        rect->setState("bordered");
        QCOMPARE(rect->color(),QColor("red"));
        QCOMPARE(rect->border()->width(),2);

        rect->setState("");
        QCOMPARE(rect->color(),QColor("red"));
        QCOMPARE(rect->border()->width(),1);
        //### we should be checking that this is an implicit rather than explicit 1 (which currently fails)

        rect->setState("bordered");
        QCOMPARE(rect->color(),QColor("red"));
        QCOMPARE(rect->border()->width(),2);

        rect->setState("blue");
        QCOMPARE(rect->color(),QColor("blue"));
        QCOMPARE(rect->border()->width(),1);

    }
}

void tst_states::basicExtension()
{
    QmlEngine engine;

    {
        QmlComponent rectComponent(&engine, SRCDIR "/data/basicExtension.qml");
        QmlGraphicsRectangle *rect = qobject_cast<QmlGraphicsRectangle*>(rectComponent.create());
        QVERIFY(rect != 0);

        QCOMPARE(rect->color(),QColor("red"));
        QCOMPARE(rect->border()->width(),1);

        rect->setState("blue");
        QCOMPARE(rect->color(),QColor("blue"));
        QCOMPARE(rect->border()->width(),1);

        rect->setState("bordered");
        QCOMPARE(rect->color(),QColor("blue"));
        QCOMPARE(rect->border()->width(),2);

        rect->setState("blue");
        QCOMPARE(rect->color(),QColor("blue"));
        QCOMPARE(rect->border()->width(),1);

        rect->setState("");
        QCOMPARE(rect->color(),QColor("red"));
        QCOMPARE(rect->border()->width(),1);

        rect->setState("bordered");
        QCOMPARE(rect->color(),QColor("blue"));
        QCOMPARE(rect->border()->width(),2);

        rect->setState("");
        QCOMPARE(rect->color(),QColor("red"));
        QCOMPARE(rect->border()->width(),1);
    }

    {
        QmlComponent rectComponent(&engine, SRCDIR "/data/fakeExtension.qml");
        QmlGraphicsRectangle *rect = qobject_cast<QmlGraphicsRectangle*>(rectComponent.create());
        QVERIFY(rect != 0);

        QCOMPARE(rect->color(),QColor("red"));

        rect->setState("blue");
        QCOMPARE(rect->color(),QColor("blue"));

        rect->setState("green");
        QCOMPARE(rect->color(),QColor("green"));

        rect->setState("blue");
        QCOMPARE(rect->color(),QColor("blue"));

        rect->setState("green");
        QCOMPARE(rect->color(),QColor("green"));

        rect->setState("");
        QCOMPARE(rect->color(),QColor("red"));

        rect->setState("green");
        QCOMPARE(rect->color(),QColor("green"));
    }
}

void tst_states::basicBinding()
{
    QmlEngine engine;

    {
        QmlComponent rectComponent(&engine, SRCDIR "/data/basicBinding.qml");
        QmlGraphicsRectangle *rect = qobject_cast<QmlGraphicsRectangle*>(rectComponent.create());
        QVERIFY(rect != 0);

        QCOMPARE(rect->color(),QColor("red"));

        rect->setState("blue");
        QCOMPARE(rect->color(),QColor("blue"));

        rect->setState("");
        QCOMPARE(rect->color(),QColor("red"));

        rect->setState("blue");
        QCOMPARE(rect->color(),QColor("blue"));
        rect->setProperty("sourceColor", QColor("green"));
        QCOMPARE(rect->color(),QColor("green"));

        rect->setState("");
        QCOMPARE(rect->color(),QColor("red"));
        rect->setProperty("sourceColor", QColor("yellow"));
        QCOMPARE(rect->color(),QColor("red"));

        rect->setState("blue");
        QCOMPARE(rect->color(),QColor("yellow"));
    }

    {
        QmlComponent rectComponent(&engine, SRCDIR "/data/basicBinding2.qml");
        QmlGraphicsRectangle *rect = qobject_cast<QmlGraphicsRectangle*>(rectComponent.create());
        QVERIFY(rect != 0);

        QCOMPARE(rect->color(),QColor("red"));

        rect->setState("blue");
        QCOMPARE(rect->color(),QColor("blue"));

        rect->setState("");
        QCOMPARE(rect->color(),QColor("red"));

        rect->setState("blue");
        QCOMPARE(rect->color(),QColor("blue"));
        rect->setProperty("sourceColor", QColor("green"));
        QCOMPARE(rect->color(),QColor("blue"));

        rect->setState("");
        QCOMPARE(rect->color(),QColor("green"));
        rect->setProperty("sourceColor", QColor("yellow"));
        QCOMPARE(rect->color(),QColor("yellow"));

        rect->setState("blue");
        QCOMPARE(rect->color(),QColor("blue"));

        rect->setState("");
        QCOMPARE(rect->color(),QColor("yellow"));
    }

    {
        QmlComponent rectComponent(&engine, SRCDIR "/data/basicBinding3.qml");
        QmlGraphicsRectangle *rect = qobject_cast<QmlGraphicsRectangle*>(rectComponent.create());
        QVERIFY(rect != 0);

        QCOMPARE(rect->color(),QColor("red"));
        rect->setProperty("sourceColor", QColor("green"));
        QCOMPARE(rect->color(),QColor("green"));

        rect->setState("blue");
        QCOMPARE(rect->color(),QColor("blue"));
        rect->setProperty("sourceColor", QColor("red"));
        QCOMPARE(rect->color(),QColor("blue"));
        rect->setProperty("sourceColor2", QColor("yellow"));
        QCOMPARE(rect->color(),QColor("yellow"));

        rect->setState("");
        QCOMPARE(rect->color(),QColor("red"));
        rect->setProperty("sourceColor2", QColor("green"));
        QCOMPARE(rect->color(),QColor("red"));
        rect->setProperty("sourceColor", QColor("yellow"));
        QCOMPARE(rect->color(),QColor("yellow"));
    }

    {
        QmlComponent rectComponent(&engine, SRCDIR "/data/basicBinding4.qml");
        QmlGraphicsRectangle *rect = qobject_cast<QmlGraphicsRectangle*>(rectComponent.create());
        QVERIFY(rect != 0);

        QCOMPARE(rect->color(),QColor("red"));

        rect->setState("blue");
        QCOMPARE(rect->color(),QColor("blue"));
        rect->setProperty("sourceColor", QColor("yellow"));
        QCOMPARE(rect->color(),QColor("yellow"));

        rect->setState("green");
        QCOMPARE(rect->color(),QColor("green"));
        rect->setProperty("sourceColor", QColor("purple"));
        QCOMPARE(rect->color(),QColor("green"));

        rect->setState("blue");
        QCOMPARE(rect->color(),QColor("purple"));

        rect->setState("green");
        QCOMPARE(rect->color(),QColor("green"));

        rect->setState("");
        QCOMPARE(rect->color(),QColor("red"));
    }
}

class MyRect : public QmlGraphicsRectangle
{
   Q_OBJECT
public:
    MyRect() {}
    void doSomething() { emit didSomething(); }
Q_SIGNALS:
    void didSomething();
};

QML_DECLARE_TYPE(MyRect)
QML_DEFINE_TYPE(Qt.test, 1, 0, MyRectangle,MyRect);

void tst_states::signalOverride()
{
    QmlEngine engine;

    {
        QmlComponent rectComponent(&engine, SRCDIR "/data/signalOverride.qml");
        MyRect *rect = qobject_cast<MyRect*>(rectComponent.create());
        QVERIFY(rect != 0);

        QCOMPARE(rect->color(),QColor("red"));
        rect->doSomething();
        QCOMPARE(rect->color(),QColor("blue"));

        rect->setState("green");
        rect->doSomething();
        QCOMPARE(rect->color(),QColor("green"));
    }

    {
        QmlComponent rectComponent(&engine, SRCDIR "/data/signalOverride2.qml");
        MyRect *rect = qobject_cast<MyRect*>(rectComponent.create());
        QVERIFY(rect != 0);

        QCOMPARE(rect->color(),QColor("white"));
        rect->doSomething();
        QCOMPARE(rect->color(),QColor("blue"));

        QmlGraphicsRectangle *innerRect = qobject_cast<QmlGraphicsRectangle*>(rect->findChild<QmlGraphicsRectangle*>("extendedRect"));

        innerRect->setState("green");
        rect->doSomething();
        QCOMPARE(rect->color(),QColor("blue"));
        QCOMPARE(innerRect->color(),QColor("green"));
        QCOMPARE(innerRect->property("extendedColor").value<QColor>(),QColor("green"));
    }
}

void tst_states::signalOverrideCrash()
{
    QmlEngine engine;

    QmlComponent rectComponent(&engine, SRCDIR "/data/signalOverrideCrash.qml");
    MyRect *rect = qobject_cast<MyRect*>(rectComponent.create());
    QVERIFY(rect != 0);

    rect->setState("overridden");
    rect->doSomething();
}

void tst_states::parentChange()
{
    QmlEngine engine;

    {
        QmlComponent rectComponent(&engine, SRCDIR "/data/parentChange.qml");
        QmlGraphicsRectangle *rect = qobject_cast<QmlGraphicsRectangle*>(rectComponent.create());
        QVERIFY(rect != 0);

        QmlGraphicsRectangle *innerRect = qobject_cast<QmlGraphicsRectangle*>(rect->findChild<QmlGraphicsRectangle*>("MyRect"));
        QVERIFY(innerRect != 0);

        QmlParentChange *pChange = qobject_cast<QmlParentChange*>(rect->states()->at(0)->changes()->at(0));
        QVERIFY(pChange != 0);
        QmlGraphicsItem *nParent = qobject_cast<QmlGraphicsItem*>(rect->findChild<QmlGraphicsItem*>("NewParent"));
        QVERIFY(nParent != 0);

        QCOMPARE(pChange->parent(), nParent);

        rect->setState("reparented");
        QCOMPARE(innerRect->rotation(), qreal(0));
        QCOMPARE(innerRect->scale(), qreal(1));
        QCOMPARE(innerRect->x(), qreal(-133));
        QCOMPARE(innerRect->y(), qreal(-300));
    }

    {
        QmlComponent rectComponent(&engine, SRCDIR "/data/parentChange2.qml");
        QmlGraphicsRectangle *rect = qobject_cast<QmlGraphicsRectangle*>(rectComponent.create());
        QVERIFY(rect != 0);

        QmlGraphicsRectangle *innerRect = qobject_cast<QmlGraphicsRectangle*>(rect->findChild<QmlGraphicsRectangle*>("MyRect"));
        QVERIFY(innerRect != 0);

        rect->setState("reparented");
        QCOMPARE(innerRect->rotation(), qreal(15));
        QCOMPARE(innerRect->scale(), qreal(.5));
        QCOMPARE(QString("%1").arg(innerRect->x()), QString("%1").arg(12.4148145657));
        QCOMPARE(QString("%1").arg(innerRect->y()), QString("%1").arg(10.6470476128));
    }

    {
        QmlComponent rectComponent(&engine, SRCDIR "/data/parentChange3.qml");
        QmlGraphicsRectangle *rect = qobject_cast<QmlGraphicsRectangle*>(rectComponent.create());
        QVERIFY(rect != 0);

        QmlGraphicsRectangle *innerRect = qobject_cast<QmlGraphicsRectangle*>(rect->findChild<QmlGraphicsRectangle*>("MyRect"));
        QVERIFY(innerRect != 0);

        rect->setState("reparented");
        QCOMPARE(innerRect->rotation(), qreal(-37));
        QCOMPARE(innerRect->scale(), qreal(.25));
        QCOMPARE(QString("%1").arg(innerRect->x()), QString("%1").arg(-217.305));
        QCOMPARE(QString("%1").arg(innerRect->y()), QString("%1").arg(-164.413));

        rect->setState("");
        QCOMPARE(innerRect->rotation(), qreal(0));
        QCOMPARE(innerRect->scale(), qreal(1));
        QCOMPARE(innerRect->x(), qreal(5));
        QCOMPARE(innerRect->y(), qreal(0));
    }
}

void tst_states::parentChangeErrors()
{
    QmlEngine engine;

    {
        QmlComponent rectComponent(&engine, SRCDIR "/data/parentChange4.qml");
        QmlGraphicsRectangle *rect = qobject_cast<QmlGraphicsRectangle*>(rectComponent.create());
        QVERIFY(rect != 0);

        QmlGraphicsRectangle *innerRect = qobject_cast<QmlGraphicsRectangle*>(rect->findChild<QmlGraphicsRectangle*>("MyRect"));
        QVERIFY(innerRect != 0);

        QTest::ignoreMessage(QtWarningMsg, "QML QmlParentChange (file://" SRCDIR "/data/parentChange4.qml:25:9) Unable to preserve appearance under non-uniform scale");
        rect->setState("reparented");
        QCOMPARE(innerRect->rotation(), qreal(0));
        QCOMPARE(innerRect->scale(), qreal(1));
        QCOMPARE(innerRect->x(), qreal(5));
        QCOMPARE(innerRect->y(), qreal(5));
    }

    {
        QmlComponent rectComponent(&engine, SRCDIR "/data/parentChange5.qml");
        QmlGraphicsRectangle *rect = qobject_cast<QmlGraphicsRectangle*>(rectComponent.create());
        QVERIFY(rect != 0);

        QmlGraphicsRectangle *innerRect = qobject_cast<QmlGraphicsRectangle*>(rect->findChild<QmlGraphicsRectangle*>("MyRect"));
        QVERIFY(innerRect != 0);

        QTest::ignoreMessage(QtWarningMsg, "QML QmlParentChange (file://" SRCDIR "/data/parentChange5.qml:25:9) Unable to preserve appearance under complex transform");
        rect->setState("reparented");
        QCOMPARE(innerRect->rotation(), qreal(0));
        QCOMPARE(innerRect->scale(), qreal(1));
        QCOMPARE(innerRect->x(), qreal(5));
        QCOMPARE(innerRect->y(), qreal(5));
    }
}

void tst_states::anchorChanges()
{
    QmlEngine engine;

    QmlComponent rectComponent(&engine, SRCDIR "/data/anchorChanges.qml");
    QmlGraphicsRectangle *rect = qobject_cast<QmlGraphicsRectangle*>(rectComponent.create());
    QVERIFY(rect != 0);

    QmlGraphicsRectangle *innerRect = qobject_cast<QmlGraphicsRectangle*>(rect->findChild<QmlGraphicsRectangle*>("MyRect"));
    QVERIFY(innerRect != 0);

    QmlAnchorChanges *aChanges = qobject_cast<QmlAnchorChanges*>(rect->states()->at(0)->changes()->at(0));
    QVERIFY(aChanges != 0);

    rect->setState("right");
    QCOMPARE(innerRect->x(), qreal(150));
    QCOMPARE(aChanges->reset(), QString("left"));
    QCOMPARE(aChanges->object(), innerRect);
    QCOMPARE(aChanges->right().item, rect->right().item);
    QCOMPARE(aChanges->right().anchorLine, rect->right().anchorLine);

    rect->setState("");
    QCOMPARE(innerRect->x(), qreal(5));

    delete rect;
}

void tst_states::anchorChanges2()
{
    QmlEngine engine;

    QmlComponent rectComponent(&engine, SRCDIR "/data/anchorChanges2.qml");
    QmlGraphicsRectangle *rect = qobject_cast<QmlGraphicsRectangle*>(rectComponent.create());
    QVERIFY(rect != 0);

    QmlGraphicsRectangle *innerRect = qobject_cast<QmlGraphicsRectangle*>(rect->findChild<QmlGraphicsRectangle*>("MyRect"));
    QVERIFY(innerRect != 0);

    rect->setState("right");
    QEXPECT_FAIL("", "QTBUG-5338", Continue);
    QCOMPARE(innerRect->x(), qreal(150));

    rect->setState("");
    QCOMPARE(innerRect->x(), qreal(5));

    delete rect;
}

void tst_states::anchorChanges3()
{
    QmlEngine engine;

    QmlComponent rectComponent(&engine, SRCDIR "/data/anchorChanges3.qml");
    QmlGraphicsRectangle *rect = qobject_cast<QmlGraphicsRectangle*>(rectComponent.create());
    QVERIFY(rect != 0);

    QmlGraphicsRectangle *innerRect = qobject_cast<QmlGraphicsRectangle*>(rect->findChild<QmlGraphicsRectangle*>("MyRect"));
    QVERIFY(innerRect != 0);

    QmlGraphicsItem *leftGuideline = qobject_cast<QmlGraphicsItem*>(rect->findChild<QmlGraphicsItem*>("LeftGuideline"));
    QVERIFY(leftGuideline != 0);

    QmlGraphicsItem *bottomGuideline = qobject_cast<QmlGraphicsItem*>(rect->findChild<QmlGraphicsItem*>("BottomGuideline"));
    QVERIFY(bottomGuideline != 0);

    QmlAnchorChanges *aChanges = qobject_cast<QmlAnchorChanges*>(rect->states()->at(0)->changes()->at(0));
    QVERIFY(aChanges != 0);

    rect->setState("reanchored");
    QCOMPARE(aChanges->object(), innerRect);
    QCOMPARE(aChanges->left().item, leftGuideline->left().item);
    QCOMPARE(aChanges->left().anchorLine, leftGuideline->left().anchorLine);
    QCOMPARE(aChanges->right().item, rect->right().item);
    QCOMPARE(aChanges->right().anchorLine, rect->right().anchorLine);
    QCOMPARE(aChanges->top().item, rect->top().item);
    QCOMPARE(aChanges->top().anchorLine, rect->top().anchorLine);
    QCOMPARE(aChanges->bottom().item, bottomGuideline->bottom().item);
    QCOMPARE(aChanges->bottom().anchorLine, bottomGuideline->bottom().anchorLine);

    QCOMPARE(innerRect->x(), qreal(10));
    QCOMPARE(innerRect->y(), qreal(0));
    QCOMPARE(innerRect->width(), qreal(190));
    QCOMPARE(innerRect->height(), qreal(150));

    rect->setState("");
    QCOMPARE(innerRect->x(), qreal(0));
    QCOMPARE(innerRect->y(), qreal(10));
    QCOMPARE(innerRect->width(), qreal(150));
    QCOMPARE(innerRect->height(), qreal(190));

    delete rect;
}

void tst_states::anchorChanges4()
{
    QmlEngine engine;

    QmlComponent rectComponent(&engine, SRCDIR "/data/anchorChanges4.qml");
    QmlGraphicsRectangle *rect = qobject_cast<QmlGraphicsRectangle*>(rectComponent.create());
    QVERIFY(rect != 0);

    QmlGraphicsRectangle *innerRect = qobject_cast<QmlGraphicsRectangle*>(rect->findChild<QmlGraphicsRectangle*>("MyRect"));
    QVERIFY(innerRect != 0);

    QmlGraphicsItem *leftGuideline = qobject_cast<QmlGraphicsItem*>(rect->findChild<QmlGraphicsItem*>("LeftGuideline"));
    QVERIFY(leftGuideline != 0);

    QmlGraphicsItem *bottomGuideline = qobject_cast<QmlGraphicsItem*>(rect->findChild<QmlGraphicsItem*>("BottomGuideline"));
    QVERIFY(bottomGuideline != 0);

    QmlAnchorChanges *aChanges = qobject_cast<QmlAnchorChanges*>(rect->states()->at(0)->changes()->at(0));
    QVERIFY(aChanges != 0);

    rect->setState("reanchored");
    QCOMPARE(aChanges->object(), innerRect);
    QCOMPARE(aChanges->horizontalCenter().item, bottomGuideline->horizontalCenter().item);
    QCOMPARE(aChanges->horizontalCenter().anchorLine, bottomGuideline->horizontalCenter().anchorLine);
    QCOMPARE(aChanges->verticalCenter().item, leftGuideline->verticalCenter().item);
    QCOMPARE(aChanges->verticalCenter().anchorLine, leftGuideline->verticalCenter().anchorLine);

    delete rect;
}

void tst_states::anchorChanges5()
{
    QmlEngine engine;

    QmlComponent rectComponent(&engine, SRCDIR "/data/anchorChanges5.qml");
    QmlGraphicsRectangle *rect = qobject_cast<QmlGraphicsRectangle*>(rectComponent.create());
    QVERIFY(rect != 0);

    QmlGraphicsRectangle *innerRect = qobject_cast<QmlGraphicsRectangle*>(rect->findChild<QmlGraphicsRectangle*>("MyRect"));
    QVERIFY(innerRect != 0);

    QmlGraphicsItem *leftGuideline = qobject_cast<QmlGraphicsItem*>(rect->findChild<QmlGraphicsItem*>("LeftGuideline"));
    QVERIFY(leftGuideline != 0);

    QmlGraphicsItem *bottomGuideline = qobject_cast<QmlGraphicsItem*>(rect->findChild<QmlGraphicsItem*>("BottomGuideline"));
    QVERIFY(bottomGuideline != 0);

    QmlAnchorChanges *aChanges = qobject_cast<QmlAnchorChanges*>(rect->states()->at(0)->changes()->at(0));
    QVERIFY(aChanges != 0);

    rect->setState("reanchored");
    QCOMPARE(aChanges->object(), innerRect);
    QCOMPARE(aChanges->horizontalCenter().item, bottomGuideline->horizontalCenter().item);
    QCOMPARE(aChanges->horizontalCenter().anchorLine, bottomGuideline->horizontalCenter().anchorLine);
    QCOMPARE(aChanges->baseline().item, leftGuideline->baseline().item);
    QCOMPARE(aChanges->baseline().anchorLine, leftGuideline->baseline().anchorLine);

    delete rect;
}

void tst_states::script()
{
    QmlEngine engine;

    {
        QmlComponent rectComponent(&engine, SRCDIR "/data/script.qml");
        QmlGraphicsRectangle *rect = qobject_cast<QmlGraphicsRectangle*>(rectComponent.create());
        QVERIFY(rect != 0);

        QCOMPARE(rect->color(),QColor("red"));

        rect->setState("blue");
        QCOMPARE(rect->color(),QColor("blue"));

        rect->setState("");
        QCOMPARE(rect->color(),QColor("blue")); // a script isn't reverted
    }
}

void tst_states::restoreEntryValues()
{
    QmlEngine engine;

    QmlComponent rectComponent(&engine, SRCDIR "/data/restoreEntryValues.qml");
    QmlGraphicsRectangle *rect = qobject_cast<QmlGraphicsRectangle*>(rectComponent.create());
    QVERIFY(rect != 0);

    QCOMPARE(rect->color(),QColor("red"));

    rect->setState("blue");
    QCOMPARE(rect->color(),QColor("blue"));

    rect->setState("");
    QCOMPARE(rect->color(),QColor("blue"));
}

void tst_states::explicitChanges()
{
    QmlEngine engine;

    QmlComponent rectComponent(&engine, SRCDIR "/data/explicit.qml");
    QmlGraphicsRectangle *rect = qobject_cast<QmlGraphicsRectangle*>(rectComponent.create());
    QVERIFY(rect != 0);

    QmlPropertyChanges *changes = qobject_cast<QmlPropertyChanges*>(rect->findChild<QmlPropertyChanges*>("changes"));
    QVERIFY(changes != 0);
    QVERIFY(changes->isExplicit());

    QCOMPARE(rect->color(),QColor("red"));

    rect->setState("blue");
    QCOMPARE(rect->color(),QColor("blue"));

    rect->setProperty("sourceColor", QColor("green"));
    QCOMPARE(rect->color(),QColor("blue"));

    rect->setState("");
    QCOMPARE(rect->color(),QColor("red"));
    rect->setProperty("sourceColor", QColor("yellow"));
    QCOMPARE(rect->color(),QColor("red"));

    rect->setState("blue");
    QCOMPARE(rect->color(),QColor("yellow"));
}

void tst_states::propertyErrors()
{
    QmlEngine engine;
    QmlComponent rectComponent(&engine, SRCDIR "/data/propertyErrors.qml");
    QmlGraphicsRectangle *rect = qobject_cast<QmlGraphicsRectangle*>(rectComponent.create());
    QVERIFY(rect != 0);

    QCOMPARE(rect->color(),QColor("red"));

    QTest::ignoreMessage(QtWarningMsg, "QML QmlPropertyChanges (file://" SRCDIR "/data/propertyErrors.qml:8:9) Cannot assign to non-existant property \"colr\"");
    QTest::ignoreMessage(QtWarningMsg, "QML QmlPropertyChanges (file://" SRCDIR "/data/propertyErrors.qml:8:9) Cannot assign to read-only property \"wantsFocus\"");
    rect->setState("blue");
}

QTEST_MAIN(tst_states)

#include "tst_states.moc"
