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
#include <private/qmlsystempalette_p.h>
#include <qpalette.h>
#include "../../../shared/util.h"

class tst_qmlsystempalette : public QObject

{
    Q_OBJECT
public:
    tst_qmlsystempalette();

private slots:
    void activePalette();
    void inactivePalette();
    void disabledPalette();

private:
    QmlEngine engine;
};

tst_qmlsystempalette::tst_qmlsystempalette()
{
}

void tst_qmlsystempalette::activePalette()
{
    QString componentStr = "import Qt 4.6\nSystemPalette { }";
    QmlComponent component(&engine, componentStr.toLatin1(), QUrl("file://"));
    QmlSystemPalette *object = qobject_cast<QmlSystemPalette*>(component.create());

    QVERIFY(object != 0);

    QPalette palette;
    palette.setCurrentColorGroup(QPalette::Active);
    QCOMPARE(palette.window().color(), object->window());
    QCOMPARE(palette.windowText().color(), object->windowText());
    QCOMPARE(palette.base().color(), object->base());
    QCOMPARE(palette.text().color(), object->text());
    QCOMPARE(palette.alternateBase().color(), object->alternateBase());
    QCOMPARE(palette.button().color(), object->button());
    QCOMPARE(palette.buttonText().color(), object->buttonText());
    QCOMPARE(palette.light().color(), object->light());
    QCOMPARE(palette.midlight().color(), object->midlight());
    QCOMPARE(palette.dark().color(), object->dark());
    QCOMPARE(palette.mid().color(), object->mid());
    QCOMPARE(palette.shadow().color(), object->shadow());
    QCOMPARE(palette.highlight().color(), object->highlight());
    QCOMPARE(palette.highlightedText().color(), object->highlightedText());

    delete object;
}

void tst_qmlsystempalette::inactivePalette()
{
    QString componentStr = "import Qt 4.6\nSystemPalette { colorGroup: SystemPalette.Inactive }";
    QmlComponent component(&engine, componentStr.toLatin1(), QUrl("file://"));
    QmlSystemPalette *object = qobject_cast<QmlSystemPalette*>(component.create());

    QVERIFY(object != 0);

    QPalette palette;
    palette.setCurrentColorGroup(QPalette::Inactive);
    QCOMPARE(palette.window().color(), object->window());
    QCOMPARE(palette.windowText().color(), object->windowText());
    QCOMPARE(palette.base().color(), object->base());
    QCOMPARE(palette.text().color(), object->text());
    QCOMPARE(palette.alternateBase().color(), object->alternateBase());
    QCOMPARE(palette.button().color(), object->button());
    QCOMPARE(palette.buttonText().color(), object->buttonText());
    QCOMPARE(palette.light().color(), object->light());
    QCOMPARE(palette.midlight().color(), object->midlight());
    QCOMPARE(palette.dark().color(), object->dark());
    QCOMPARE(palette.mid().color(), object->mid());
    QCOMPARE(palette.shadow().color(), object->shadow());
    QCOMPARE(palette.highlight().color(), object->highlight());
    QCOMPARE(palette.highlightedText().color(), object->highlightedText());

    delete object;
}

void tst_qmlsystempalette::disabledPalette()
{
    QString componentStr = "import Qt 4.6\nSystemPalette { colorGroup: SystemPalette.Disabled }";
    QmlComponent component(&engine, componentStr.toLatin1(), QUrl("file://"));
    QmlSystemPalette *object = qobject_cast<QmlSystemPalette*>(component.create());

    QVERIFY(object != 0);

    QPalette palette;
    palette.setCurrentColorGroup(QPalette::Disabled);
    QCOMPARE(palette.window().color(), object->window());
    QCOMPARE(palette.windowText().color(), object->windowText());
    QCOMPARE(palette.base().color(), object->base());
    QCOMPARE(palette.text().color(), object->text());
    QCOMPARE(palette.alternateBase().color(), object->alternateBase());
    QCOMPARE(palette.button().color(), object->button());
    QCOMPARE(palette.buttonText().color(), object->buttonText());
    QCOMPARE(palette.light().color(), object->light());
    QCOMPARE(palette.midlight().color(), object->midlight());
    QCOMPARE(palette.dark().color(), object->dark());
    QCOMPARE(palette.mid().color(), object->mid());
    QCOMPARE(palette.shadow().color(), object->shadow());
    QCOMPARE(palette.highlight().color(), object->highlight());
    QCOMPARE(palette.highlightedText().color(), object->highlightedText());

    delete object;
}

QTEST_MAIN(tst_qmlsystempalette)

#include "tst_qmlsystempalette.moc"
