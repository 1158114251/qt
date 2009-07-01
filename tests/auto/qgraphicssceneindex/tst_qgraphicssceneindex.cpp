/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the test suite of the Qt Toolkit.
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


#include <QtTest/QtTest>
#include <QtGui/qgraphicsscene.h>
#include <private/qgraphicsscenebsptreeindex_p.h>
#include <private/qgraphicssceneindex_p.h>
#include <private/qgraphicsscenelinearindex_p.h>

//TESTED_CLASS=
//TESTED_FILES=

class tst_QGraphicsSceneIndex : public QObject
{
    Q_OBJECT
public slots:
    void initTestCase();

private slots:
    void customIndex_data();
    void customIndex();
    void scatteredItems_data();
    void scatteredItems();
    void overlappedItems_data();
    void overlappedItems();
    void movingItems_data();
    void movingItems();
    void connectedToSceneRectChanged();

private:
    void common_data();
    QGraphicsSceneIndex *createIndex(const QString &name);
};

void tst_QGraphicsSceneIndex::initTestCase()
{
}

void tst_QGraphicsSceneIndex::common_data()
{
    QTest::addColumn<QString>("indexMethod");

    QTest::newRow("BSP") << QString("bsp");
    QTest::newRow("Linear") << QString("linear");
}

QGraphicsSceneIndex *tst_QGraphicsSceneIndex::createIndex(const QString &indexMethod)
{
    QGraphicsSceneIndex *index = 0;
    QGraphicsScene *scene = new QGraphicsScene();
    if (indexMethod == "bsp")
        index = new QGraphicsSceneBspTreeIndex(scene);

    if (indexMethod == "linear")
        index = new QGraphicsSceneLinearIndex(scene);

    return index;
}

void tst_QGraphicsSceneIndex::customIndex_data()
{
    common_data();
}

void tst_QGraphicsSceneIndex::customIndex()
{
#if 0
    QFETCH(QString, indexMethod);
    QGraphicsSceneIndex *index = createIndex(indexMethod);

    QGraphicsScene scene;
    scene.setSceneIndex(index);

    scene.addRect(0, 0, 30, 40);
    QCOMPARE(scene.items(QRectF(0, 0, 10, 10)).count(), 1);
#endif
}

void tst_QGraphicsSceneIndex::scatteredItems_data()
{
    common_data();
}

void tst_QGraphicsSceneIndex::scatteredItems()
{
    QFETCH(QString, indexMethod);

    QGraphicsScene scene;
#if 1
    scene.setItemIndexMethod(indexMethod == "linear" ? QGraphicsScene::NoIndex : QGraphicsScene::BspTreeIndex);
#else
    QGraphicsSceneIndex *index = createIndex(indexMethod);
    scene.setSceneIndex(index);
#endif

    for (int i = 0; i < 10; ++i)
        scene.addRect(i*50, i*50, 40, 35);

    QCOMPARE(scene.items(QPointF(5, 5)).count(), 1);
    QCOMPARE(scene.items(QPointF(55, 55)).count(), 1);
    QCOMPARE(scene.items(QPointF(-100, -100)).count(), 0);

    QCOMPARE(scene.items(QRectF(0, 0, 10, 10)).count(), 1);
    QCOMPARE(scene.items(QRectF(0, 0, 1000, 1000)).count(), 10);
    QCOMPARE(scene.items(QRectF(-100, -1000, 0, 0)).count(), 0);
}

void tst_QGraphicsSceneIndex::overlappedItems_data()
{
    common_data();
}

void tst_QGraphicsSceneIndex::overlappedItems()
{
    QFETCH(QString, indexMethod);

    QGraphicsScene scene;
#if 1
    scene.setItemIndexMethod(indexMethod == "linear" ? QGraphicsScene::NoIndex : QGraphicsScene::BspTreeIndex);
#else
    QGraphicsSceneIndex *index = createIndex(indexMethod);
    scene.setSceneIndex(index);
#endif

    for (int i = 0; i < 10; ++i)
        for (int j = 0; j < 10; ++j)
            scene.addRect(i*50, j*50, 200, 200);

    QCOMPARE(scene.items(QPointF(5, 5)).count(), 1);
    QCOMPARE(scene.items(QPointF(55, 55)).count(), 4);
    QCOMPARE(scene.items(QPointF(105, 105)).count(), 9);
    QCOMPARE(scene.items(QPointF(-100, -100)).count(), 0);

    QCOMPARE(scene.items(QRectF(0, 0, 1000, 1000)).count(), 100);
    QCOMPARE(scene.items(QRectF(-100, -1000, 0, 0)).count(), 0);
    QCOMPARE(scene.items(QRectF(0, 0, 200, 200)).count(), 16);
    QCOMPARE(scene.items(QRectF(0, 0, 100, 100)).count(), 4);
    QCOMPARE(scene.items(QRectF(0, 0, 1, 100)).count(), 2);
    QCOMPARE(scene.items(QRectF(0, 0, 1, 1000)).count(), 10);
}

void tst_QGraphicsSceneIndex::movingItems_data()
{
    common_data();
}

void tst_QGraphicsSceneIndex::movingItems()
{
    QFETCH(QString, indexMethod);

    QGraphicsScene scene;
#if 1
    scene.setItemIndexMethod(indexMethod == "linear" ? QGraphicsScene::NoIndex : QGraphicsScene::BspTreeIndex);
#else
    QGraphicsSceneIndex *index = createIndex(indexMethod);
    scene.setSceneIndex(index);
#endif

    for (int i = 0; i < 10; ++i)
        scene.addRect(i*50, i*50, 40, 35);

    QGraphicsRectItem *box = scene.addRect(0, 0, 10, 10);
    QCOMPARE(scene.items(QPointF(5, 5)).count(), 2);
    QCOMPARE(scene.items(QPointF(-1, -1)).count(), 0);
    QCOMPARE(scene.items(QRectF(0, 0, 5, 5)).count(), 2);

    box->setPos(10, 10);
    QCOMPARE(scene.items(QPointF(9, 9)).count(), 1);
    QCOMPARE(scene.items(QPointF(15, 15)).count(), 2);
    QCOMPARE(scene.items(QRectF(0, 0, 1, 1)).count(), 1);

    box->setPos(-5, -5);
    QCOMPARE(scene.items(QPointF(-1, -1)).count(), 1);
    QCOMPARE(scene.items(QRectF(0, 0, 1, 1)).count(), 2);

    QCOMPARE(scene.items(QRectF(0, 0, 1000, 1000)).count(), 11);
}

void tst_QGraphicsSceneIndex::connectedToSceneRectChanged()
{

    class MyScene : public QGraphicsScene
    { public: using QGraphicsScene::receivers; };

    MyScene scene; // Uses QGraphicsSceneBspTreeIndex by default.
    QCOMPARE(scene.receivers(SIGNAL(sceneRectChanged(const QRectF&))), 1);

    scene.setItemIndexMethod(QGraphicsScene::NoIndex); // QGraphicsSceneLinearIndex
    QCOMPARE(scene.receivers(SIGNAL(sceneRectChanged(const QRectF&))), 1);
}

QTEST_MAIN(tst_QGraphicsSceneIndex)
#include "tst_qgraphicssceneindex.moc"
