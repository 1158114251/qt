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
#include <QtDeclarative/qmldom.h>

#include <QtCore/QDebug>
#include <QtCore/QFile>

class tst_qmldom : public QObject
{
    Q_OBJECT
public:
    tst_qmldom() {}

private slots:
    void loadSimple();
    void loadProperties();
    void loadChildObject();
    void loadComposite();
    void loadImports();

    void testValueSource();
    void testValueInterceptor();

private:
    QmlEngine engine;
};


void tst_qmldom::loadSimple()
{
    QByteArray qml = "import Qt 4.6\n"
                      "Item {}";

    QmlDomDocument document;
    QVERIFY(document.load(&engine, qml));
    QVERIFY(document.errors().isEmpty());

    QmlDomObject rootObject = document.rootObject();
    QVERIFY(rootObject.isValid());
    QVERIFY(!rootObject.isComponent());
    QVERIFY(!rootObject.isCustomType());
    QVERIFY(rootObject.objectType() == "Qt/Item");
    QVERIFY(rootObject.objectTypeMajorVersion() == 4);
    QVERIFY(rootObject.objectTypeMinorVersion() == 6);
}

void tst_qmldom::loadProperties()
{
    QByteArray qml = "import Qt 4.6\n"
                     "Item { id : item; x : 300; visible : true }";

    QmlDomDocument document;
    QVERIFY(document.load(&engine, qml));

    QmlDomObject rootObject = document.rootObject();
    QVERIFY(rootObject.isValid());
    QVERIFY(rootObject.objectId() == "item");
    QCOMPARE(rootObject.properties().size(), 3);

    QmlDomProperty xProperty = rootObject.property("x");
    QVERIFY(xProperty.propertyName() == "x");
    QVERIFY(xProperty.value().isLiteral());
    QVERIFY(xProperty.value().toLiteral().literal() == "300");

    QmlDomProperty visibleProperty = rootObject.property("visible");
    QVERIFY(visibleProperty.propertyName() == "visible");
    QVERIFY(visibleProperty.value().isLiteral());
    QVERIFY(visibleProperty.value().toLiteral().literal() == "true");
}

void tst_qmldom::loadChildObject()
{
    QByteArray qml = "import Qt 4.6\n"
                     "Item { Item {} }";

    QmlDomDocument document;
    QVERIFY(document.load(&engine, qml));

    QmlDomObject rootItem = document.rootObject();
    QVERIFY(rootItem.isValid());
    QVERIFY(rootItem.properties().size() == 1);

    QmlDomProperty listProperty = rootItem.properties().at(0);
    QVERIFY(listProperty.isDefaultProperty());
    QVERIFY(listProperty.value().isList());

    QmlDomList list = listProperty.value().toList();
    QVERIFY(list.values().size() == 1);

    QmlDomObject childItem = list.values().first().toObject();
    QVERIFY(childItem.isValid());
    QVERIFY(childItem.objectType() == "Qt/Item");
}

void tst_qmldom::loadComposite()
{
    QFile file(SRCDIR  "/data/top.qml");
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QmlDomDocument document;
    QVERIFY(document.load(&engine, file.readAll(), QUrl::fromLocalFile(file.fileName())));
    QVERIFY(document.errors().isEmpty());

    QmlDomObject rootItem = document.rootObject();
    QVERIFY(rootItem.isValid());
    QCOMPARE(rootItem.objectType(), QByteArray("MyComponent"));
    QCOMPARE(rootItem.properties().size(), 2);

    QmlDomProperty widthProperty = rootItem.property("width");
    QVERIFY(widthProperty.value().isLiteral());

    QmlDomProperty heightProperty = rootItem.property("height");
    QVERIFY(heightProperty.value().isLiteral());
}

void tst_qmldom::testValueSource()
{
    QByteArray qml = "import Qt 4.6\n"
                     "Rectangle { height: SpringFollow { spring: 1.4; damping: .15; source: Math.min(Math.max(-130, value*2.2 - 130), 133); }}";

    QmlEngine freshEngine;
    QmlDomDocument document;
    QVERIFY(document.load(&freshEngine, qml));

    QmlDomObject rootItem = document.rootObject();
    QVERIFY(rootItem.isValid());
    QmlDomProperty heightProperty = rootItem.properties().at(0);
    QVERIFY(heightProperty.propertyName() == "height");
    QVERIFY(heightProperty.value().isValueSource());

    const QmlDomValueValueSource valueSource = heightProperty.value().toValueSource();
    QmlDomObject valueSourceObject = valueSource.object();
    QVERIFY(valueSourceObject.isValid());

    QVERIFY(valueSourceObject.objectType() == "Qt/SpringFollow");
    
    const QmlDomValue springValue = valueSourceObject.property("spring").value();
    QVERIFY(!springValue.isInvalid());
    QVERIFY(springValue.isLiteral());
    QVERIFY(springValue.toLiteral().literal() == "1.4");

    const QmlDomValue sourceValue = valueSourceObject.property("source").value();
    QVERIFY(!sourceValue.isInvalid());
    QVERIFY(sourceValue.isBinding());
    QVERIFY(sourceValue.toBinding().binding() == "Math.min(Math.max(-130, value*2.2 - 130), 133)");
}

void tst_qmldom::testValueInterceptor()
{
    QByteArray qml = "import Qt 4.6\n"
                     "Rectangle { height: Behavior { NumberAnimation { duration: 100 } } }";

    QmlEngine freshEngine;
    QmlDomDocument document;
    QVERIFY(document.load(&freshEngine, qml));

    QmlDomObject rootItem = document.rootObject();
    QVERIFY(rootItem.isValid());
    QmlDomProperty heightProperty = rootItem.properties().at(0);
    QVERIFY(heightProperty.propertyName() == "height");
    QVERIFY(heightProperty.value().isValueInterceptor());

    const QmlDomValueValueInterceptor valueInterceptor = heightProperty.value().toValueInterceptor();
    QmlDomObject valueInterceptorObject = valueInterceptor.object();
    QVERIFY(valueInterceptorObject.isValid());

    QVERIFY(valueInterceptorObject.objectType() == "Qt/Behavior");

    const QmlDomValue animationValue = valueInterceptorObject.property("animation").value();
    QVERIFY(!animationValue.isInvalid());
    QVERIFY(animationValue.isObject());
}

void tst_qmldom::loadImports()
{
    QByteArray qml = "import Qt 4.6\n"
                     "import importlib.sublib 4.7\n"
                     "import importlib.sublib 4.6 as NewFoo\n"
                     "import 'import'\n"
                     "import 'import' as X\n"
                     "Item {}";

    QmlEngine engine;
    engine.addImportPath(SRCDIR "/data");
    QmlDomDocument document;
    QVERIFY(document.load(&engine, qml));

    QCOMPARE(document.imports().size(), 5);

    QmlDomImport import = document.imports().at(0);
    QCOMPARE(import.type(), QmlDomImport::Library);
    QCOMPARE(import.uri(), QLatin1String("Qt"));
    QCOMPARE(import.qualifier(), QString());
    QCOMPARE(import.version(), QLatin1String("4.6"));

    import = document.imports().at(1);
    QCOMPARE(import.type(), QmlDomImport::Library);
    QCOMPARE(import.uri(), QLatin1String("importlib.sublib"));
    QCOMPARE(import.qualifier(), QString());
    QCOMPARE(import.version(), QLatin1String("4.7"));

    import = document.imports().at(2);
    QCOMPARE(import.type(), QmlDomImport::Library);
    QCOMPARE(import.uri(), QLatin1String("importlib.sublib"));
    QCOMPARE(import.qualifier(), QLatin1String("NewFoo"));
    QCOMPARE(import.version(), QLatin1String("4.6"));

    import = document.imports().at(3);
    QCOMPARE(import.type(), QmlDomImport::File);
    QCOMPARE(import.uri(), QLatin1String("import"));
    QCOMPARE(import.qualifier(), QLatin1String(""));
    QCOMPARE(import.version(), QLatin1String(""));

    import = document.imports().at(4);
    QCOMPARE(import.type(), QmlDomImport::File);
    QCOMPARE(import.uri(), QLatin1String("import"));
    QCOMPARE(import.qualifier(), QLatin1String("X"));
    QCOMPARE(import.version(), QLatin1String(""));
}


QTEST_MAIN(tst_qmldom)

#include "tst_qmldom.moc"
