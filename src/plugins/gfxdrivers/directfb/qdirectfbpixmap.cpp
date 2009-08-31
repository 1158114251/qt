/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the plugins of the Qt Toolkit.
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
** In addition, as a special exception, Nokia gives you certain
** additional rights.  These rights are described in the Nokia Qt LGPL
** Exception version 1.1, included in the file LGPL_EXCEPTION.txt in this
** package.
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

#include "qdirectfbpixmap.h"

#ifndef QT_NO_QWS_DIRECTFB

#include "qdirectfbscreen.h"
#include "qdirectfbpaintengine.h"

#include <QtGui/qbitmap.h>
#include <QtCore/qfile.h>
#include <directfb.h>


QT_BEGIN_NAMESPACE

static int global_ser_no = 0;

QDirectFBPixmapData::QDirectFBPixmapData(QDirectFBScreen *screen, PixelType pixelType)
    : QPixmapData(pixelType, DirectFBClass), QDirectFBPaintDevice(screen),
      alpha(false)
{
    setSerialNumber(0);
}

QDirectFBPixmapData::~QDirectFBPixmapData()
{
    unlockDirectFB();
    if (dfbSurface && QDirectFBScreen::instance())
        screen->releaseDFBSurface(dfbSurface);
}

void QDirectFBPixmapData::resize(int width, int height)
{
    if (width <= 0 || height <= 0) {
        invalidate();
        return;
    }

    imageFormat = screen->pixelFormat();
    dfbSurface = screen->createDFBSurface(QSize(width, height),
                                          imageFormat,
                                          QDirectFBScreen::TrackSurface);
    d = QDirectFBScreen::depth(imageFormat);
    alpha = false;
    if (!dfbSurface) {
        invalidate();
        qWarning("QDirectFBPixmapData::resize(): Unable to allocate surface");
        return;
    }

    w = width;
    h = height;
    is_null = (w <= 0 || h <= 0);
    setSerialNumber(++global_ser_no);
}

// mostly duplicated from qimage.cpp (QImageData::checkForAlphaPixels)
static bool checkForAlphaPixels(const QImage &img)
{
    const uchar *bits = img.bits();
    const int bytes_per_line = img.bytesPerLine();
    const uchar *end_bits = bits + bytes_per_line;
    const int width = img.width();
    const int height = img.height();
    switch (img.format()) {
    case QImage::Format_Indexed8:
        return img.hasAlphaChannel();
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
        for (int y=0; y<height; ++y) {
            for (int x=0; x<width; ++x) {
                if ((((uint *)bits)[x] & 0xff000000) != 0xff000000) {
                    return true;
                }
            }
            bits += bytes_per_line;
        }
        break;

    case QImage::Format_ARGB8555_Premultiplied:
    case QImage::Format_ARGB8565_Premultiplied:
        for (int y=0; y<height; ++y) {
            while (bits < end_bits) {
                if (bits[0] != 0) {
                    return true;
                }
                bits += 3;
            }
            bits = end_bits;
            end_bits += bytes_per_line;
        }
        break;

    case QImage::Format_ARGB6666_Premultiplied:
        for (int y=0; y<height; ++y) {
            while (bits < end_bits) {
                if ((bits[0] & 0xfc) != 0) {
                    return true;
                }
                bits += 3;
            }
            bits = end_bits;
            end_bits += bytes_per_line;
        }
        break;

    case QImage::Format_ARGB4444_Premultiplied:
        for (int y=0; y<height; ++y) {
            while (bits < end_bits) {
                if ((bits[0] & 0xf0) != 0) {
                    return true;
                }
                bits += 2;
            }
            bits = end_bits;
            end_bits += bytes_per_line;
        }
        break;

    default:
        break;
    }

    return false;
}

bool QDirectFBPixmapData::hasAlphaChannel(const QImage &img)
{
#ifndef QT_NO_DIRECTFB_OPAQUE_DETECTION
    return ::checkForAlphaPixels(img);
#else
    return img.hasAlphaChannel();
#endif
}

#ifdef QT_DIRECTFB_IMAGEPROVIDER
bool QDirectFBPixmapData::fromFile(const QString &filename, const char *format,
                                   Qt::ImageConversionFlags flags)
{
    if (flags == Qt::AutoColor) {
        if (filename.startsWith(QLatin1Char(':'))) { // resource
            QFile file(filename);
            if (!file.open(QIODevice::ReadOnly))
                return false;
            const QByteArray data = file.readAll();
            file.close();
            return fromData(reinterpret_cast<const uchar*>(data.constData()), data.size(), format, flags);
        } else {
            DFBDataBufferDescription description;
            description.flags = DBDESC_FILE;
            const QByteArray fileNameData = filename.toLocal8Bit();
            description.file = fileNameData.constData();
            if (fromDataBufferDescription(description)) {
                return true;
            }
            // fall back to Qt
        }
    }
    return QPixmapData::fromFile(filename, format, flags);
}

bool QDirectFBPixmapData::fromData(const uchar *buffer, uint len, const char *format,
                                   Qt::ImageConversionFlags flags)
{
    if (flags == Qt::AutoColor) {
        DFBDataBufferDescription description;
        description.flags = DBDESC_MEMORY;
        description.memory.data = buffer;
        description.memory.length = len;
        if (fromDataBufferDescription(description))
            return true;
        // fall back to Qt
    }
    return QPixmapData::fromData(buffer, len, format, flags);
}

template <typename T> struct QDirectFBInterfaceCleanupHandler
{
    static void cleanup(T *t) { if (t) t->Release(t); }
};

template <typename T>
class QDirectFBPointer : public QScopedPointer<T, QDirectFBInterfaceCleanupHandler<T> >
{
public:
    QDirectFBPointer(T *t = 0)
        : QScopedPointer<T, QDirectFBInterfaceCleanupHandler<T> >(t)
    {}
};

bool QDirectFBPixmapData::fromDataBufferDescription(const DFBDataBufferDescription &dataBufferDescription)
{
    IDirectFB *dfb = screen->dfb();
    Q_ASSERT(dfb);
    DFBResult result = DFB_OK;
    IDirectFBDataBuffer *dataBufferPtr;
    if ((result = dfb->CreateDataBuffer(dfb, &dataBufferDescription, &dataBufferPtr)) != DFB_OK) {
        DirectFBError("QDirectFBPixmapData::fromDataBufferDescription()", result);
        return false;
    }
    QDirectFBPointer<IDirectFBDataBuffer> dataBuffer(dataBufferPtr);

    IDirectFBImageProvider *providerPtr;
    if ((result = dataBuffer->CreateImageProvider(dataBuffer.data(), &providerPtr)) != DFB_OK) {
        DirectFBError("QDirectFBPixmapData::fromDataBufferDescription(): Can't create image provider", result);
        return false;
    }
    QDirectFBPointer<IDirectFBImageProvider> provider(providerPtr);

    DFBSurfaceDescription surfaceDescription;
    if ((result = provider->GetSurfaceDescription(provider.data(), &surfaceDescription)) != DFB_OK) {
        DirectFBError("QDirectFBPixmapData::fromDataBufferDescription(): Can't get surface description", result);
        return false;
    }

    QDirectFBPointer<IDirectFBSurface> surfaceFromDescription;
    surfaceFromDescription.reset(screen->createDFBSurface(surfaceDescription, QDirectFBScreen::DontTrackSurface, &result));
    if (!surfaceFromDescription) {
        DirectFBError("QDirectFBPixmapData::fromSurfaceDescription(): Can't create surface", result);
        return false;
    }

    result = provider->RenderTo(provider.data(), surfaceFromDescription.data(), 0);
    if (result != DFB_OK) {
        DirectFBError("QDirectFBPixmapData::fromSurfaceDescription(): Can't render to surface", result);
        return false;
    }

    DFBImageDescription imageDescription;
    result = provider->GetImageDescription(provider.data(), &imageDescription);
    if (result != DFB_OK) {
        DirectFBError("QDirectFBPixmapData::fromSurfaceDescription(): Can't get image description", result);
        return false;
    }

    alpha = imageDescription.caps & (DICAPS_ALPHACHANNEL|DICAPS_COLORKEY);
    imageFormat = alpha ? screen->alphaPixmapFormat() : screen->pixelFormat();

    dfbSurface = screen->createDFBSurface(QSize(surfaceDescription.width, surfaceDescription.height),
                                          imageFormat, QDirectFBScreen::TrackSurface);
    if (alpha)
        dfbSurface->Clear(dfbSurface, 0, 0, 0, 0);

    DFBSurfaceBlittingFlags blittingFlags = DSBLIT_NOFX;
    if (imageDescription.caps & DICAPS_COLORKEY) {
        blittingFlags |= DSBLIT_SRC_COLORKEY;
        result = surfaceFromDescription->SetSrcColorKey(surfaceFromDescription.data(),
                                                        imageDescription.colorkey_r,
                                                        imageDescription.colorkey_g,
                                                        imageDescription.colorkey_b);
        if (result != DFB_OK) {
            DirectFBError("QDirectFBPixmapData::fromSurfaceDescription: Can't set src color key", result);
            invalidate(); // release dfbSurface
            return false;
        }
    }
    if (imageDescription.caps & DICAPS_ALPHACHANNEL) {
        blittingFlags |= DSBLIT_BLEND_ALPHACHANNEL;
    }
    result = dfbSurface->SetBlittingFlags(dfbSurface, blittingFlags);
    if (result != DFB_OK) {
        DirectFBError("QDirectFBPixmapData::fromSurfaceDescription: Can't set blitting flags", result);
        invalidate(); // release dfbSurface
        return false;
    }

    result = dfbSurface->Blit(dfbSurface, surfaceFromDescription.data(), 0, 0, 0);
    if (result != DFB_OK) {
        DirectFBError("QDirectFBPixmapData::fromSurfaceDescription: Can't blit to surface", result);
        invalidate(); // release dfbSurface
        return false;
    }
    if (blittingFlags != DSBLIT_NOFX) {
        dfbSurface->SetBlittingFlags(dfbSurface, DSBLIT_NOFX);
    }


    w = surfaceDescription.width;
    h = surfaceDescription.height;
    is_null = (w <= 0 || h <= 0);
    d = QDirectFBScreen::depth(imageFormat);
    setSerialNumber(++global_ser_no);
#if (Q_DIRECTFB_VERSION >= 0x010000)
    dfbSurface->ReleaseSource(dfbSurface);
#endif
#if defined QT_DIRECTFB_IMAGEPROVIDER_KEEPALIVE
    screen->setDirectFBImageProvider(providerPtr);
    provider.take();
#endif

    return true;
}

#endif

void QDirectFBPixmapData::fromImage(const QImage &image,
                                    Qt::ImageConversionFlags flags)
{
    if (image.depth() == 1) {
        fromImage(image.convertToFormat(screen->alphaPixmapFormat()), flags);
        return;
    }

    if (image.hasAlphaChannel()
#ifndef QT_NO_DIRECTFB_OPAQUE_DETECTION
        && (flags & Qt::NoOpaqueDetection || QDirectFBPixmapData::hasAlphaChannel(image))
#endif
        ) {
        alpha = true;
        imageFormat = screen->alphaPixmapFormat();
    } else {
        alpha = false;
        imageFormat = screen->pixelFormat();
    }

    dfbSurface = screen->createDFBSurface(image, imageFormat, QDirectFBScreen::TrackSurface|QDirectFBScreen::NoPreallocated);
    if (!dfbSurface) {
        qWarning("QDirectFBPixmapData::fromImage()");
        invalidate();
        return;
    }
    w = image.width();
    h = image.height();
    is_null = (w <= 0 || h <= 0);
    d = QDirectFBScreen::depth(imageFormat);
    setSerialNumber(++global_ser_no);
#ifdef QT_NO_DIRECTFB_OPAQUE_DETECTION
    Q_UNUSED(flags);
#endif
}

void QDirectFBPixmapData::copy(const QPixmapData *data, const QRect &rect)
{
    if (data->classId() != DirectFBClass) {
        QPixmapData::copy(data, rect);
        return;
    }
    unlockDirectFB();

    const QDirectFBPixmapData *otherData = static_cast<const QDirectFBPixmapData*>(data);
    IDirectFBSurface *src = otherData->directFBSurface();
    alpha = data->hasAlphaChannel();
    imageFormat = (alpha
                   ? QDirectFBScreen::instance()->alphaPixmapFormat()
                   : QDirectFBScreen::instance()->pixelFormat());


    dfbSurface = screen->createDFBSurface(rect.size(), imageFormat,
                                          QDirectFBScreen::TrackSurface);
    if (!dfbSurface) {
        qWarning("QDirectFBPixmapData::copy()");
        invalidate();
        return;
    }

    if (alpha) {
        dfbSurface->Clear(dfbSurface, 0, 0, 0, 0);
        dfbSurface->SetBlittingFlags(dfbSurface, DSBLIT_BLEND_ALPHACHANNEL);
    } else {
        dfbSurface->SetBlittingFlags(dfbSurface, DSBLIT_NOFX);
    }
    const DFBRectangle blitRect = { rect.x(), rect.y(),
                                    rect.width(), rect.height() };
    w = rect.width();
    h = rect.height();
    d = otherData->d;
    is_null = (w <= 0 || h <= 0);
    DFBResult result = dfbSurface->Blit(dfbSurface, src, &blitRect, 0, 0);
#if (Q_DIRECTFB_VERSION >= 0x010000)
    dfbSurface->ReleaseSource(dfbSurface);
#endif
    if (result != DFB_OK) {
        DirectFBError("QDirectFBPixmapData::copy()", result);
        invalidate();
        return;
    }

    setSerialNumber(++global_ser_no);
}

static inline bool isOpaqueFormat(QImage::Format format)
{
    switch (format) {
    case QImage::Format_RGB32:
    case QImage::Format_RGB16:
    case QImage::Format_RGB666:
    case QImage::Format_RGB555:
    case QImage::Format_RGB888:
    case QImage::Format_RGB444:
        return true;
    default:
        break;
    }
    return false;
}

void QDirectFBPixmapData::fill(const QColor &color)
{
    if (!serialNumber())
        return;

    Q_ASSERT(dfbSurface);

    alpha = (color.alpha() < 255);

    if (alpha && ::isOpaqueFormat(imageFormat)) {
        QSize size;
        dfbSurface->GetSize(dfbSurface, &size.rwidth(), &size.rheight());
        screen->releaseDFBSurface(dfbSurface);
        imageFormat = screen->alphaPixmapFormat();
        d = QDirectFBScreen::depth(imageFormat);
        dfbSurface = screen->createDFBSurface(size, screen->alphaPixmapFormat(), QDirectFBScreen::TrackSurface);
        setSerialNumber(++global_ser_no);
        if (!dfbSurface) {
            qWarning("QDirectFBPixmapData::fill()");
            invalidate();
            return;
        }
    }

    dfbSurface->Clear(dfbSurface, color.red(), color.green(), color.blue(), color.alpha());
}

QPixmap QDirectFBPixmapData::transformed(const QTransform &transform,
                                         Qt::TransformationMode mode) const
{
    QDirectFBPixmapData *that = const_cast<QDirectFBPixmapData*>(this);
    if (!dfbSurface || transform.type() != QTransform::TxScale
        || mode != Qt::FastTransformation)
    {
        const QImage *image = that->buffer();
        Q_ASSERT(image);
        const QImage transformed = image->transformed(transform, mode);
        that->unlockDirectFB();
        QDirectFBPixmapData *data = new QDirectFBPixmapData(screen, QPixmapData::PixmapType);
        data->fromImage(transformed, Qt::AutoColor);
        return QPixmap(data);
    }
    that->unlockDirectFB();

    const QSize size = transform.mapRect(QRect(0, 0, w, h)).size();
    if (size.isEmpty())
        return QPixmap();

    QDirectFBPixmapData *data = new QDirectFBPixmapData(screen, QPixmapData::PixmapType);
    DFBSurfaceBlittingFlags flags = DSBLIT_NOFX;
    data->alpha = alpha;
    if (alpha) {
        flags = DSBLIT_BLEND_ALPHACHANNEL;
    }
    data->dfbSurface = screen->createDFBSurface(size,
                                                imageFormat,
                                                QDirectFBScreen::TrackSurface);
    if (flags & DSBLIT_BLEND_ALPHACHANNEL) {
        data->dfbSurface->Clear(data->dfbSurface, 0, 0, 0, 0);
    }
    data->dfbSurface->SetBlittingFlags(data->dfbSurface, flags);

    const DFBRectangle destRect = { 0, 0, size.width(), size.height() };
    data->dfbSurface->StretchBlit(data->dfbSurface, dfbSurface, 0, &destRect);
    data->w = size.width();
    data->h = size.height();
    data->is_null = (data->w <= 0 || data->h <= 0);

#if (Q_DIRECTFB_VERSION >= 0x010000)
    data->dfbSurface->ReleaseSource(data->dfbSurface);
#endif
    return QPixmap(data);
}

QImage QDirectFBPixmapData::toImage() const
{
    if (!dfbSurface)
        return QImage();

#if 0
    // In later versions of DirectFB one can set a flag to tell
    // DirectFB not to move the surface to videomemory. When that
    // happens we can use this (hopefully faster) codepath
#ifndef QT_NO_DIRECTFB_PREALLOCATED
    QImage ret(w, h, QDirectFBScreen::getImageFormat(dfbSurface));
    if (IDirectFBSurface *imgSurface = screen->createDFBSurface(ret, QDirectFBScreen::DontTrackSurface)) {
        if (hasAlphaChannel()) {
            imgSurface->SetBlittingFlags(imgSurface, DSBLIT_BLEND_ALPHACHANNEL);
            imgSurface->Clear(imgSurface, 0, 0, 0, 0);
        } else {
            imgSurface->SetBlittingFlags(imgSurface, DSBLIT_NOFX);
        }
        imgSurface->Blit(imgSurface, dfbSurface, 0, 0, 0);
#if (Q_DIRECTFB_VERSION >= 0x010000)
        imgSurface->ReleaseSource(imgSurface);
#endif
        imgSurface->Release(imgSurface);
        return ret;
    }
#endif
#endif

    QDirectFBPixmapData *that = const_cast<QDirectFBPixmapData*>(this);
    const QImage *img = that->buffer();
    return img->copy();
}

/* This is QPixmapData::paintEngine(), not QPaintDevice::paintEngine() */

QPaintEngine *QDirectFBPixmapData::paintEngine() const
{
    if (!engine) {
        // QDirectFBPixmapData is also a QCustomRasterPaintDevice, so pass
        // that to the paint engine:
        QDirectFBPixmapData *that = const_cast<QDirectFBPixmapData*>(this);
        that->engine = new QDirectFBPaintEngine(that);
    }
    return engine;
}

QImage *QDirectFBPixmapData::buffer()
{
    lockDirectFB(DSLF_READ|DSLF_WRITE);
    return lockedImage;
}

QImage * QDirectFBPixmapData::buffer(DFBSurfaceLockFlags lockFlags)
{
    lockDirectFB(lockFlags);
    return lockedImage;
}

void QDirectFBPixmapData::invalidate()
{
    if (dfbSurface) {
        screen->releaseDFBSurface(dfbSurface);
        dfbSurface = 0;
    }
    setSerialNumber(0);
    alpha = false;
    d = w = h = 0;
    is_null = true;
    imageFormat = QImage::Format_Invalid;
}

QT_END_NAMESPACE

#endif // QT_NO_QWS_DIRECTFB



