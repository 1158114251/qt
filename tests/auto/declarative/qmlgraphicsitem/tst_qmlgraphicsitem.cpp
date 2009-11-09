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
#include <QtDeclarative/qmlcontext.h>
#include <QtDeclarative/qmlview.h>
#include <qmlgraphicsitem.h>

class tst_QmlGraphicsItem : public QObject

{
    Q_OBJECT
public:
    tst_QmlGraphicsItem();

private slots:
    void keys();
    void keyNavigation();

private:
    template<typename T>
    T *findItem(QmlGraphicsItem *parent, const QString &objectName);
};

class KeysTestObject : public QObject
{
    Q_OBJECT
public:
    KeysTestObject() : mKey(0), mModifiers(0), mForwardedKey(0) {}

    void reset() {
        mKey = 0;
        mText = QString();
        mModifiers = 0;
        mForwardedKey = 0;
    }

public slots:
    void keyPress(int key, QString text, int modifiers) {
        mKey = key;
        mText = text;
        mModifiers = modifiers;
    }
    void keyRelease(int key, QString text, int modifiers) {
        mKey = key;
        mText = text;
        mModifiers = modifiers;
    }
    void forwardedKey(int key) {
        mForwardedKey = key;
    }

public:
    int mKey;
    QString mText;
    int mModifiers;
    int mForwardedKey;

private:
};


tst_QmlGraphicsItem::tst_QmlGraphicsItem()
{
}

void tst_QmlGraphicsItem::keys()
{
    QmlView *canvas = new QmlView(0);
    canvas->setFixedSize(240,320);

    canvas->setUrl(QUrl("file://" SRCDIR "/data/keys.qml"));

    KeysTestObject *testObject = new KeysTestObject;
    canvas->rootContext()->setContextProperty("keysTestObject", testObject);

    canvas->execute();
    canvas->show();
    qApp->processEvents();

    QEvent wa(QEvent::WindowActivate);
    QApplication::sendEvent(canvas, &wa);
    QFocusEvent fe(QEvent::FocusIn);
    QApplication::sendEvent(canvas, &fe);

    QKeyEvent key(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier, "A", false, 1);
    QApplication::sendEvent(canvas, &key);
    QCOMPARE(testObject->mKey, int(Qt::Key_A));
    QCOMPARE(testObject->mForwardedKey, int(Qt::Key_A));
    QCOMPARE(testObject->mText, QLatin1String("A"));
    QVERIFY(testObject->mModifiers == Qt::NoModifier);
    QVERIFY(!key.isAccepted());

    testObject->reset();

    key = QKeyEvent(QEvent::KeyRelease, Qt::Key_A, Qt::ShiftModifier, "A", false, 1);
    QApplication::sendEvent(canvas, &key);
    QCOMPARE(testObject->mKey, int(Qt::Key_A));
    QCOMPARE(testObject->mForwardedKey, int(Qt::Key_A));
    QCOMPARE(testObject->mText, QLatin1String("A"));
    QVERIFY(testObject->mModifiers == Qt::ShiftModifier);
    QVERIFY(key.isAccepted());

    testObject->reset();

    key = QKeyEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier, "", false, 1);
    QApplication::sendEvent(canvas, &key);
    QCOMPARE(testObject->mKey, int(Qt::Key_Return));
    QCOMPARE(testObject->mForwardedKey, int(Qt::Key_Return));
    QCOMPARE(testObject->mText, QLatin1String("Return"));
    QVERIFY(testObject->mModifiers == Qt::NoModifier);
    QVERIFY(key.isAccepted());

    delete canvas;
    delete testObject;
}

void tst_QmlGraphicsItem::keyNavigation()
{
    QmlView *canvas = new QmlView(0);
    canvas->setFixedSize(240,320);

    canvas->setUrl(QUrl("file://" SRCDIR "/data/keynavigation.qml"));
    canvas->execute();
    canvas->show();
    qApp->processEvents();

    QEvent wa(QEvent::WindowActivate);
    QApplication::sendEvent(canvas, &wa);
    QFocusEvent fe(QEvent::FocusIn);
    QApplication::sendEvent(canvas, &fe);

    QmlGraphicsItem *item = findItem<QmlGraphicsItem>(canvas->root(), "item1");
    QVERIFY(item);
    QVERIFY(item->hasFocus());

    // right
    QKeyEvent key(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier, "", false, 1);
    QApplication::sendEvent(canvas, &key);
    QVERIFY(key.isAccepted());

    item = findItem<QmlGraphicsItem>(canvas->root(), "item2");
    QVERIFY(item);
    QVERIFY(item->hasFocus());

    // down
    key = QKeyEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier, "", false, 1);
    QApplication::sendEvent(canvas, &key);
    QVERIFY(key.isAccepted());

    item = findItem<QmlGraphicsItem>(canvas->root(), "item4");
    QVERIFY(item);
    QVERIFY(item->hasFocus());

    // left
    key = QKeyEvent(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier, "", false, 1);
    QApplication::sendEvent(canvas, &key);
    QVERIFY(key.isAccepted());

    item = findItem<QmlGraphicsItem>(canvas->root(), "item3");
    QVERIFY(item);
    QVERIFY(item->hasFocus());

    // up
    key = QKeyEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier, "", false, 1);
    QApplication::sendEvent(canvas, &key);
    QVERIFY(key.isAccepted());

    item = findItem<QmlGraphicsItem>(canvas->root(), "item1");
    QVERIFY(item);
    QVERIFY(item->hasFocus());
}

template<typename T>
T *tst_QmlGraphicsItem::findItem(QmlGraphicsItem *parent, const QString &objectName)
{
    if (!parent)
        return 0;

    const QMetaObject &mo = T::staticMetaObject;
    //qDebug() << parent->QGraphicsObject::children().count() << "children";
    for (int i = 0; i < parent->QGraphicsObject::children().count(); ++i) {
        QmlGraphicsItem *item = qobject_cast<QmlGraphicsItem*>(parent->QGraphicsObject::children().at(i));
        if(!item)
            continue;
        //qDebug() << "try" << item;
        if (mo.cast(item) && (objectName.isEmpty() || item->objectName() == objectName))
            return static_cast<T*>(item);
        item = findItem<T>(item, objectName);
        if (item)
            return static_cast<T*>(item);
    }

    return 0;
}



QTEST_MAIN(tst_QmlGraphicsItem)

#include "tst_qmlgraphicsitem.moc"
