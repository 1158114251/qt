#include <qtest.h>
#include <QtDeclarative/qmlengine.h>
#include <QtDeclarative/qmlcontext.h>
#include <QtDeclarative/qbindablemap.h>
#include <QtDeclarative/qmlcomponent.h>
#include <QSignalSpy>

class tst_QBindableMap : public QObject
{
    Q_OBJECT
public:
    tst_QBindableMap() {}

private slots:
    void insert();
    void clear();
    void changed();
};

void tst_QBindableMap::insert()
{
    QBindableMap map;
    map.setValue(QLatin1String("key1"),100);
    map.setValue(QLatin1String("key2"),200);
    QVERIFY(map.keys().count() == 2);

    QCOMPARE(map.value(QLatin1String("key1")), QVariant(100));
    QCOMPARE(map.value(QLatin1String("key2")), QVariant(200));

    map.setValue(QLatin1String("key1"),"Hello World");
    QCOMPARE(map.value(QLatin1String("key1")), QVariant("Hello World"));
}

void tst_QBindableMap::clear()
{
    QBindableMap map;
    map.setValue(QLatin1String("key1"),100);
    QVERIFY(map.keys().count() == 1);

    QCOMPARE(map.value(QLatin1String("key1")), QVariant(100));

    map.clearValue(QLatin1String("key1"));
    QVERIFY(map.keys().count() == 1);
    QCOMPARE(map.value(QLatin1String("key1")), QVariant());
}

void tst_QBindableMap::changed()
{
    QBindableMap map;
    QSignalSpy spy(&map, SIGNAL(changed(const QString&)));
    map.setValue(QLatin1String("key1"),100);
    map.setValue(QLatin1String("key2"),200);
    QCOMPARE(spy.count(), 0);

    map.clearValue(QLatin1String("key1"));
    QCOMPARE(spy.count(), 0);

    //make changes in QML
    QmlEngine engine;
    QmlContext *ctxt = engine.rootContext();
    ctxt->setProperty("data", &map);
    QmlComponent component(&engine, "<Script script=\"data.key1 = 'Hello World';\"/>");
    component.create();
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toString(),QLatin1String("key1"));
    QCOMPARE(map.value(QLatin1String("key1")), QVariant("Hello World"));
}

QTEST_MAIN(tst_QBindableMap)

#include "tst_qbindablemap.moc"
