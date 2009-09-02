/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtOpenGL module of the Qt Toolkit.
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

#include "qpixmap.h"
#include "qglframebufferobject.h"

#include <private/qpaintengine_raster_p.h>

#include "qpixmapdata_gl_p.h"

#include <private/qgl_p.h>
#include <private/qdrawhelper_p.h>

#include <private/qpaintengineex_opengl2_p.h>

QT_BEGIN_NAMESPACE

extern QGLWidget* qt_gl_share_widget();

/*!
    \class QGLFramebufferObjectPool
    \since 4.6

    \brief The QGLFramebufferObject class provides a pool of framebuffer
    objects for offscreen rendering purposes.

    When requesting an FBO of a given size and format, an FBO of the same
    format and a size at least as big as the requested size will be returned.

    \internal
*/

static inline int areaDiff(const QSize &size, const QGLFramebufferObject *fbo)
{
    return qAbs(size.width() * size.height() - fbo->width() * fbo->height());
}

QGLFramebufferObject *QGLFramebufferObjectPool::acquire(const QSize &requestSize, const QGLFramebufferObjectFormat &requestFormat)
{
    QGLFramebufferObject *chosen = 0;
    QGLFramebufferObject *candidate = 0;
    for (int i = 0; !chosen && i < m_fbos.size(); ++i) {
        QGLFramebufferObject *fbo = m_fbos.at(i);

        QGLFramebufferObjectFormat format = fbo->format();
        if (format.samples() == requestFormat.samples()
            && format.attachment() == requestFormat.attachment()
            && format.textureTarget() == requestFormat.textureTarget()
            && format.internalFormat() == requestFormat.internalFormat())
        {
            // choose the fbo with a matching format and the closest size
            if (!candidate || areaDiff(requestSize, candidate) > areaDiff(requestSize, fbo))
                candidate = fbo;
        }

        if (candidate) {
            m_fbos.removeOne(candidate);

            const QSize fboSize = candidate->size();
            QSize sz = fboSize;

            if (sz.width() < requestSize.width())
                sz.setWidth(qMax(requestSize.width(), qRound(sz.width() * 1.5)));
            if (sz.height() < requestSize.height())
                sz.setHeight(qMax(requestSize.height(), qRound(sz.height() * 1.5)));

            // wasting too much space?
            if (sz.width() * sz.height() > requestSize.width() * requestSize.height() * 2.5)
                sz = requestSize;

            if (sz != fboSize) {
                delete candidate;
                candidate = new QGLFramebufferObject(sz, requestFormat);
            }

            chosen = candidate;
        }
    }

    if (!chosen) {
        chosen = new QGLFramebufferObject(requestSize, requestFormat);
    }

    if (!chosen->isValid()) {
        delete chosen;
        chosen = 0;
    }

    return chosen;
}

void QGLFramebufferObjectPool::release(QGLFramebufferObject *fbo)
{
    m_fbos << fbo;
}

static int qt_gl_pixmap_serial = 0;

QGLPixmapData::QGLPixmapData(PixelType type)
    : QPixmapData(type, OpenGLClass)
    , m_renderFbo(0)
    , m_engine(0)
    , m_ctx(0)
    , m_dirty(false)
    , m_hasFillColor(false)
    , m_hasAlpha(false)
{
    setSerialNumber(++qt_gl_pixmap_serial);
}

QGLPixmapData::~QGLPixmapData()
{
    QGLWidget *shareWidget = qt_gl_share_widget();
    if (!shareWidget)
        return;

    delete m_engine;

    if (m_texture.id) {
        QGLShareContextScope ctx(shareWidget->context());
        glDeleteTextures(1, &m_texture.id);
    }
}

bool QGLPixmapData::isValid() const
{
    return w > 0 && h > 0;
}

bool QGLPixmapData::isValidContext(const QGLContext *ctx) const
{
    if (ctx == m_ctx)
        return true;

    const QGLContext *share_ctx = qt_gl_share_widget()->context();
    return ctx == share_ctx || qgl_share_reg()->checkSharing(ctx, share_ctx);
}

void QGLPixmapData::resize(int width, int height)
{
    if (width == w && height == h)
        return;

    if (width <= 0 || height <= 0) {
        width = 0;
        height = 0;
    }

    w = width;
    h = height;
    is_null = (w <= 0 || h <= 0);
    d = pixelType() == QPixmapData::PixmapType ? 32 : 1;

    if (m_texture.id) {
        QGLShareContextScope ctx(qt_gl_share_widget()->context());
        glDeleteTextures(1, &m_texture.id);
        m_texture.id = 0;
    }

    m_source = QImage();
    m_dirty = isValid();
    setSerialNumber(++qt_gl_pixmap_serial);
}

void QGLPixmapData::ensureCreated() const
{
    if (!m_dirty)
        return;

    m_dirty = false;

    QGLShareContextScope ctx(qt_gl_share_widget()->context());
    m_ctx = ctx;

    const GLenum format = qt_gl_preferredTextureFormat();
    const GLenum target = GL_TEXTURE_2D;

    if (!m_texture.id) {
        glGenTextures(1, &m_texture.id);
        glBindTexture(target, m_texture.id);
        GLenum format = m_hasAlpha ? GL_RGBA : GL_RGB;
        glTexImage2D(target, 0, format, w, h, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    if (!m_source.isNull()) {
        const QImage tx = ctx->d_func()->convertToGLFormat(m_source, true, format);

        glBindTexture(target, m_texture.id);
        glTexSubImage2D(target, 0, 0, 0, w, h, format,
                        GL_UNSIGNED_BYTE, tx.bits());

        if (useFramebufferObjects())
            m_source = QImage();
    }

    m_texture.options &= ~QGLContext::MemoryManagedBindOption;
}

QGLFramebufferObject *QGLPixmapData::fbo() const
{
    return m_renderFbo;
}

void QGLPixmapData::fromImage(const QImage &image,
                              Qt::ImageConversionFlags)
{
    if (image.size() == QSize(w, h))
        setSerialNumber(++qt_gl_pixmap_serial);
    resize(image.width(), image.height());

    if (pixelType() == BitmapType) {
        m_source = image.convertToFormat(QImage::Format_MonoLSB);
    } else {
        m_source = image.hasAlphaChannel()
            ? image.convertToFormat(QImage::Format_ARGB32_Premultiplied)
            : image.convertToFormat(QImage::Format_RGB32);
    }

    m_dirty = true;
    m_hasFillColor = false;

    m_hasAlpha = image.hasAlphaChannel();
    w = image.width();
    h = image.height();
    is_null = (w <= 0 || h <= 0);
    d = pixelType() == QPixmapData::PixmapType ? 32 : 1;

    if (m_texture.id) {
        QGLShareContextScope ctx(qt_gl_share_widget()->context());
        glDeleteTextures(1, &m_texture.id);
        m_texture.id = 0;
    }
}

bool QGLPixmapData::scroll(int dx, int dy, const QRect &rect)
{
    Q_UNUSED(dx);
    Q_UNUSED(dy);
    Q_UNUSED(rect);
    return false;
}

void QGLPixmapData::copy(const QPixmapData *data, const QRect &rect)
{
    if (data->classId() != QPixmapData::OpenGLClass) {
        QPixmapData::copy(data, rect);
        return;
    }

    // can be optimized to do a framebuffer blit or similar ...
    QPixmapData::copy(data, rect);
}

void QGLPixmapData::fill(const QColor &color)
{
    if (!isValid())
        return;

    bool hasAlpha = color.alpha() != 255;
    if (hasAlpha && !m_hasAlpha) {
        if (m_texture.id) {
            glDeleteTextures(1, &m_texture.id);
            m_texture.id = 0;
            m_dirty = true;
        }
        m_hasAlpha = color.alpha() != 255;
    }

    if (useFramebufferObjects()) {
        m_source = QImage();
        m_hasFillColor = true;
        m_fillColor = color;
    } else {
        QImage image = fillImage(color);
        fromImage(image, 0);
    }
}

bool QGLPixmapData::hasAlphaChannel() const
{
    return m_hasAlpha;
}

QImage QGLPixmapData::fillImage(const QColor &color) const
{
    QImage img;
    if (pixelType() == BitmapType) {
        img = QImage(w, h, QImage::Format_MonoLSB);
        img.setNumColors(2);
        img.setColor(0, QColor(Qt::color0).rgba());
        img.setColor(1, QColor(Qt::color1).rgba());

        int gray = qGray(color.rgba());
        if (qAbs(255 - gray) < gray)
            img.fill(0);
        else
            img.fill(1);
    } else {
        img = QImage(w, h,
                m_hasAlpha
                ? QImage::Format_ARGB32_Premultiplied
                : QImage::Format_RGB32);
        img.fill(PREMUL(color.rgba()));
    }
    return img;
}

extern QImage qt_gl_read_texture(const QSize &size, bool alpha_format, bool include_alpha);

QImage QGLPixmapData::toImage() const
{
    if (!isValid())
        return QImage();

    if (m_renderFbo) {
        copyBackFromRenderFbo(true);
    } else if (!m_source.isNull()) {
        return m_source;
    } else if (m_dirty || m_hasFillColor) {
        return fillImage(m_fillColor);
    } else {
        ensureCreated();
    }

    QGLShareContextScope ctx(qt_gl_share_widget()->context());
    glBindTexture(GL_TEXTURE_2D, m_texture.id);
    return qt_gl_read_texture(QSize(w, h), true, true);
}

struct TextureBuffer
{
    QGLFramebufferObject *fbo;
    QGL2PaintEngineEx *engine;
};

Q_GLOBAL_STATIC(QGLFramebufferObjectPool, _qgl_fbo_pool)
QGLFramebufferObjectPool* qgl_fbo_pool()
{
    return _qgl_fbo_pool();
}

void QGLPixmapData::copyBackFromRenderFbo(bool keepCurrentFboBound) const
{
    if (!isValid())
        return;

    m_hasFillColor = false;

    const QGLContext *share_ctx = qt_gl_share_widget()->context();
    QGLShareContextScope ctx(share_ctx);

    ensureCreated();

    if (!ctx->d_ptr->fbo)
        glGenFramebuffers(1, &ctx->d_ptr->fbo);

    glBindFramebuffer(GL_FRAMEBUFFER_EXT, ctx->d_ptr->fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
        GL_TEXTURE_2D, m_texture.id, 0);

    const int x0 = 0;
    const int x1 = w;
    const int y0 = 0;
    const int y1 = h;

    glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, m_renderFbo->handle());

    glDisable(GL_SCISSOR_TEST);

    glBlitFramebufferEXT(x0, y0, x1, y1,
            x0, y0, x1, y1,
            GL_COLOR_BUFFER_BIT,
            GL_NEAREST);

    if (keepCurrentFboBound)
        glBindFramebuffer(GL_FRAMEBUFFER_EXT, ctx->d_ptr->current_fbo);
}

void QGLPixmapData::swapBuffers()
{
    if (!isValid())
        return;

    copyBackFromRenderFbo(false);
    m_renderFbo->release();

    qgl_fbo_pool()->release(m_renderFbo);

    m_renderFbo = 0;
}

void QGLPixmapData::makeCurrent()
{
    if (isValid() && m_renderFbo)
        m_renderFbo->bind();
}

void QGLPixmapData::doneCurrent()
{
    if (isValid() && m_renderFbo)
        m_renderFbo->release();
}

bool QGLPixmapData::useFramebufferObjects()
{
    return QGLFramebufferObject::hasOpenGLFramebufferObjects()
           && QGLFramebufferObject::hasOpenGLFramebufferBlit()
           && qt_gl_preferGL2Engine();
}

QPaintEngine* QGLPixmapData::paintEngine() const
{
    if (!isValid())
        return 0;

    if (m_renderFbo)
        return m_engine;

    if (useFramebufferObjects()) {
        extern QGLWidget* qt_gl_share_widget();

        if (!QGLContext::currentContext())
            qt_gl_share_widget()->makeCurrent();
        QGLShareContextScope ctx(qt_gl_share_widget()->context());

        QGLFramebufferObjectFormat format;
        format.setAttachment(QGLFramebufferObject::CombinedDepthStencil);
        format.setSamples(4);
        format.setInternalFormat(m_hasAlpha ? GL_RGBA : GL_RGB);

        m_renderFbo = qgl_fbo_pool()->acquire(size(), format);

        if (m_renderFbo) {
            if (!m_engine)
                m_engine = new QGL2PaintEngineEx;
            return m_engine;
        }

        qWarning() << "Failed to create pixmap texture buffer of size " << size() << ", falling back to raster paint engine";
    }

    m_dirty = true;
    if (m_source.size() != size())
        m_source = QImage(size(), QImage::Format_ARGB32_Premultiplied);
    if (m_hasFillColor) {
        m_source.fill(PREMUL(m_fillColor.rgba()));
        m_hasFillColor = false;
    }
    return m_source.paintEngine();
}

GLuint QGLPixmapData::bind(bool copyBack) const
{
    if (m_renderFbo && copyBack) {
        copyBackFromRenderFbo(true);
    } else {
        if (m_hasFillColor) {
            m_dirty = true;
            m_source = QImage(w, h, QImage::Format_ARGB32_Premultiplied);
            m_source.fill(PREMUL(m_fillColor.rgba()));
            m_hasFillColor = false;
        }
        ensureCreated();
    }

    GLuint id = m_texture.id;
    glBindTexture(GL_TEXTURE_2D, id);
    return id;
}

GLuint QGLPixmapData::textureId() const
{
    ensureCreated();
    return m_texture.id;
}

QGLTexture* QGLPixmapData::texture() const
{
    return &m_texture;
}

extern int qt_defaultDpiX();
extern int qt_defaultDpiY();

int QGLPixmapData::metric(QPaintDevice::PaintDeviceMetric metric) const
{
    if (w == 0)
        return 0;

    switch (metric) {
    case QPaintDevice::PdmWidth:
        return w;
    case QPaintDevice::PdmHeight:
        return h;
    case QPaintDevice::PdmNumColors:
        return 0;
    case QPaintDevice::PdmDepth:
        return d;
    case QPaintDevice::PdmWidthMM:
        return qRound(w * 25.4 / qt_defaultDpiX());
    case QPaintDevice::PdmHeightMM:
        return qRound(h * 25.4 / qt_defaultDpiY());
    case QPaintDevice::PdmDpiX:
    case QPaintDevice::PdmPhysicalDpiX:
        return qt_defaultDpiX();
    case QPaintDevice::PdmDpiY:
    case QPaintDevice::PdmPhysicalDpiY:
        return qt_defaultDpiY();
    default:
        qWarning("QGLPixmapData::metric(): Invalid metric");
        return 0;
    }
}

QT_END_NAMESPACE
