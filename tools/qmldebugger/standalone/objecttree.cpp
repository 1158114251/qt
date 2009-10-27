#include <QtGui/qevent.h>
#include <QtGui/qmenu.h>
#include <QtGui/qaction.h>

#include <QInputDialog>

#include <QtDeclarative/qmldebugservice.h>
#include <QtDeclarative/qmldebug.h>
#include <QtDeclarative/qmldebugclient.h>

#include "objecttree.h"

Q_DECLARE_METATYPE(QmlDebugObjectReference)

ObjectTree::ObjectTree(QmlEngineDebug *client, QWidget *parent)
    : QTreeWidget(parent),
      m_client(client),
      m_query(0)
{
    setHeaderHidden(true);
    setMinimumWidth(250);

    connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
            this, SLOT(currentItemChanged(QTreeWidgetItem *)));
}

void ObjectTree::setEngineDebug(QmlEngineDebug *client)
{
    m_client = client;
}

void ObjectTree::reload(int objectDebugId)
{
    if (!m_client)
        return;
        
    if (m_query) {
        delete m_query;
        m_query = 0;
    }

    m_query = m_client->queryObjectRecursive(QmlDebugObjectReference(objectDebugId), this);
    if (!m_query->isWaiting())
        objectFetched();
    else
        QObject::connect(m_query, SIGNAL(stateChanged(State)), 
                         this, SLOT(objectFetched()));
}

void ObjectTree::setCurrentObject(int debugId)
{
    QTreeWidgetItem *item = findItemByObjectId(debugId);
    if (item) {
        setCurrentItem(item);
        scrollToItem(item);
        item->setExpanded(true);
    }
}

void ObjectTree::objectFetched()
{
    dump(m_query->object(), 0);
    buildTree(m_query->object(), 0);

    delete m_query;
    m_query = 0;
}

void ObjectTree::currentItemChanged(QTreeWidgetItem *item)
{
    if (!item)
        return;

    QmlDebugObjectReference obj = item->data(0, Qt::UserRole).value<QmlDebugObjectReference>();
    if (obj.debugId() < 0) {
        qWarning("QML Object Tree: bad object id");
        return;
    }
    emit currentObjectChanged(obj);
}

void ObjectTree::buildTree(const QmlDebugObjectReference &obj, QTreeWidgetItem *parent)
{
    if (!parent)
        clear();

    QTreeWidgetItem *item = parent ? new QTreeWidgetItem(parent) : new QTreeWidgetItem(this);
    item->setText(0, obj.className());
    item->setData(0, Qt::UserRole, qVariantFromValue(obj));

    if (parent && obj.contextDebugId() >= 0
            && obj.contextDebugId() != parent->data(0, Qt::UserRole
                    ).value<QmlDebugObjectReference>().contextDebugId()) {
        QmlDebugFileReference source = obj.source();
        if (!source.url().isEmpty()) {
            QString toolTipString = QLatin1String("URL: ") + source.url().toString();
            item->setToolTip(0, toolTipString);
        }
        item->setForeground(0, QColor("orange"));
    } else {
        item->setExpanded(true);
    }

    if (obj.contextDebugId() < 0)
        item->setForeground(0, Qt::lightGray);

    for (int ii = 0; ii < obj.children().count(); ++ii)
        buildTree(obj.children().at(ii), item);
}

void ObjectTree::dump(const QmlDebugContextReference &ctxt, int ind)
{
    QByteArray indent(ind * 4, ' ');
    qWarning().nospace() << indent.constData() << ctxt.debugId() << " " 
                         << qPrintable(ctxt.name());

    for (int ii = 0; ii < ctxt.contexts().count(); ++ii)
        dump(ctxt.contexts().at(ii), ind + 1);

    for (int ii = 0; ii < ctxt.objects().count(); ++ii)
        dump(ctxt.objects().at(ii), ind);
}

void ObjectTree::dump(const QmlDebugObjectReference &obj, int ind)
{
    QByteArray indent(ind * 4, ' ');
    qWarning().nospace() << indent.constData() << qPrintable(obj.className())
                         << " " << qPrintable(obj.name()) << " " 
                         << obj.debugId();

    for (int ii = 0; ii < obj.children().count(); ++ii)
        dump(obj.children().at(ii), ind + 1);
}

QTreeWidgetItem *ObjectTree::findItemByObjectId(int debugId) const
{
    for (int i=0; i<topLevelItemCount(); i++) {
        QTreeWidgetItem *item = findItem(topLevelItem(i), debugId);
        if (item)
            return item;
    }

    return 0;
}

QTreeWidgetItem *ObjectTree::findItem(QTreeWidgetItem *item, int debugId) const
{
    if (item->data(0, Qt::UserRole).value<QmlDebugObjectReference>().debugId() == debugId)
        return item;

    QTreeWidgetItem *child;
    for (int i=0; i<item->childCount(); i++) {
        child = findItem(item->child(i), debugId);
        if (child)
            return child;
    }

    return 0;
}

void ObjectTree::mousePressEvent(QMouseEvent *me)
{
    QTreeWidget::mousePressEvent(me);
    if (!currentItem())
        return;
    if(me->button()  == Qt::RightButton && me->type() == QEvent::MouseButtonPress) {
        QAction action(tr("Add watch..."), 0);
        QList<QAction *> actions;
        actions << &action;
        QmlDebugObjectReference obj =
                currentItem()->data(0, Qt::UserRole).value<QmlDebugObjectReference>();
        if (QMenu::exec(actions, me->globalPos())) {
            bool ok = false;
            QString watch = QInputDialog::getText(this, tr("Watch expression"),
                    tr("Expression:"), QLineEdit::Normal, QString(), &ok);
            if (ok && !watch.isEmpty()) 
                emit expressionWatchRequested(obj, watch);
        }
    } 
}
