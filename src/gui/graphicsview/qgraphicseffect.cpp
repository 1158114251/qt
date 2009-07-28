/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#include "qgraphicseffect_p.h"

#ifndef QT_NO_GRAPHICSVIEW

#include <QtGui/qimage.h>
#include <QtGui/qgraphicsitem.h>
#include <QtGui/qgraphicsscene.h>
#include <QtGui/qpainter.h>

/*

  List of known drawbacks which are being discussed:

  * No d-pointer yet.

  * No auto test yet.

  * No API documentation yet.

  * The API is far from being finalized.

  * Most of the effect implementation is not efficient,
    as this is still a proof of concept only.

  * Painting artifacts occasionally occur when e.g. moving
    an item over another item that has a large effective
    bounding rect.

  * Item transformation is not taken into account.
    For example, the effective bounding rect is calculated at
    item coordinate (fast), but the painting is mostly
    done at device coordinate.

  * Coordinate mode: item vs device. Most effects make sense only
    in device coordinate. Should we keep both options open?
    See also above transformation issue.

  * Right now the pixmap for effect drawing is stored in each item.
    There can be problems with coordinates, see above.

  * There is a lot of duplication in drawItems() for each effect.

  * Port to use the new layer feature in QGraphicsView.
    This should solve the above pixmap problem.

  * Frame effect is likely useless. However it is very useful
    to check than the effective bounding rect is handled properly.

  * Proper exposed region and rect for style option are missing.

  * Pixelize effect is using raster only, because there is no
    pixmap filter for it. We need to implement the missing pixmap filter.

  * Blur effect is using raster only, with exponential blur algorithm.
    Perhaps use stack blur (better approximate Gaussian blur) instead?
    QPixmapConvolutionFilter is too slow for this simple blur effect.

  * Bloom and shadow effect are also raster only. Same reason as above.

  * Make it work with widgets (QGraphicsWidget).

*/

QGraphicsEffect::QGraphicsEffect()
    : QObject(*new QGraphicsEffectPrivate, 0)
{}

/*!
    \internal
*/
QGraphicsEffect::QGraphicsEffect(QGraphicsEffectPrivate &dd)
    : QObject(dd, 0)
{}

QGraphicsEffect::~QGraphicsEffect()
{
    Q_D(QGraphicsEffect);
    d->setGraphicsEffectSource(0);
}

QRectF QGraphicsEffect::boundingRect() const
{
    return sourceBoundingRect();
}

QRectF QGraphicsEffect::sourceBoundingRect() const
{
    Q_D(const QGraphicsEffect);
    if (d->source)
        return d->source->boundingRect();
    return QRectF();
}

void QGraphicsEffect::drawSource(QPainter *painter)
{
    Q_D(QGraphicsEffect);
    if (d->source)
        d->source->draw(painter);
}

bool QGraphicsEffect::drawSourceIntoPixmap(QPixmap *pixmap, const QTransform &itemToPixmapTransform)
{
    Q_D(QGraphicsEffect);
    if (d->source)
        return d->source->drawIntoPixmap(pixmap, itemToPixmapTransform);
    return false;
}

QGraphicsGrayscaleEffect::QGraphicsGrayscaleEffect()
    : QGraphicsEffect(*new QGraphicsGrayscaleEffectPrivate)
{}

QGraphicsGrayscaleEffect::~QGraphicsGrayscaleEffect()
{}

void QGraphicsGrayscaleEffect::draw(QPainter *painter)
{
    Q_D(QGraphicsGrayscaleEffect);
    if (!d->source)
        return;

    // Find the item's bounds in device coordinates.
    QTransform itemToPixmapTransform(painter->worldTransform());
    QRectF deviceBounds = itemToPixmapTransform.mapRect(d->source->boundingRect());
    QRect deviceRect = deviceBounds.toRect().adjusted(-1, -1, 1, 1);
    if (deviceRect.isEmpty())
        return;

    if (deviceRect.x() != 0 || deviceRect.y() != 0)
        itemToPixmapTransform *= QTransform::fromTranslate(-deviceRect.x(), -deviceRect.y());

    QPixmap pixmap(deviceRect.size());
    if (!d->source->drawIntoPixmap(&pixmap, itemToPixmapTransform))
        return;

    // Draw the pixmap with the filter using an untransformed painter.
    QTransform restoreTransform = painter->worldTransform();
    painter->setWorldTransform(QTransform());
    d->filter->draw(painter, deviceRect.topLeft(), pixmap, pixmap.rect());
    painter->setWorldTransform(restoreTransform);

}

QGraphicsColorizeEffect::QGraphicsColorizeEffect()
    : QGraphicsEffect(*new QGraphicsColorizeEffectPrivate)
{}

QGraphicsColorizeEffect::~QGraphicsColorizeEffect()
{}

QColor QGraphicsColorizeEffect::color() const
{
    Q_D(const QGraphicsColorizeEffect);
    return d->filter->color();
}

void QGraphicsColorizeEffect::setColor(const QColor &c)
{
    Q_D(QGraphicsColorizeEffect);
    d->filter->setColor(c);
}

void QGraphicsColorizeEffect::draw(QPainter *painter)
{
    Q_D(QGraphicsColorizeEffect);
    if (!d->source)
        return;

    // Find the item's bounds in device coordinates.
    QTransform itemToPixmapTransform(painter->worldTransform());
    QRectF deviceBounds = itemToPixmapTransform.mapRect(d->source->boundingRect());
    QRect deviceRect = deviceBounds.toRect().adjusted(-1, -1, 1, 1);
    if (deviceRect.isEmpty())
        return;

    if (deviceRect.x() != 0 || deviceRect.y() != 0)
        itemToPixmapTransform *= QTransform::fromTranslate(-deviceRect.x(), -deviceRect.y());

    QPixmap pixmap(deviceRect.size());
    if (!d->source->drawIntoPixmap(&pixmap, itemToPixmapTransform))
        return;

    // Draw the pixmap with the filter using an untransformed painter.
    QTransform restoreTransform = painter->worldTransform();
    painter->setWorldTransform(QTransform());
    d->filter->draw(painter, deviceRect.topLeft(), pixmap, pixmap.rect());
    painter->setWorldTransform(restoreTransform);
}

QGraphicsPixelizeEffect::QGraphicsPixelizeEffect()
    : QGraphicsEffect(*new QGraphicsPixelizeEffectPrivate)
{
}

QGraphicsPixelizeEffect::~QGraphicsPixelizeEffect()
{
}

int QGraphicsPixelizeEffect::pixelSize() const
{
    Q_D(const QGraphicsPixelizeEffect);
    return d->pixelSize;
}

void QGraphicsPixelizeEffect::setPixelSize(int size)
{
    Q_D(QGraphicsPixelizeEffect);
    d->pixelSize = size;
}

void QGraphicsPixelizeEffect::draw(QPainter *painter)
{
    Q_D(QGraphicsPixelizeEffect);
    if (!d->source)
        return;

    // Find the item's bounds in device coordinates.
    QTransform itemToPixmapTransform(painter->worldTransform());
    QRectF deviceBounds = itemToPixmapTransform.mapRect(d->source->boundingRect());
    QRect deviceRect = deviceBounds.toRect().adjusted(-1, -1, 1, 1);
    if (deviceRect.isEmpty())
        return;

    if (deviceRect.x() != 0 || deviceRect.y() != 0)
        itemToPixmapTransform *= QTransform::fromTranslate(-deviceRect.x(), -deviceRect.y());

    QPixmap pixmap(deviceRect.size());
    if (!d->source->drawIntoPixmap(&pixmap))
        return;

    // pixelize routine
    QImage img = pixmap.toImage().convertToFormat(QImage::Format_ARGB32);
    if (d->pixelSize > 0) {
        int width = img.width();
        int height = img.height();
        for (int y = 0; y < height; y += d->pixelSize) {
            int ys = qMin(height - 1, y + d->pixelSize / 2);
            QRgb *sbuf = reinterpret_cast<QRgb*>(img.scanLine(ys));
            for (int x = 0; x < width; x += d->pixelSize) {
                int xs = qMin(width - 1, x + d->pixelSize / 2);
                QRgb color = sbuf[xs];
                for (int yi = 0; yi < qMin(d->pixelSize, height - y); ++yi) {
                    QRgb *buf = reinterpret_cast<QRgb*>(img.scanLine(y + yi));
                    for (int xi = 0; xi < qMin(d->pixelSize, width - x); ++xi)
                        buf[x + xi] = color;
                }
            }
        }
    }

    // Draw using an untransformed painter.
    QTransform restoreTransform = painter->worldTransform();
    painter->setWorldTransform(QTransform());
    painter->drawImage(deviceRect.topLeft(), img);
    painter->setWorldTransform(restoreTransform);
}

QGraphicsBlurEffect::QGraphicsBlurEffect()
    : QGraphicsEffect(*new QGraphicsBlurEffectPrivate)
{
}

QGraphicsBlurEffect::~QGraphicsBlurEffect()
{
}

// Blur the image according to the blur radius
// Based on exponential blur algorithm by Jani Huhtanen
// (maximum radius is set to 16)
static QImage blurred(const QImage& image, const QRect& rect, int radius)
{
    int tab[] = { 14, 10, 8, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 };
    int alpha = (radius < 1)  ? 16 : (radius > 17) ? 1 : tab[radius-1];

    QImage result = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    int r1 = rect.top();
    int r2 = rect.bottom();
    int c1 = rect.left();
    int c2 = rect.right();

    int bpl = result.bytesPerLine();
    int rgba[4];
    unsigned char* p;

    for (int col = c1; col <= c2; col++) {
        p = result.scanLine(r1) + col * 4;
        for (int i = 0; i < 4; i++)
            rgba[i] = p[i] << 4;

        p += bpl;
        for (int j = r1; j < r2; j++, p += bpl)
            for (int i = 0; i < 4; i++)
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    for (int row = r1; row <= r2; row++) {
        p = result.scanLine(row) + c1 * 4;
        for (int i = 0; i < 4; i++)
            rgba[i] = p[i] << 4;

        p += 4;
        for (int j = c1; j < c2; j++, p += 4)
            for (int i = 0; i < 4; i++)
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    for (int col = c1; col <= c2; col++) {
        p = result.scanLine(r2) + col * 4;
        for (int i = 0; i < 4; i++)
            rgba[i] = p[i] << 4;

        p -= bpl;
        for (int j = r1; j < r2; j++, p -= bpl)
            for (int i = 0; i < 4; i++)
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    for (int row = r1; row <= r2; row++) {
        p = result.scanLine(row) + c2 * 4;
        for (int i = 0; i < 4; i++)
            rgba[i] = p[i] << 4;

        p -= 4;
        for (int j = c1; j < c2; j++, p -= 4)
            for (int i = 0; i < 4; i++)
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    return result;
}

int QGraphicsBlurEffect::blurRadius() const
{
    Q_D(const QGraphicsBlurEffect);
    return int(d->filter->radius());
}

void QGraphicsBlurEffect::setBlurRadius(int radius)
{
    Q_D(QGraphicsBlurEffect);
    d->filter->setRadius(radius);
}

QRectF QGraphicsBlurEffect::boundingRect() const
{
    Q_D(const QGraphicsBlurEffect);
    if (d->source)
        return d->filter->boundingRectFor(d->source->boundingRect());
    return QRectF();
}

void QGraphicsBlurEffect::draw(QPainter *painter)
{
    Q_D(QGraphicsBlurEffect);
    if (!d->source)
        return;

    // Find the item's bounds in device coordinates.
    QTransform itemToPixmapTransform(painter->worldTransform());
    QRectF deviceBounds = itemToPixmapTransform.mapRect(d->source->boundingRect());
    QRect deviceRect = deviceBounds.toRect().adjusted(-1, -1, 1, 1);
    if (deviceRect.isEmpty())
        return;

    // ### Potentially big rect; must be clipped to the viewport rect.
    const qreal delta = d->blurRadius * 3;
    const QRect effectRect = deviceBounds.adjusted(-delta, -delta, delta, delta)
                                         .toRect().adjusted(-1, -1, 1, 1);
    if (effectRect.x() != 0 || effectRect.y() != 0)
        itemToPixmapTransform *= QTransform::fromTranslate(-effectRect.x(), -effectRect.y());

    QPixmap pixmap(effectRect.size());
    if (!d->source->drawIntoPixmap(&pixmap, itemToPixmapTransform))
        return;

    // Draw the pixmap with the filter using an untransformed painter.
    QTransform restoreTransform = painter->worldTransform();
    painter->setWorldTransform(QTransform());
    d->filter->draw(painter, deviceRect.topLeft(), pixmap, pixmap.rect());
    painter->setWorldTransform(restoreTransform);
}

QGraphicsBloomEffect::QGraphicsBloomEffect()
    : QGraphicsEffect(*new QGraphicsBloomEffectPrivate)
{
}

QGraphicsBloomEffect::~QGraphicsBloomEffect()
{
}

int QGraphicsBloomEffect::blurRadius() const
{
    Q_D(const QGraphicsBloomEffect);
    return d->blurRadius;
}

void QGraphicsBloomEffect::setBlurRadius(int radius)
{
    Q_D(QGraphicsBloomEffect);
    d->blurRadius = radius;
}

qreal QGraphicsBloomEffect::opacity() const
{
    Q_D(const QGraphicsBloomEffect);
    return d->opacity;
}

void QGraphicsBloomEffect::setOpacity(qreal alpha)
{
    Q_D(QGraphicsBloomEffect);
    d->opacity = alpha;
}

QRectF QGraphicsBloomEffect::boundingRect() const
{
    Q_D(const QGraphicsBloomEffect);
    if (!d->source)
        return QRectF();
    qreal delta = d->blurRadius * 3;
    return d->source->boundingRect().adjusted(-delta, -delta, delta, delta);
}

// Change brightness (positive integer) of each pixel
static QImage brightened(const QImage& image, int brightness)
{
    int tab[ 256 ];
    for (int i = 0; i < 256; ++i)
        tab[i] = qMin(i + brightness, 255);

    QImage img = image.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < img.height(); y++) {
        QRgb* line = (QRgb*)(img.scanLine(y));
        for (int x = 0; x < img.width(); x++) {
            QRgb c = line[x];
            line[x] = qRgba(tab[qRed(c)], tab[qGreen(c)], tab[qBlue(c)], qAlpha(c));
        }
    }

    return img;
}

// Composite two QImages using given composition mode and opacity
static QImage composited(const QImage& img1, const QImage& img2, qreal opacity, QPainter::CompositionMode mode)
{
    QImage result = img1.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&result);
    painter.setCompositionMode(mode);
    painter.setOpacity(opacity);
    painter.drawImage(0, 0, img2);
    painter.end();
    return result;
}

void QGraphicsBloomEffect::draw(QPainter *painter)
{
    Q_D(QGraphicsBloomEffect);
    if (!d->source)
        return;

    // Find the item's bounds in device coordinates.
    QTransform itemToPixmapTransform(painter->worldTransform());
    QRectF deviceBounds = itemToPixmapTransform.mapRect(d->source->boundingRect());
    QRect deviceRect = deviceBounds.toRect().adjusted(-1, -1, 1, 1);
    if (deviceRect.isEmpty())
        return;

    if (deviceRect.x() != 0 || deviceRect.y() != 0)
        itemToPixmapTransform *= QTransform::fromTranslate(-deviceRect.x(), -deviceRect.y());

    QPixmap pixmap(deviceRect.size());
    if (!d->source->drawIntoPixmap(&pixmap, itemToPixmapTransform))
        return;

    // bloom routine
    int radius = d->blurRadius;
    QImage img = pixmap.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QImage overlay = blurred(img, img.rect(), radius);
    overlay = brightened(overlay, 70);
    img = composited(img, overlay, d->opacity, QPainter::CompositionMode_Overlay);

    // Draw using an untransformed painter.
    QTransform restoreTransform = painter->worldTransform();
    painter->setWorldTransform(QTransform());
    painter->drawImage(deviceRect.topLeft() - QPointF(radius * 3, radius * 3), img);
    painter->setWorldTransform(restoreTransform);
}

QGraphicsFrameEffect::QGraphicsFrameEffect()
    : QGraphicsEffect(*new QGraphicsFrameEffectPrivate)
{
}

QGraphicsFrameEffect::~QGraphicsFrameEffect()
{
}

QColor QGraphicsFrameEffect::frameColor() const
{
    Q_D(const QGraphicsFrameEffect);
    return d->color;
}

void QGraphicsFrameEffect::setFrameColor(const QColor &c)
{
    Q_D(QGraphicsFrameEffect);
    d->color = c;
}

qreal QGraphicsFrameEffect::frameWidth() const
{
    Q_D(const QGraphicsFrameEffect);
    return d->width;
}

void QGraphicsFrameEffect::setFrameWidth(qreal frameWidth)
{
    Q_D(QGraphicsFrameEffect);
    d->width = frameWidth;
}

qreal QGraphicsFrameEffect::frameOpacity() const
{
    Q_D(const QGraphicsFrameEffect);
    return d->alpha;
}

void QGraphicsFrameEffect::setFrameOpacity(qreal opacity)
{
    Q_D(QGraphicsFrameEffect);
    d->alpha = opacity;
}

QRectF QGraphicsFrameEffect::boundingRect() const
{
    Q_D(const QGraphicsFrameEffect);
    if (!d->source)
        return QRectF();
    return d->source->boundingRect().adjusted(-d->width, -d->width, d->width, d->width);
}

void QGraphicsFrameEffect::draw(QPainter *painter)
{
    Q_D(QGraphicsFrameEffect);
    if (!d->source)
        return;

    // Find the item's bounds in device coordinates.
    QTransform itemToPixmapTransform(painter->worldTransform());
    QRectF deviceBounds = itemToPixmapTransform.mapRect(d->source->boundingRect());
    if (deviceBounds.isEmpty())
        return;

    QRect frameRect = deviceBounds.adjusted(-d->width, -d->width, d->width, d->width)
                      .toRect().adjusted(-1, -1, 1, 1);

    if (frameRect.x() != 0 || frameRect.y() != 0)
        itemToPixmapTransform *= QTransform::fromTranslate(-frameRect.x(), -frameRect.y());

    QPixmap pixmap(frameRect.size());
    if (!d->source->drawIntoPixmap(&pixmap, itemToPixmapTransform))
        return;

    painter->save();
    painter->setWorldTransform(QTransform());

    painter->save();
    painter->setOpacity(painter->opacity() * d->alpha);
    painter->setPen(Qt::NoPen);
    painter->setBrush(d->color);
    painter->drawRoundedRect(frameRect, 20, 20, Qt::RelativeSize);
    painter->restore();

    painter->drawPixmap(frameRect.topLeft(), pixmap);

    painter->restore();
}

QGraphicsShadowEffect::QGraphicsShadowEffect()
    : QGraphicsEffect(*new QGraphicsShadowEffectPrivate)
{
}

QGraphicsShadowEffect::~QGraphicsShadowEffect()
{
}

QPointF  QGraphicsShadowEffect::shadowOffset() const
{
    Q_D(const QGraphicsShadowEffect);
    return d->offset;
}

void  QGraphicsShadowEffect::setShadowOffset(const QPointF &ofs)
{
    Q_D(QGraphicsShadowEffect);
    d->offset = ofs;
}

int  QGraphicsShadowEffect::blurRadius() const
{
    Q_D(const QGraphicsShadowEffect);
    return d->radius;
}

void  QGraphicsShadowEffect::setBlurRadius(int blurRadius)
{
    Q_D(QGraphicsShadowEffect);
    d->radius = blurRadius;
}

qreal  QGraphicsShadowEffect::opacity() const
{
    Q_D(const QGraphicsShadowEffect);
    return d->alpha;
}

void  QGraphicsShadowEffect::setOpacity(qreal opacity)
{
    Q_D(QGraphicsShadowEffect);
    d->alpha = opacity;
}

QRectF QGraphicsShadowEffect::boundingRect() const
{
    Q_D(const QGraphicsShadowEffect);
    if (!d->source)
        return QRectF();
    const QRectF srcBrect = d->source->boundingRect();
    QRectF shadowRect = srcBrect;
    shadowRect.adjust(d->offset.x(), d->offset.y(), d->offset.x(), d->offset.y());
    QRectF blurRect = shadowRect;
    qreal delta = d->radius * 3;
    blurRect.adjust(-delta, -delta, delta, delta);
    QRectF totalRect = blurRect.united(srcBrect);
    return totalRect;
}

void QGraphicsShadowEffect::draw(QPainter *painter)
{
    Q_D(QGraphicsShadowEffect);
    if (!d->source)
        return;

    // Find the item's bounds in device coordinates.
    QTransform itemToPixmapTransform(painter->worldTransform());
    QRectF deviceBounds = itemToPixmapTransform.mapRect(d->source->boundingRect());
    QRect deviceRect = deviceBounds.toRect().adjusted(-1, -1, 1, 1);
    if (deviceRect.isEmpty())
        return;

    QRectF shadowRect = deviceBounds;
    shadowRect.adjust(d->offset.x(), d->offset.y(), d->offset.x(), d->offset.y());
    QRectF blurRect = shadowRect;
    qreal delta = d->radius * 3;
    blurRect.adjust(-delta, -delta, delta, delta);
    QRect totalRect = blurRect.united(deviceRect).toRect().adjusted(-1, -1, 1, 1);

    if (totalRect.x() != 0 || totalRect.y() != 0)
        itemToPixmapTransform *= QTransform::fromTranslate(-totalRect.x(), -totalRect.y());

    QPixmap pixmap(totalRect.size());
    if (!d->source->drawIntoPixmap(&pixmap, itemToPixmapTransform))
        return;

    QImage img = pixmap.toImage();
    QImage shadowImage(img.size(), QImage::Format_ARGB32);
    shadowImage.fill(qRgba(0, 0, 0, d->alpha * 255));
    shadowImage.setAlphaChannel(img.alphaChannel());
    shadowImage = blurred(shadowImage, shadowImage.rect(), d->radius);

    // Draw using an untransformed painter.
    QTransform restoreTransform = painter->worldTransform();
    painter->setWorldTransform(QTransform());

    QRect shadowAlignedRect = shadowRect.toAlignedRect();

    qreal shadowx = blurRect.x() + delta;
    qreal shadowy = blurRect.y() + delta;
    if (blurRect.x() < deviceRect.x())
        shadowx = blurRect.x() + d->offset.x();
    if (blurRect.y() < deviceRect.y())
        shadowy = blurRect.y() + d->offset.y();
    painter->drawImage(shadowx, shadowy, shadowImage);

    qreal itemx = qMin(blurRect.x(), deviceBounds.x());
    qreal itemy = qMin(blurRect.y(), deviceBounds.y());
    painter->drawPixmap(itemx, itemy, pixmap);

    painter->setWorldTransform(restoreTransform);
}

#endif
