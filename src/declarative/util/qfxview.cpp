/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
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

#include "qscriptvalueiterator.h"
#include "qdebug.h"
#include "qtimer.h"
#include "qevent.h"
#include "qdir.h"
#include "qcoreapplication.h"
#include "qfontdatabase.h"
#include "qicon.h"
#include "qurl.h"
#include "qboxlayout.h"
#include "qbasictimer.h"

#include "qmlbindablevalue.h"
#include "qml.h"
#include "qfxitem.h"
#include "qperformancelog.h"
#include "qfxperf.h"

#include "qfxview.h"
#include <QtDeclarative/qmlengine.h>
#include <QtDeclarative/qmlcontext.h>


QT_BEGIN_NAMESPACE
DEFINE_BOOL_CONFIG_OPTION(itemTreeDump, ITEMTREE_DUMP);

static QVariant stringToPixmap(const QString &str)
{
    //XXX need to use correct paths
    return QVariant(QPixmap(str));
}

static QVariant stringToIcon(const QString &str)
{
    //XXX need to use correct paths
    return QVariant(QIcon(str));
}

static QVariant stringToKeySequence(const QString &str)
{
    return QVariant::fromValue(QKeySequence(str));
}

static QVariant stringToUrl(const QString &str)
{
    return QVariant(QUrl(str));
}

class QFxViewPrivate
{
public:
    QFxViewPrivate(QFxView *w)
        : q(w), root(0), component(0) {}

    QFxView *q;
    QFxItem *root;

    QUrl source;
    QString xml;

    QmlEngine engine;
    QmlComponent *component;
    QBasicTimer resizetimer;

    void init();
};

/*!
    \class QFxView
    \brief The QFxView class provides a widget for displaying a Qt Declarative user interface.

    QFxView currently provides a minimal interface for displaying QML
    files, and connecting between QML and C++ Qt objects.

    Typcial usage:
    \code
    ...
    QFxView *view = new QFxView(this);
    vbox->addWidget(view);

    QFile file(fileName);
    file.open(QFile::ReadOnly);
    QString xml = file.readAll();
    view->setXml(xml, fileName);

    QFileInfo fi(file);
    view->setPath(fi.path());

    view->execute();
    ...
    \endcode
*/

/*!
  \fn QFxView::QFxView(QWidget *parent)
  
  Constructs a QFxView with the given \a parent.
*/
QFxView::QFxView(QWidget *parent)
: QSimpleCanvas(parent), d(new QFxViewPrivate(this))
{
    d->init();
}

/*!
  \fn QFxView::QFxView(QSimpleCanvas::CanvasMode mode, QWidget *parent)
  
  Constructs a QFxView with the given \a parent. The canvas
  \a mode can be QSimpleCanvas::GraphicsView or
  QSimpleCanvas::SimpleCanvas.
*/
QFxView::QFxView(QSimpleCanvas::CanvasMode mode, QWidget *parent)
: QSimpleCanvas(mode, parent), d(new QFxViewPrivate(this))
{
    d->init();
}

void QFxViewPrivate::init()
{
    // XXX: These need to be put in a central location for this kind of thing
    qRegisterMetaType<QFxAnchorLine>("QFxAnchorLine");

    QmlMetaType::registerCustomStringConverter(QVariant::Pixmap, &stringToPixmap);
    QmlMetaType::registerCustomStringConverter(QVariant::Icon, &stringToIcon);
    QmlMetaType::registerCustomStringConverter(QVariant::KeySequence, &stringToKeySequence);
    QmlMetaType::registerCustomStringConverter(QVariant::Url, &stringToUrl);

#ifdef Q_ENABLE_PERFORMANCE_LOG
    QFxPerfTimer<QFxPerf::FontDatabase> perf;
#endif
    QFontDatabase database;
}

/*!
  The destructor clears the instance and deletes the internal
  representation.

  \sa clearItems()
 */
QFxView::~QFxView()
{
    clearItems();
    delete d; d = 0;
}

/*!
  Sets the source to the \a url. The XML string is set to
  empty.
 */
void QFxView::setUrl(const QUrl& url)
{
    d->source = url;
    d->xml = QString();
}

/*!
  Sets the source to the URL from the \a filename, and sets
  the XML string to \a xml.
 */
void QFxView::setXml(const QString &xml, const QString &filename)
{
    d->source = QUrl::fromLocalFile(filename);
    d->xml = xml;
}

/*!
  Returns the XML string.
 */
QString QFxView::xml() const
{
    return d->xml;
}

/*!
  Returns a pointer to the QmlEngine.
 */
QmlEngine* QFxView::engine()
{
    return &d->engine;
}

QmlContext* QFxView::rootContext()
{
    return d->engine.rootContext();
}

void QFxView::execute()
{
    rootContext()->activate();

    if (d->xml.isEmpty()) {
        d->component = new QmlComponent(&d->engine, d->source, this);
    } else {
        d->component = new QmlComponent(&d->engine, d->xml.toUtf8(), d->source);
    }

    if (!d->component->isLoading()) {
        continueExecute();
    } else {
        connect(d->component, SIGNAL(statusChanged(QmlComponent::Status)), this, SLOT(continueExecute()));
    }
}

void QFxView::continueExecute()
{
    disconnect(d->component, SIGNAL(statusChanged(QmlComponent::Status)), this, SLOT(continueExecute()));

    if (!d->component){
        qWarning() << "Error in loading" << d->source;
        return;
    }

    QObject *obj = d->component->create();
    rootContext()->deactivate();
    if (obj) {
        if (QFxItem *item = qobject_cast<QFxItem *>(obj)) {
            item->QSimpleCanvasItem::setParent(QSimpleCanvas::root());

            if (itemTreeDump())
                item->dump();

            QPerformanceLog::displayData();
            QPerformanceLog::clear();
            d->root = item;
            connect(item, SIGNAL(widthChanged()), this, SLOT(sizeChanged()));
            connect(item, SIGNAL(heightChanged()), this, SLOT(sizeChanged()));
            emit sceneResized(QSize(d->root->width(),d->root->height()));
        } else if (QWidget *wid = qobject_cast<QWidget *>(obj)) {
            window()->setAttribute(Qt::WA_OpaquePaintEvent, false);
            window()->setAttribute(Qt::WA_NoSystemBackground, false);
            if (!layout()) {
                setLayout(new QVBoxLayout);
            } else if (layout()->count()) {
                // Hide the QGraphicsView in GV mode.
                QLayoutItem *item = layout()->itemAt(0);
                if (item->widget())
                    item->widget()->hide();
            }
            layout()->addWidget(wid);
            emit sceneResized(wid->size());
        }
    }
}

void QFxView::sizeChanged()
{
    // delay, so we catch both width and height changing.
    d->resizetimer.start(0,this);
}

void QFxView::timerEvent(QTimerEvent* e)
{
    if (e->timerId() == d->resizetimer.timerId()) {
        if (d->root)
            emit sceneResized(QSize(d->root->width(),d->root->height()));
        d->resizetimer.stop();
    }
}

QFxItem* QFxView::addItem(const QString &xml, QFxItem* parent)
{
    if (!d->root)
        return 0;

    QmlComponent component(&d->engine, xml.toUtf8(), QUrl()); 
    QObject *obj = component.create();
    if (obj){
        QFxItem *item = static_cast<QFxItem *>(obj);
        if (!parent)
            parent = d->root;

        item->setItemParent(parent);
        return item;
    }
    return 0;
}

void QFxView::reset()
{
    clearItems();
    d->engine.clearComponentCache();
}

void QFxView::clearItems()
{
    if (!d->root)
        return;
    delete d->root;
    d->root = 0;
}

QFxItem *QFxView::root() const
{
    return d->root;
}

void QFxView::resizeEvent(QResizeEvent *e)
{
    if (d->root) {
        d->root->setWidth(width());
        d->root->setHeight(height());
    }
    QSimpleCanvas::resizeEvent(e);
}

void QFxView::focusInEvent(QFocusEvent *)
{
    // Do nothing (do not call QWidget::update())
}

void QFxView::focusOutEvent(QFocusEvent *)
{
    // Do nothing (do not call QWidget::update())
}


void QFxView::dumpRoot()
{
    root()->dump();
}
QT_END_NAMESPACE
