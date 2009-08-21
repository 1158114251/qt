#include <QGraphicsWidget>
#include <QGraphicsProxyWidget>
#include <QGraphicsAnchorLayout>
#include <QtGui>

static QGraphicsProxyWidget *createItem(const QSizeF &minimum = QSizeF(100.0, 100.0),
                                   const QSizeF &preferred = QSize(150.0, 100.0),
                                   const QSizeF &maximum = QSizeF(200.0, 100.0),
                                   const QString &name = "0")
{
    QGraphicsProxyWidget *w = new QGraphicsProxyWidget;
    w->setWidget(new QPushButton(name));
    w->setData(0, name);
    w->setMinimumSize(minimum);
    w->setPreferredSize(preferred);
    w->setMaximumSize(maximum);

    return w;
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QGraphicsScene scene;
    scene.setSceneRect(0, 0, 800, 480);

    QSizeF min(30, 100);
    QSizeF pref(210, 100);
    QSizeF max(300, 100);

    QGraphicsProxyWidget *a = createItem(min, pref, max, "A");
    QGraphicsProxyWidget *b = createItem(min, pref, max, "B");
    QGraphicsProxyWidget *c = createItem(min, pref, max, "C");
    QGraphicsProxyWidget *d = createItem(min, pref, max, "D");
    QGraphicsProxyWidget *e = createItem(min, pref, max, "E");
    QGraphicsProxyWidget *f = createItem(QSizeF(30, 50), QSizeF(150, 50), max, "F");
    QGraphicsProxyWidget *g = createItem(QSizeF(30, 50), QSizeF(30, 100), max, "G");

    QGraphicsAnchorLayout *l = new QGraphicsAnchorLayout;

    QGraphicsWidget *w = new QGraphicsWidget(0, Qt::Window);
    w->setPos(20, 20);
    w->setLayout(l);

    // vertical
    l->addAnchor(a, Qt::AnchorTop, l, Qt::AnchorTop);
    l->setAnchorSpacing(a, Qt::AnchorTop, l, Qt::AnchorTop, 0);
    l->addAnchor(b, Qt::AnchorTop, l, Qt::AnchorTop);
    l->setAnchorSpacing(b, Qt::AnchorTop, l, Qt::AnchorTop, 0);

    l->addAnchor(c, Qt::AnchorTop, a, Qt::AnchorBottom);
    l->setAnchorSpacing(c, Qt::AnchorTop, a, Qt::AnchorBottom, 0);
    l->addAnchor(c, Qt::AnchorTop, b, Qt::AnchorBottom);
    l->setAnchorSpacing(c, Qt::AnchorTop, b, Qt::AnchorBottom, 0);
    l->addAnchor(c, Qt::AnchorBottom, d, Qt::AnchorTop);
    l->setAnchorSpacing(c, Qt::AnchorBottom, d, Qt::AnchorTop, 0);
    l->addAnchor(c, Qt::AnchorBottom, e, Qt::AnchorTop);
    l->setAnchorSpacing(c, Qt::AnchorBottom, e, Qt::AnchorTop, 0);

    l->addAnchor(d, Qt::AnchorBottom, l, Qt::AnchorBottom);
    l->setAnchorSpacing(d, Qt::AnchorBottom, l, Qt::AnchorBottom, 0);
    l->addAnchor(e, Qt::AnchorBottom, l, Qt::AnchorBottom);
    l->setAnchorSpacing(e, Qt::AnchorBottom, l, Qt::AnchorBottom, 0);

    l->addAnchor(c, Qt::AnchorTop, f, Qt::AnchorTop);
    l->setAnchorSpacing(c, Qt::AnchorTop, f, Qt::AnchorTop, 0);
    l->addAnchor(c, Qt::AnchorVerticalCenter, f, Qt::AnchorBottom);
    l->setAnchorSpacing(c, Qt::AnchorVerticalCenter, f, Qt::AnchorBottom, 0);
    l->addAnchor(f, Qt::AnchorBottom, g, Qt::AnchorTop);
    l->setAnchorSpacing(f, Qt::AnchorBottom, g, Qt::AnchorTop, 0);
    l->addAnchor(c, Qt::AnchorBottom, g, Qt::AnchorBottom);
    l->setAnchorSpacing(c, Qt::AnchorBottom, g, Qt::AnchorBottom, 0);

    // horizontal
    l->addAnchor(l, Qt::AnchorLeft, a, Qt::AnchorLeft);
    l->setAnchorSpacing(l, Qt::AnchorLeft, a, Qt::AnchorLeft, 0);
    l->addAnchor(l, Qt::AnchorLeft, d, Qt::AnchorLeft);
    l->setAnchorSpacing(l, Qt::AnchorLeft, d, Qt::AnchorLeft, 0);
    l->addAnchor(a, Qt::AnchorRight, b, Qt::AnchorLeft);
    l->setAnchorSpacing(a, Qt::AnchorRight, b, Qt::AnchorLeft, 0);

    l->addAnchor(a, Qt::AnchorRight, c, Qt::AnchorLeft);
    l->setAnchorSpacing(a, Qt::AnchorRight, c, Qt::AnchorLeft, 0);
    l->addAnchor(c, Qt::AnchorRight, e, Qt::AnchorLeft);
    l->setAnchorSpacing(c, Qt::AnchorRight, e, Qt::AnchorLeft, 0);

    l->addAnchor(b, Qt::AnchorRight, l, Qt::AnchorRight);
    l->setAnchorSpacing(b, Qt::AnchorRight, l, Qt::AnchorRight, 0);
    l->addAnchor(e, Qt::AnchorRight, l, Qt::AnchorRight);
    l->setAnchorSpacing(e, Qt::AnchorRight, l, Qt::AnchorRight, 0);
    l->addAnchor(d, Qt::AnchorRight, e, Qt::AnchorLeft);
    l->setAnchorSpacing(d, Qt::AnchorRight, e, Qt::AnchorLeft, 0);

    l->addAnchor(l, Qt::AnchorLeft, f, Qt::AnchorLeft);
    l->setAnchorSpacing(l, Qt::AnchorLeft, f, Qt::AnchorLeft, 0);
    l->addAnchor(l, Qt::AnchorLeft, g, Qt::AnchorLeft);
    l->setAnchorSpacing(l, Qt::AnchorLeft, g, Qt::AnchorLeft, 0);
    l->addAnchor(f, Qt::AnchorRight, g, Qt::AnchorRight);
    l->setAnchorSpacing(f, Qt::AnchorRight, g, Qt::AnchorRight, 0);

    scene.addItem(w);
    scene.setBackgroundBrush(Qt::darkGreen);
    QGraphicsView *view = new QGraphicsView(&scene);
    view->show();

    return app.exec();
}
