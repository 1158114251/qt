/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
#include <QmlError>
#include <QDebug>

class tst_qmlerror : public QObject
{
    Q_OBJECT
private slots:
    void url();
    void description();
    void line();
    void column();
    void toString();

    void copy();
    void debug();
};

void tst_qmlerror::url()
{
    QmlError error;

    QCOMPARE(error.url(), QUrl());

    error.setUrl(QUrl("http://www.nokia.com/main.qml"));

    QCOMPARE(error.url(), QUrl("http://www.nokia.com/main.qml"));

    QmlError error2 = error;

    QCOMPARE(error2.url(), QUrl("http://www.nokia.com/main.qml"));

    error.setUrl(QUrl("http://qt.nokia.com/main.qml"));

    QCOMPARE(error.url(), QUrl("http://qt.nokia.com/main.qml"));
    QCOMPARE(error2.url(), QUrl("http://www.nokia.com/main.qml"));
}

void tst_qmlerror::description()
{
    QmlError error;

    QCOMPARE(error.description(), QString());

    error.setDescription("An Error");

    QCOMPARE(error.description(), QString("An Error"));

    QmlError error2 = error;

    QCOMPARE(error2.description(), QString("An Error"));

    error.setDescription("Another Error");

    QCOMPARE(error.description(), QString("Another Error"));
    QCOMPARE(error2.description(), QString("An Error"));
}

void tst_qmlerror::line()
{
    QmlError error;

    QCOMPARE(error.line(), -1);

    error.setLine(102);

    QCOMPARE(error.line(), 102);

    QmlError error2 = error;

    QCOMPARE(error2.line(), 102);

    error.setLine(4);

    QCOMPARE(error.line(), 4);
    QCOMPARE(error2.line(), 102);
}

void tst_qmlerror::column()
{
    QmlError error;

    QCOMPARE(error.column(), -1);

    error.setColumn(16);

    QCOMPARE(error.column(), 16);

    QmlError error2 = error;

    QCOMPARE(error2.column(), 16);

    error.setColumn(3);

    QCOMPARE(error.column(), 3);
    QCOMPARE(error2.column(), 16);
}

void tst_qmlerror::toString()
{
    {
        QmlError error;
        error.setUrl(QUrl("http://www.nokia.com/main.qml"));
        error.setDescription("An Error");
        error.setLine(92);
        error.setColumn(13);

        QCOMPARE(error.toString(), QString("http://www.nokia.com/main.qml:92:13: An Error"));
    }

    {
        QmlError error;
        error.setUrl(QUrl("http://www.nokia.com/main.qml"));
        error.setDescription("An Error");
        error.setLine(92);

        QCOMPARE(error.toString(), QString("http://www.nokia.com/main.qml:92: An Error"));
    }
}

void tst_qmlerror::copy()
{
    QmlError error;
    error.setUrl(QUrl("http://www.nokia.com/main.qml"));
    error.setDescription("An Error");
    error.setLine(92);
    error.setColumn(13);

    QmlError error2(error);
    QmlError error3;
    error3 = error;

    error.setUrl(QUrl("http://qt.nokia.com/main.qml"));
    error.setDescription("Another Error");
    error.setLine(2);
    error.setColumn(33);

    QCOMPARE(error.url(), QUrl("http://qt.nokia.com/main.qml"));
    QCOMPARE(error.description(), QString("Another Error"));
    QCOMPARE(error.line(), 2);
    QCOMPARE(error.column(), 33);

    QCOMPARE(error2.url(), QUrl("http://www.nokia.com/main.qml"));
    QCOMPARE(error2.description(), QString("An Error"));
    QCOMPARE(error2.line(), 92);
    QCOMPARE(error2.column(), 13);

    QCOMPARE(error3.url(), QUrl("http://www.nokia.com/main.qml"));
    QCOMPARE(error3.description(), QString("An Error"));
    QCOMPARE(error3.line(), 92);
    QCOMPARE(error3.column(), 13);

}

void tst_qmlerror::debug()
{
    {
        QmlError error;
        error.setUrl(QUrl("http://www.nokia.com/main.qml"));
        error.setDescription("An Error");
        error.setLine(92);
        error.setColumn(13);

        QTest::ignoreMessage(QtWarningMsg, "http://www.nokia.com/main.qml:92:13: An Error ");
        qWarning() << error;
    }

    {
        QUrl url(QUrl::fromLocalFile(QString(SRCDIR) + "/").resolved(QUrl("test.txt")));
        QmlError error;
        error.setUrl(url);
        error.setDescription("An Error");
        error.setLine(2);
        error.setColumn(5);

        QString out = url.toString() + ":2:5: An Error \n     Line2 Content \n         ^ ";
        QTest::ignoreMessage(QtWarningMsg, qPrintable(out));

        qWarning() << error;
    }

    {
        QUrl url(QUrl::fromLocalFile(QString(SRCDIR) + "/").resolved(QUrl("foo.txt")));
        QmlError error;
        error.setUrl(url);
        error.setDescription("An Error");
        error.setLine(2);
        error.setColumn(5);

        QString out = url.toString() + ":2:5: An Error ";
        QTest::ignoreMessage(QtWarningMsg, qPrintable(out));

        qWarning() << error;
    }
}



QTEST_MAIN(tst_qmlerror)

#include "tst_qmlerror.moc"
