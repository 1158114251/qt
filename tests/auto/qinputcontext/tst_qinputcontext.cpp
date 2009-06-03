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
#include "../../shared/util.h"

#include <qinputcontext.h>
#include <qlineedit.h>
#include <qplaintextedit.h>
#include <qlayout.h>
#include <qradiobutton.h>

class tst_QInputContext : public QObject
{
Q_OBJECT

public:
    tst_QInputContext() {}
    virtual ~tst_QInputContext() {}

public slots:
    void initTestCase() {}
    void cleanupTestCase() {}
    void init() {}
    void cleanup() {}
private slots:
    void maximumTextLength();
    void filterMouseEvents();
    void requestSoftwareInputPanel();
    void closeSoftwareInputPanel();
};

void tst_QInputContext::maximumTextLength()
{
    QLineEdit le;

    le.setMaxLength(15);
    QVariant variant = le.inputMethodQuery(Qt::ImMaximumTextLength);
    QVERIFY(variant.isValid());
    QCOMPARE(variant.toInt(), 15);

    QPlainTextEdit pte;
    // For BC/historical reasons, QPlainTextEdit::inputMethodQuery is protected.
    variant = static_cast<QWidget *>(&pte)->inputMethodQuery(Qt::ImMaximumTextLength);
    QVERIFY(!variant.isValid());
}

class QFilterInputContext : public QInputContext
{
public:
    QFilterInputContext() : lastType(QEvent::None) {}
    ~QFilterInputContext() {}

    QString identifierName() { return QString(); }
    QString language() { return QString(); }

    void reset() {}

    bool isComposing() const { return false; }

    bool filterEvent( const QEvent *event )
    {
        lastType = event->type();
        return false;
    }

public:
    QEvent::Type lastType;
};

void tst_QInputContext::filterMouseEvents()
{
    QLineEdit le;
    le.show();
    QApplication::setActiveWindow(&le);

    QFilterInputContext *ic = new QFilterInputContext;
    le.setInputContext(ic);
    QTest::mouseClick(&le, Qt::LeftButton);

    QCOMPARE(ic->lastType, QEvent::MouseButtonRelease);

    le.setInputContext(0);
}

void tst_QInputContext::requestSoftwareInputPanel()
{
    QWidget w;
    QLayout *layout = new QVBoxLayout;
    QLineEdit *le1, *le2;
    le1 = new QLineEdit;
    le2 = new QLineEdit;
    layout->addWidget(le1);
    layout->addWidget(le2);
    w.setLayout(layout);

    QFilterInputContext *ic1, *ic2;
    ic1 = new QFilterInputContext;
    ic2 = new QFilterInputContext;
    le1->setInputContext(ic1);
    le2->setInputContext(ic2);

    w.show();
    QApplication::setActiveWindow(&w);

    // Testing single click panel activation.
    qApp->setAutoSipOnMouseFocus(true);
    QTest::mouseClick(le2, Qt::LeftButton, Qt::NoModifier, QPoint(5, 5));
    QCOMPARE(ic2->lastType, QEvent::RequestSoftwareInputPanel);

    // Testing double click panel activation.
    qApp->setAutoSipOnMouseFocus(false);
    QTest::mouseClick(le1, Qt::LeftButton, Qt::NoModifier, QPoint(5, 5));
    QVERIFY(ic1->lastType != QEvent::RequestSoftwareInputPanel);
    QTest::mouseClick(le1, Qt::LeftButton, Qt::NoModifier, QPoint(5, 5));
    QCOMPARE(ic1->lastType, QEvent::RequestSoftwareInputPanel);

    // Testing right mouse button
    QTest::mouseClick(le1, Qt::RightButton, Qt::NoModifier, QPoint(5, 5));
    QVERIFY(ic1->lastType != QEvent::RequestSoftwareInputPanel);
}

void tst_QInputContext::closeSoftwareInputPanel()
{
    QWidget w;
    QLayout *layout = new QVBoxLayout;
    QLineEdit *le1, *le2;
    QRadioButton *rb;
    le1 = new QLineEdit;
    le2 = new QLineEdit;
    rb = new QRadioButton;
    layout->addWidget(le1);
    layout->addWidget(le2);
    layout->addWidget(rb);
    w.setLayout(layout);

    QFilterInputContext *ic1, *ic2;
    ic1 = new QFilterInputContext;
    ic2 = new QFilterInputContext;
    le1->setInputContext(ic1);
    le2->setInputContext(ic2);

    w.show();
    QApplication::setActiveWindow(&w);

    // Testing that panel doesn't close between two input methods aware widgets.
    QTest::mouseClick(le1, Qt::LeftButton, Qt::NoModifier, QPoint(5, 5));
    QTest::mouseClick(le2, Qt::LeftButton, Qt::NoModifier, QPoint(5, 5));
    QVERIFY(ic2->lastType != QEvent::CloseSoftwareInputPanel);

    // Testing that panel closes when focusing non-aware widget.
    QTest::mouseClick(rb, Qt::LeftButton, Qt::NoModifier, QPoint(5, 5));
    QCOMPARE(ic2->lastType, QEvent::CloseSoftwareInputPanel);
}

QTEST_MAIN(tst_QInputContext)
#include "tst_qinputcontext.moc"
