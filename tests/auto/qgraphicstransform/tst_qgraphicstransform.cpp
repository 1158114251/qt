/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Nokia Corporation (qt-info@nokia.com)
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
** contact the sales department at http://www.qtsoftware.com/contact.
** $QT_END_LICENSE$
**
****************************************************************************/


#include <QtTest/QtTest>
#include <qgraphicsitem.h>
#include <qgraphicstransform.h>
#include "../../shared/util.h"

class tst_QGraphicsTransform : public QObject {
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void scale();
    void rotation();
    void rotation3d();
};


// This will be called before the first test function is executed.
// It is only called once.
void tst_QGraphicsTransform::initTestCase()
{
}

// This will be called after the last test function is executed.
// It is only called once.
void tst_QGraphicsTransform::cleanupTestCase()
{
}

// This will be called before each test function is executed.
void tst_QGraphicsTransform::init()
{
}

// This will be called after every test function.
void tst_QGraphicsTransform::cleanup()
{
}


void tst_QGraphicsTransform::scale()
{
    QGraphicsScale scale;
    scale.setOrigin(QPointF(10, 10));

    QTransform t;
    scale.applyTo(&t);

    QCOMPARE(t, QTransform());
    QCOMPARE(scale.transform(), QTransform());

    scale.setXScale(10);
    scale.setOrigin(QPointF(0, 0));

    QTransform res;
    res.scale(10, 1);

    QCOMPARE(scale.transform(), res);
    QCOMPARE(scale.transform().map(QPointF(10, 10)), QPointF(100, 10));

    scale.setOrigin(QPointF(10, 10));
    QCOMPARE(scale.transform().map(QPointF(10, 10)), QPointF(10, 10));
    QCOMPARE(scale.transform().map(QPointF(11, 10)), QPointF(20, 10));
}

void tst_QGraphicsTransform::rotation()
{
    QGraphicsRotation rotation;
    rotation.setOrigin(QPointF(10, 10));

    QTransform t;
    rotation.applyTo(&t);

    QCOMPARE(t, QTransform());
    QCOMPARE(rotation.transform(), QTransform());

    rotation.setAngle(40);
    rotation.setOrigin(QPointF(0, 0));

    QTransform res;
    res.rotate(40);

    QCOMPARE(rotation.transform(), res);

    rotation.setOrigin(QPointF(10, 10));
    rotation.setAngle(90);
    QCOMPARE(rotation.transform().map(QPointF(10, 10)), QPointF(10, 10));
    QCOMPARE(rotation.transform().map(QPointF(20, 10)), QPointF(10, 20));
}

void tst_QGraphicsTransform::rotation3d()
{
    QGraphicsRotation3D rotation;
    rotation.setOrigin(QPointF(10, 10));

    QTransform t;
    rotation.applyTo(&t);

    QCOMPARE(t, QTransform());
    QCOMPARE(rotation.transform(), QTransform());

    rotation.setAngle(180);

    QCOMPARE(t, QTransform());
    QCOMPARE(rotation.transform(), QTransform());

    rotation.setOrigin(QPointF(0, 0));

    QCOMPARE(t, QTransform());
    QCOMPARE(rotation.transform(), QTransform());
}


QTEST_MAIN(tst_QGraphicsTransform)
#include "tst_qgraphicstransform.moc"

