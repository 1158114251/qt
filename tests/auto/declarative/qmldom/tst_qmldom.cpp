#include <qtest.h>
#include <QtDeclarative/qmlengine.h>
#include <QtDeclarative/qmlcomponent.h>
#include <QtDeclarative/qmldom.h>

#include <QtCore/QDebug>

class tst_qmldom : public QObject
{
    Q_OBJECT
public:
    tst_qmldom() {}

private slots:
    void loadSimple();
    void loadProperties();
    void loadChildObject();

    void testValueSource();

private:
    QmlEngine engine;
};


void tst_qmldom::loadSimple()
{
    QByteArray qml = "Item {}";
    //QByteArray qml = "<Item/>";

    QmlDomDocument document;
    QVERIFY(document.load(&engine, qml));
    QVERIFY(document.loadError().isEmpty());

    QmlDomObject rootObject = document.rootObject();
    QVERIFY(rootObject.isValid());
    QVERIFY(!rootObject.isComponent());
    QVERIFY(!rootObject.isCustomType());
    QVERIFY(rootObject.objectType() == "Item");
}

void tst_qmldom::loadProperties()
{
    QByteArray qml = "Item { id : item; x : 300; visible : true }";
    //QByteArray qml = "<Item id='item' x='300' visible='true'/>";

    QmlDomDocument document;
    QVERIFY(document.load(&engine, qml));

    QmlDomObject rootObject = document.rootObject();
    QVERIFY(rootObject.isValid());
    QVERIFY(rootObject.objectId() == "item");
    QVERIFY(rootObject.properties().size() == 2);

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
    QByteArray qml = "Item { Item {} }";
    //QByteArray qml = "<Item> <Item/> </Item>";

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
    QVERIFY(childItem.objectType() == "Item");
}

void tst_qmldom::testValueSource()
{
    QByteArray qml = "Rect { height: Follow { spring: 1.4; damping: .15; source: Math.min(Math.max(-130, value*2.2 - 130), 133); }}";

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

    QVERIFY(valueSourceObject.objectType() == "Follow");
    
    const QmlDomValue springValue = valueSourceObject.property("spring").value();
    QVERIFY(!springValue.isInvalid());
    QVERIFY(springValue.isLiteral());
    QVERIFY(springValue.toLiteral().literal() == "1.4");

    const QmlDomValue sourceValue = valueSourceObject.property("source").value();
    QVERIFY(!sourceValue.isInvalid());
    QVERIFY(sourceValue.isBinding());
    QVERIFY(sourceValue.toBinding().binding() == "Math.min(Math.max(-130, value*2.2 - 130), 133)");
}

QTEST_MAIN(tst_qmldom)

#include "tst_qmldom.moc"
