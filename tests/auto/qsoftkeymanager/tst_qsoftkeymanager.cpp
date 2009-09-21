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

#include "qevent.h"
#include "private/qsoftkeymanager_p.h"

class tst_QSoftKeyManager : public QObject
{
Q_OBJECT

public:
    tst_QSoftKeyManager();
    virtual ~tst_QSoftKeyManager();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:
    void updateSoftKeysCompressed();
};

class EventListener : public QObject
{
public:
    EventListener(QObject *listenTo)
    {
        resetCounts();
        if (listenTo)
            listenTo->installEventFilter(this);
    }

    void resetCounts()
    {
        numUpdateSoftKeys = 0;
    }

    int numUpdateSoftKeys;

protected:
    bool eventFilter(QObject * /*object*/, QEvent *event)
    {
        if (event->type() == QEvent::UpdateSoftKeys)
            numUpdateSoftKeys++;
        return false;
    }
};

tst_QSoftKeyManager::tst_QSoftKeyManager() : QObject()
{
}

tst_QSoftKeyManager::~tst_QSoftKeyManager()
{
}

void tst_QSoftKeyManager::initTestCase()
{
}

void tst_QSoftKeyManager::cleanupTestCase()
{
}

void tst_QSoftKeyManager::init()
{
}

void tst_QSoftKeyManager::cleanup()
{
}

/*
    This tests that we only get one UpdateSoftKeys event even though
    multiple events that trigger soft keys occur.
*/
void tst_QSoftKeyManager::updateSoftKeysCompressed()
{
    QWidget w;
    EventListener listener(qApp);

    QList<QAction *> softKeys;
    for (int i = 0; i < 10; ++i) {
        QAction *action = new QAction("foo", &w);
        action->setSoftKeyRole(QAction::PositiveSoftKey);
        softKeys << action;
    }
    w.addActions(softKeys);

    QApplication::processEvents();

    QVERIFY(listener.numUpdateSoftKeys == 1);
}


QTEST_MAIN(tst_QSoftKeyManager)
#include "tst_qsoftkeymanager.moc"
