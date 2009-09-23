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

#include "qfxpainteditem.h"
#include "qfxpainteditem_p.h"

#include <QDebug>
#include <QPen>
#include <QFile>
#include <QEvent>
#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

QT_BEGIN_NAMESPACE

/*!
    \class QFxPaintedItem
    \brief The QFxPaintedItem class is an abstract base class for QFxView items that want cached painting.
    \ingroup group_coreitems

    This is a convenience class for implementing items that paint their contents
    using a QPainter.  The contents of the item are are cached behind the scenes.
    The dirtyCache() function should be called if the contents change to
    ensure the cache is refreshed the next time painting occurs.

    To subclass QFxPaintedItem, you must reimplement drawContents() to draw
    the contents of the item.
*/

/*!
    \fn void QFxPaintedItem::drawContents(QPainter *painter, const QRect &rect)

    This function is called when the cache needs to be refreshed. When
    sub-classing QFxPaintedItem this function should be implemented so as to
    paint the contents of the item using the given \a painter for the
    area of the contents specified by \a rect.
*/

/*!
    \property QFxPaintedItem::contentsSize
    \brief The size of the contents

    The contents size is the size of the item in regards to how it is painted
    using the drawContents() function.  This is distinct from the size of the
    item in regards to height() and width().
*/

// XXX bug in WebKit - can call repaintRequested and other cache-changing functions from within render!
static int inpaint=0;
static int inpaint_clearcache=0;

/*!
    Marks areas of the cache that intersect with the given \a rect as dirty and
    in need of being refreshed.

    \sa clearCache()
*/
void QFxPaintedItem::dirtyCache(const QRect& rect)
{
    Q_D(QFxPaintedItem);
    for (int i=0; i < d->imagecache.count(); ) {
        QFxPaintedItemPrivate::ImageCacheItem *c = d->imagecache[i];
        QRect isect = (c->area & rect) | c->dirty;
        if (isect == c->area && !inpaint) {
            delete d->imagecache.takeAt(i);
        } else {
            c->dirty = isect;
            ++i;
        }
    }
}

/*!
    Marks the entirety of the contents cache as dirty.

    \sa dirtyCache()
*/
void QFxPaintedItem::clearCache()
{
    if (inpaint) {
        inpaint_clearcache=1;
        return;
    }
    Q_D(QFxPaintedItem);
    qDeleteAll(d->imagecache);
    d->imagecache.clear();
}

/*!
    Returns the size of the contents.

    \sa setContentsSize()
*/
QSize QFxPaintedItem::contentsSize() const
{
    Q_D(const QFxPaintedItem);
    return d->contentsSize;
}

/*!
    Sets the size of the contents to the given \a size.

    \sa contentsSize()
*/
void QFxPaintedItem::setContentsSize(const QSize &size)
{
    Q_D(QFxPaintedItem);
    if (d->contentsSize == size) return;
    d->contentsSize = size;
    clearCache();
    update();
}

/*!
    Constructs a new QFxPaintedItem with the given \a parent.
*/
QFxPaintedItem::QFxPaintedItem(QFxItem *parent)
  : QFxItem(*(new QFxPaintedItemPrivate), parent)
{
    init();
}

/*!
    \internal
    Constructs a new QFxPaintedItem with the given \a parent and
    initialized private data member \a dd.
*/
QFxPaintedItem::QFxPaintedItem(QFxPaintedItemPrivate &dd, QFxItem *parent)
  : QFxItem(dd, parent)
{
    init();
}

/*!
    Destroys the image item.
*/
QFxPaintedItem::~QFxPaintedItem()
{
    clearCache();
}

/*!
    \internal
*/
void QFxPaintedItem::init()
{
    connect(this,SIGNAL(widthChanged()),this,SLOT(clearCache()));
    connect(this,SIGNAL(heightChanged()),this,SLOT(clearCache()));
    connect(this,SIGNAL(visibleChanged()),this,SLOT(clearCache()));
}

void QFxPaintedItem::setCacheFrozen(bool frozen)
{
    Q_D(QFxPaintedItem);
    if (d->cachefrozen == frozen)
        return;
    d->cachefrozen = frozen;
    // XXX clear cache?
}

/*!
    \reimp
*/
void QFxPaintedItem::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *)
{
    Q_D(QFxPaintedItem);
    const QRect content(QPoint(0,0),d->contentsSize);
    if (content.width() <= 0 || content.height() <= 0)
        return;

    ++inpaint;

    QRectF clipf = p->clipRegion().boundingRect();
    if (clipf.isEmpty())
        clipf = mapToScene(content).boundingRect(); // ### Inefficient: Maps toScene and then fromScene
    else
        clipf = mapToScene(clipf).boundingRect();

    const QRect clip = mapFromScene(clipf).boundingRect().toRect();

    QRegion topaint(clip);
    topaint &= content;
    QRegion uncached(content);

    int cachesize=0;
    for (int i=0; i<d->imagecache.count(); ++i) {
        QRect area = d->imagecache[i]->area;
        if (topaint.contains(area)) {
            QRectF target(area.x(), area.y(), area.width(), area.height());
            if (!d->cachefrozen) {
                if (!d->imagecache[i]->dirty.isNull() && topaint.contains(d->imagecache[i]->dirty)) {
                    QPainter qp(&d->imagecache[i]->image);
                    qp.setRenderHints(QPainter::HighQualityAntialiasing | QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform, d->smooth);
                    qp.translate(-area.x(), -area.y());
                    qp.eraseRect(d->imagecache[i]->dirty);
                    if (d->fillColor.isValid())
                        qp.fillRect(d->imagecache[i]->dirty,d->fillColor);
                    qp.setClipRect(d->imagecache[i]->dirty);
                    drawContents(&qp, d->imagecache[i]->dirty);
                    d->imagecache[i]->dirty = QRect();
                }
            }
            p->drawPixmap(target.toRect(), d->imagecache[i]->image);
            topaint -= area;
            d->imagecache[i]->age=0;
        } else {
            d->imagecache[i]->age++;
        }
        cachesize += area.width()*area.height();
        uncached -= area;
    }

    if (!topaint.isEmpty()) {
        if (!d->cachefrozen) {
            // Find a sensible larger area, otherwise will paint lots of tiny images.
            QRect biggerrect = topaint.boundingRect().adjusted(-64,-64,128,128);
            cachesize += biggerrect.width() * biggerrect.height();
            while (d->imagecache.count() && cachesize > d->max_imagecache_size) {
                int oldest=-1;
                int age=-1;
                for (int i=0; i<d->imagecache.count(); ++i) {
                    int a = d->imagecache[i]->age;
                    if (a > age) {
                        oldest = i;
                        age = a;
                    }
                }
                cachesize -= d->imagecache[oldest]->area.width()*d->imagecache[oldest]->area.height();
                uncached += d->imagecache[oldest]->area;
                d->imagecache.removeAt(oldest);
            }
            const QRegion bigger = QRegion(biggerrect) & uncached;
            const QVector<QRect> rects = bigger.rects();
            for (int i = 0; i < rects.count(); ++i) {
                const QRect &r = rects.at(i);
                QPixmap img(r.size());
                if (d->fillColor.isValid())
                    img.fill(d->fillColor);
                {
                    QPainter qp(&img);
                    qp.setRenderHints(QPainter::HighQualityAntialiasing | QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform, d->smooth);

                    qp.translate(-r.x(),-r.y());
                    drawContents(&qp, r);
                }
                QFxPaintedItemPrivate::ImageCacheItem *newitem = new QFxPaintedItemPrivate::ImageCacheItem;
                newitem->area = r;
                newitem->image = img;
                d->imagecache.append(newitem);
                p->drawPixmap(r, newitem->image);
            }
        } else {
            const QVector<QRect> rects = uncached.rects();
            for (int i = 0; i < rects.count(); ++i)
                p->fillRect(rects.at(i), Qt::lightGray);
        }
    }

    if (inpaint_clearcache) {
        clearCache();
        inpaint_clearcache = 0;
    }

    --inpaint;
}

/*!
  \qmlproperty int PaintedItem::cacheSize

  This property holds the maximum number of pixels of image cache to
  allow. The default is 0.1 megapixels. The cache will not be larger
  than the (unscaled) size of the item.
*/

/*!
  \property QFxPaintedItem::cacheSize

  The maximum number of pixels of image cache to allow. The default
  is 0.1 megapixels. The cache will not be larger than the (unscaled)
  size of the QFxPaintedItem.
*/
int QFxPaintedItem::cacheSize() const
{
    Q_D(const QFxPaintedItem);
    return d->max_imagecache_size;
}

void QFxPaintedItem::setCacheSize(int pixels)
{
    Q_D(QFxPaintedItem);
    if (pixels < d->max_imagecache_size) {
        int cachesize=0;
        for (int i=0; i<d->imagecache.count(); ++i) {
            QRect area = d->imagecache[i]->area;
            cachesize += area.width()*area.height();
        }
        while (d->imagecache.count() && cachesize > pixels) {
            int oldest=-1;
            int age=-1;
            for (int i=0; i<d->imagecache.count(); ++i) {
                int a = d->imagecache[i]->age;
                if (a > age) {
                    oldest = i;
                    age = a;
                }
            }
            cachesize -= d->imagecache[oldest]->area.width()*d->imagecache[oldest]->area.height();
            d->imagecache.removeAt(oldest);
        }
    }
    d->max_imagecache_size = pixels;
}

/*!
    \property QFxPaintedItem::fillColor

    The color to be used to fill the item prior to calling drawContents().
    By default, this is Qt::transparent.

    Performance improvements can be achieved if subclasses call this with either an
    invalid color (QColor()), or an appropriate solid color.
*/
void QFxPaintedItem::setFillColor(const QColor& c)
{
    Q_D(QFxPaintedItem);
    if (d->fillColor == c)
        return;
    d->fillColor = c;
    emit fillColorChanged();
    update();
}

QColor QFxPaintedItem::fillColor() const
{
    Q_D(const QFxPaintedItem);
    return d->fillColor;
}


QT_END_NAMESPACE
