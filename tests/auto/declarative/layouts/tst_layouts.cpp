#include <QtTest/QtTest>
#include <qlistmodelinterface.h>
#include <qfxview.h>
#include <qfxrect.h>
#include <qmlexpression.h>

class tst_QFxLayouts : public QObject
{
    Q_OBJECT
public:
    tst_QFxLayouts();

private slots:
    void test_horizontal();
    void test_horizontal_spacing();
    void test_horizontal_margin();
    void test_horizontal_spacing_margin();
    void test_vertical();
    void test_vertical_spacing();
    void test_vertical_margin();
    void test_vertical_spacing_margin();
    void test_grid();
    void test_grid_spacing();
    void test_grid_margin();
    void test_grid_spacing_margin();

private:
    QFxView *createView(const QString &filename);
    template<typename T>
    T *findItem(QFxItem *parent, const QString &id, int index=0);
};

tst_QFxLayouts::tst_QFxLayouts()
{
}

void tst_QFxLayouts::test_horizontal()
{
    QFxView *canvas = createView(SRCDIR "/data/horizontal.xml");

    canvas->execute();
    qApp->processEvents();

    QFxRect *one = findItem<QFxRect>(canvas->root(), "one");
    QVERIFY(one != 0);

    QFxRect *two = findItem<QFxRect>(canvas->root(), "two");
    QVERIFY(two != 0);

    QFxRect *three = findItem<QFxRect>(canvas->root(), "three");
    QVERIFY(three != 0);

    QCOMPARE(one->x(), 0.0);
    QCOMPARE(one->y(), 0.0);
    QCOMPARE(two->x(), 50.0);
    QCOMPARE(two->y(), 0.0);
    QCOMPARE(three->x(), 70.0);
    QCOMPARE(three->y(), 0.0);
}

void tst_QFxLayouts::test_horizontal_spacing()
{
    QFxView *canvas = createView(SRCDIR "/data/horizontal-spacing.xml");

    canvas->execute();
    qApp->processEvents();

    QFxRect *one = findItem<QFxRect>(canvas->root(), "one");
    QVERIFY(one != 0);

    QFxRect *two = findItem<QFxRect>(canvas->root(), "two");
    QVERIFY(two != 0);

    QFxRect *three = findItem<QFxRect>(canvas->root(), "three");
    QVERIFY(three != 0);

    QCOMPARE(one->x(), 0.0);
    QCOMPARE(one->y(), 0.0);
    QCOMPARE(two->x(), 60.0);
    QCOMPARE(two->y(), 0.0);
    QCOMPARE(three->x(), 90.0);
    QCOMPARE(three->y(), 0.0);
}

void tst_QFxLayouts::test_horizontal_margin()
{
    QFxView *canvas = createView(SRCDIR "/data/horizontal-margin.xml");

    canvas->execute();
    qApp->processEvents();

    QFxRect *one = findItem<QFxRect>(canvas->root(), "one");
    QVERIFY(one != 0);

    QFxRect *two = findItem<QFxRect>(canvas->root(), "two");
    QVERIFY(two != 0);

    QFxRect *three = findItem<QFxRect>(canvas->root(), "three");
    QVERIFY(three != 0);

    QCOMPARE(one->x(), 10.0);
    QCOMPARE(one->y(), 10.0);
    QCOMPARE(two->x(), 60.0);
    QCOMPARE(two->y(), 10.0);
    QCOMPARE(three->x(), 80.0);
    QCOMPARE(three->y(), 10.0);
}

void tst_QFxLayouts::test_horizontal_spacing_margin()
{
    QFxView *canvas = createView(SRCDIR "/data/horizontal-spacing-margin.xml");

    canvas->execute();
    qApp->processEvents();

    QFxRect *one = findItem<QFxRect>(canvas->root(), "one");
    QVERIFY(one != 0);

    QFxRect *two = findItem<QFxRect>(canvas->root(), "two");
    QVERIFY(two != 0);

    QFxRect *three = findItem<QFxRect>(canvas->root(), "three");
    QVERIFY(three != 0);

    QCOMPARE(one->x(), 10.0);
    QCOMPARE(one->y(), 10.0);
    QCOMPARE(two->x(), 65.0);
    QCOMPARE(two->y(), 10.0);
    QCOMPARE(three->x(), 90.0);
    QCOMPARE(three->y(), 10.0);
}

void tst_QFxLayouts::test_vertical()
{
    QFxView *canvas = createView(SRCDIR "/data/vertical.xml");

    canvas->execute();
    qApp->processEvents();

    QFxRect *one = findItem<QFxRect>(canvas->root(), "one");
    QVERIFY(one != 0);

    QFxRect *two = findItem<QFxRect>(canvas->root(), "two");
    QVERIFY(two != 0);

    QFxRect *three = findItem<QFxRect>(canvas->root(), "three");
    QVERIFY(three != 0);

    QCOMPARE(one->x(), 0.0);
    QCOMPARE(one->y(), 0.0);
    QCOMPARE(two->x(), 0.0);
    QCOMPARE(two->y(), 50.0);
    QCOMPARE(three->x(), 0.0);
    QCOMPARE(three->y(), 60.0);
}

void tst_QFxLayouts::test_vertical_spacing()
{
    QFxView *canvas = createView(SRCDIR "/data/vertical-spacing.xml");

    canvas->execute();
    qApp->processEvents();

    QFxRect *one = findItem<QFxRect>(canvas->root(), "one");
    QVERIFY(one != 0);

    QFxRect *two = findItem<QFxRect>(canvas->root(), "two");
    QVERIFY(two != 0);

    QFxRect *three = findItem<QFxRect>(canvas->root(), "three");
    QVERIFY(three != 0);

    QCOMPARE(one->x(), 0.0);
    QCOMPARE(one->y(), 0.0);
    QCOMPARE(two->x(), 0.0);
    QCOMPARE(two->y(), 60.0);
    QCOMPARE(three->x(), 0.0);
    QCOMPARE(three->y(), 80.0);
}

void tst_QFxLayouts::test_vertical_margin()
{
    QFxView *canvas = createView(SRCDIR "/data/vertical-margin.xml");

    canvas->execute();
    qApp->processEvents();

    QFxRect *one = findItem<QFxRect>(canvas->root(), "one");
    QVERIFY(one != 0);

    QFxRect *two = findItem<QFxRect>(canvas->root(), "two");
    QVERIFY(two != 0);

    QFxRect *three = findItem<QFxRect>(canvas->root(), "three");
    QVERIFY(three != 0);

    QCOMPARE(one->x(), 10.0);
    QCOMPARE(one->y(), 10.0);
    QCOMPARE(two->x(), 10.0);
    QCOMPARE(two->y(), 60.0);
    QCOMPARE(three->x(), 10.0);
    QCOMPARE(three->y(), 70.0);
}

void tst_QFxLayouts::test_vertical_spacing_margin()
{
    QFxView *canvas = createView(SRCDIR "/data/vertical-spacing-margin.xml");

    canvas->execute();
    qApp->processEvents();

    QFxRect *one = findItem<QFxRect>(canvas->root(), "one");
    QVERIFY(one != 0);

    QFxRect *two = findItem<QFxRect>(canvas->root(), "two");
    QVERIFY(two != 0);

    QFxRect *three = findItem<QFxRect>(canvas->root(), "three");
    QVERIFY(three != 0);

    QCOMPARE(one->x(), 10.0);
    QCOMPARE(one->y(), 10.0);
    QCOMPARE(two->x(), 10.0);
    QCOMPARE(two->y(), 65.0);
    QCOMPARE(three->x(), 10.0);
    QCOMPARE(three->y(), 80.0);
}

void tst_QFxLayouts::test_grid()
{
    QFxView *canvas = createView("data/grid.xml");

    canvas->execute();
    qApp->processEvents();

    QFxRect *one = findItem<QFxRect>(canvas->root(), "one");
    QVERIFY(one != 0);
    QFxRect *two = findItem<QFxRect>(canvas->root(), "two");
    QVERIFY(two != 0);
    QFxRect *three = findItem<QFxRect>(canvas->root(), "three");
    QVERIFY(three != 0);
    QFxRect *four = findItem<QFxRect>(canvas->root(), "four");
    QVERIFY(four != 0);
    QFxRect *five = findItem<QFxRect>(canvas->root(), "five");
    QVERIFY(five != 0);

    QCOMPARE(one->x(), 0.0);
    QCOMPARE(one->y(), 0.0);
    QCOMPARE(two->x(), 50.0);
    QCOMPARE(two->y(), 0.0);
    QCOMPARE(three->x(), 70.0);
    QCOMPARE(three->y(), 0.0);
    QCOMPARE(four->x(), 0.0);
    QCOMPARE(four->y(), 50.0);
    QCOMPARE(five->x(), 50.0);
    QCOMPARE(five->y(), 50.0);
}

void tst_QFxLayouts::test_grid_spacing()
{
    QFxView *canvas = createView("data/grid-spacing.xml");

    canvas->execute();
    qApp->processEvents();

    QFxRect *one = findItem<QFxRect>(canvas->root(), "one");
    QVERIFY(one != 0);
    QFxRect *two = findItem<QFxRect>(canvas->root(), "two");
    QVERIFY(two != 0);
    QFxRect *three = findItem<QFxRect>(canvas->root(), "three");
    QVERIFY(three != 0);
    QFxRect *four = findItem<QFxRect>(canvas->root(), "four");
    QVERIFY(four != 0);
    QFxRect *five = findItem<QFxRect>(canvas->root(), "five");
    QVERIFY(five != 0);

    QCOMPARE(one->x(), 0.0);
    QCOMPARE(one->y(), 0.0);
    QCOMPARE(two->x(), 54.0);
    QCOMPARE(two->y(), 0.0);
    QCOMPARE(three->x(), 78.0);
    QCOMPARE(three->y(), 0.0);
    QCOMPARE(four->x(), 0.0);
    QCOMPARE(four->y(), 54.0);
    QCOMPARE(five->x(), 54.0);
    QCOMPARE(five->y(), 54.0);
}

void tst_QFxLayouts::test_grid_margin()
{
    QFxView *canvas = createView("data/grid-margin.xml");

    canvas->execute();
    qApp->processEvents();

    QFxRect *one = findItem<QFxRect>(canvas->root(), "one");
    QVERIFY(one != 0);
    QFxRect *two = findItem<QFxRect>(canvas->root(), "two");
    QVERIFY(two != 0);
    QFxRect *three = findItem<QFxRect>(canvas->root(), "three");
    QVERIFY(three != 0);
    QFxRect *four = findItem<QFxRect>(canvas->root(), "four");
    QVERIFY(four != 0);
    QFxRect *five = findItem<QFxRect>(canvas->root(), "five");
    QVERIFY(five != 0);

    QCOMPARE(one->x(), 8.0);
    QCOMPARE(one->y(), 8.0);
    QCOMPARE(two->x(), 58.0);
    QCOMPARE(two->y(), 8.0);
    QCOMPARE(three->x(), 78.0);
    QCOMPARE(three->y(), 8.0);
    QCOMPARE(four->x(), 8.0);
    QCOMPARE(four->y(), 58.0);
    QCOMPARE(five->x(), 58.0);
    QCOMPARE(five->y(), 58.0);
}


void tst_QFxLayouts::test_grid_spacing_margin()
{
    QFxView *canvas = createView("data/grid-spacing-margin.xml");

    canvas->execute();
    qApp->processEvents();

    QFxRect *one = findItem<QFxRect>(canvas->root(), "one");
    QVERIFY(one != 0);
    QFxRect *two = findItem<QFxRect>(canvas->root(), "two");
    QVERIFY(two != 0);
    QFxRect *three = findItem<QFxRect>(canvas->root(), "three");
    QVERIFY(three != 0);
    QFxRect *four = findItem<QFxRect>(canvas->root(), "four");
    QVERIFY(four != 0);
    QFxRect *five = findItem<QFxRect>(canvas->root(), "five");
    QVERIFY(five != 0);

    QCOMPARE(one->x(), 8.0);
    QCOMPARE(one->y(), 8.0);
    QCOMPARE(two->x(), 62.0);
    QCOMPARE(two->y(), 8.0);
    QCOMPARE(three->x(), 86.0);
    QCOMPARE(three->y(), 8.0);
    QCOMPARE(four->x(), 8.0);
    QCOMPARE(four->y(), 62.0);
    QCOMPARE(five->x(), 62.0);
    QCOMPARE(five->y(), 62.0);
}

QFxView *tst_QFxLayouts::createView(const QString &filename)
{
    QFxView *canvas = new QFxView(0);

    QFile file(filename);
    file.open(QFile::ReadOnly);
    QString xml = file.readAll();
    canvas->setQml(xml, filename);

    return canvas;
}

/*
   Find an item with the specified id.  If index is supplied then the
   item must also evaluate the {index} expression equal to index
*/
template<typename T>
T *tst_QFxLayouts::findItem(QFxItem *parent, const QString &id, int index)
{
    const QMetaObject &mo = T::staticMetaObject;
    for (int i = 0; i < parent->children()->count(); ++i) {
        QFxItem *item = parent->children()->at(i);
        if (mo.cast(item) && (id.isEmpty() || item->id() == id)) {
            if (index != -1) {
                QmlExpression e(qmlContext(item), "index", item);
                e.setTrackChange(false);
                if (e.value().toInt() == index)
                    return static_cast<T*>(item);
            } else {
                return static_cast<T*>(item);
            }
        }
        item = findItem<T>(item, id, index);
        if (item)
            return static_cast<T*>(item);
    }

    return 0;
}

QTEST_MAIN(tst_QFxLayouts)

#include "tst_layouts.moc"
