/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
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
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
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

/*
    When the active program changes, we need to update it's uniforms.
    We could track state for each program and only update stale uniforms
        - Could lead to lots of overhead if there's a lot of programs
    We could update all the uniforms when the program changes
        - Could end up updating lots of uniforms which don't need updating

    Updating uniforms should be cheap, so the overhead of updating up-to-date
    uniforms should be minimal. It's also less complex.

    Things which _may_ cause a different program to be used:
        - Change in brush/pen style
        - Change in painter opacity
        - Change in composition mode

    Whenever we set a mode on the shader manager - it needs to tell us if it had
    to switch to a different program.

    The shader manager should only switch when we tell it to. E.g. if we set a new
    brush style and then switch to transparent painter, we only want it to compile
    and use the correct program when we really need it.
*/

#include "qpaintengineex_opengl2_p.h"

#include <string.h> //for memcpy
#include <qmath.h>

#include <private/qgl_p.h>
#include <private/qmath_p.h>
#include <private/qpaintengineex_p.h>
#include <QPaintEngine>
#include <private/qpainter_p.h>
#include <private/qfontengine_p.h>
#include <private/qtextureglyphcache_p.h>
#include <private/qpixmapdata_gl_p.h>
#include <private/qdatabuffer_p.h>

#include "qglgradientcache_p.h"
#include "qglengineshadermanager_p.h"
#include "qgl2pexvertexarray_p.h"

#include <QDebug>

QT_BEGIN_NAMESPACE

static const GLuint QT_BRUSH_TEXTURE_UNIT       = 0;
static const GLuint QT_IMAGE_TEXTURE_UNIT       = 0; //Can be the same as brush texture unit
static const GLuint QT_MASK_TEXTURE_UNIT        = 1;
static const GLuint QT_BACKGROUND_TEXTURE_UNIT  = 2;

#ifdef Q_WS_WIN
extern Q_GUI_EXPORT bool qt_cleartype_enabled;
#endif

class QGLTextureGlyphCache : public QObject, public QTextureGlyphCache
{
    Q_OBJECT
public:
    QGLTextureGlyphCache(QGLContext *context, QFontEngineGlyphCache::Type type, const QTransform &matrix);
    ~QGLTextureGlyphCache();

    virtual void createTextureData(int width, int height);
    virtual void resizeTextureData(int width, int height);
    virtual void fillTexture(const Coord &c, glyph_t glyph);
    virtual int glyphMargin() const;

    inline GLuint texture() const { return m_texture; }

    inline int width() const { return m_width; }
    inline int height() const { return m_height; }

    inline void setPaintEnginePrivate(QGL2PaintEngineExPrivate *p) { pex = p; }


public Q_SLOTS:
    void contextDestroyed(const QGLContext *context) {
        if (context == ctx) {
            QList<const QGLContext *> shares = qgl_share_reg()->shares(ctx);
            if (shares.isEmpty()) {
                glDeleteFramebuffers(1, &m_fbo);
                if (m_width || m_height)
                    glDeleteTextures(1, &m_texture);
                ctx = 0;
            } else {
                // since the context holding the texture is shared, and
                // about to be destroyed, we have to transfer ownership
                // of the texture to one of the share contexts
                ctx = const_cast<QGLContext *>(shares.at(0));
            }
        }
    }

private:
    QGLContext *ctx;

    QGL2PaintEngineExPrivate *pex;

    GLuint m_texture;
    GLuint m_fbo;

    int m_width;
    int m_height;

    QGLShaderProgram *m_program;
};

QGLTextureGlyphCache::QGLTextureGlyphCache(QGLContext *context, QFontEngineGlyphCache::Type type, const QTransform &matrix)
    : QTextureGlyphCache(type, matrix)
    , ctx(context)
    , m_width(0)
    , m_height(0)
{
    glGenFramebuffers(1, &m_fbo);
    connect(QGLSignalProxy::instance(), SIGNAL(aboutToDestroyContext(const QGLContext *)),
            SLOT(contextDestroyed(const QGLContext *)));
}

QGLTextureGlyphCache::~QGLTextureGlyphCache()
{
    if (ctx) {
        QGLContext *oldContext = const_cast<QGLContext *>(QGLContext::currentContext());
        if (oldContext != ctx)
            ctx->makeCurrent();
        glDeleteFramebuffers(1, &m_fbo);

        if (m_width || m_height)
            glDeleteTextures(1, &m_texture);
        if (oldContext && oldContext != ctx)
            oldContext->makeCurrent();
    }
}

void QGLTextureGlyphCache::createTextureData(int width, int height)
{
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    m_width = width;
    m_height = height;

    QVarLengthArray<uchar> data(width * height);
    for (int i = 0; i < data.size(); ++i)
        data[i] = 0;

    if (m_type == QFontEngineGlyphCache::Raster_RGBMask)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, &data[0]);
    else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, &data[0]);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void QGLTextureGlyphCache::resizeTextureData(int width, int height)
{
    // ### the QTextureGlyphCache API needs to be reworked to allow
    // ### resizeTextureData to fail

    int oldWidth = m_width;
    int oldHeight = m_height;

    GLuint oldTexture = m_texture;
    createTextureData(width, height);

    glBindFramebuffer(GL_FRAMEBUFFER_EXT, m_fbo);

    GLuint tmp_texture;
    glGenTextures(1, &tmp_texture);
    glBindTexture(GL_TEXTURE_2D, tmp_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, oldWidth, oldHeight, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                           GL_TEXTURE_2D, tmp_texture, 0);

    glActiveTexture(GL_TEXTURE0 + QT_IMAGE_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_2D, oldTexture);

    pex->transferMode(BrushDrawingMode);

#ifndef QT_OPENGL_ES_2
    if (pex->inRenderText)
        glPushAttrib(GL_ENABLE_BIT | GL_VIEWPORT_BIT | GL_SCISSOR_BIT);
#endif

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);

    glViewport(0, 0, oldWidth, oldHeight);

    float vertexCoordinateArray[] = { -1, -1, 1, -1, 1, 1, -1, 1 };
    float textureCoordinateArray[] = { 0, 0, 1, 0, 1, 1, 0, 1 };

    glEnableVertexAttribArray(QT_VERTEX_COORDS_ATTR);
    glEnableVertexAttribArray(QT_TEXTURE_COORDS_ATTR);

    glVertexAttribPointer(QT_VERTEX_COORDS_ATTR, 2, GL_FLOAT, GL_FALSE, 0, vertexCoordinateArray);
    glVertexAttribPointer(QT_TEXTURE_COORDS_ATTR, 2, GL_FLOAT, GL_FALSE, 0, textureCoordinateArray);

    pex->shaderManager->blitProgram()->enable();
    pex->shaderManager->blitProgram()->setUniformValue("imageTexture", QT_IMAGE_TEXTURE_UNIT);
    pex->shaderManager->setDirty();

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisableVertexAttribArray(QT_VERTEX_COORDS_ATTR);
    glDisableVertexAttribArray(QT_TEXTURE_COORDS_ATTR);

    glBindTexture(GL_TEXTURE_2D, m_texture);

#ifdef QT_OPENGL_ES_2
    QDataBuffer<uchar> buffer(4*oldWidth*oldHeight);
    buffer.resize(4*oldWidth*oldHeight);
    glReadPixels(0, 0, oldWidth, oldHeight, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());

    // do an in-place conversion from GL_RGBA to GL_ALPHA
    for (int i=0; i<oldWidth*oldHeight; ++i)
        buffer.data()[i] = buffer.at(4*i + 3);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, oldWidth, oldHeight,
                    GL_ALPHA, GL_UNSIGNED_BYTE, buffer.data());
#else
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, oldWidth, oldHeight);
#endif

    glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                              GL_RENDERBUFFER_EXT, 0);
    glDeleteTextures(1, &tmp_texture);
    glDeleteTextures(1, &oldTexture);

    glBindFramebuffer(GL_FRAMEBUFFER_EXT, ctx->d_ptr->current_fbo);

    glViewport(0, 0, pex->width, pex->height);
    pex->updateDepthScissorTest();

#ifndef QT_OPENGL_ES_2
    if (pex->inRenderText)
        glPopAttrib();
#endif
}

void QGLTextureGlyphCache::fillTexture(const Coord &c, glyph_t glyph)
{
    QImage mask = textureMapForGlyph(glyph);

    const uint maskWidth = mask.width();
    const uint maskHeight = mask.height();

    glBindTexture(GL_TEXTURE_2D, m_texture);
    if (mask.format() == QImage::Format_RGB32) {
        glTexSubImage2D(GL_TEXTURE_2D, 0, c.x, c.y, maskWidth, maskHeight, GL_BGRA, GL_UNSIGNED_BYTE, mask.bits());
    } else {
#ifdef QT_OPENGL_ES2
        glTexSubImage2D(GL_TEXTURE_2D, 0, c.x, c.y, maskWidth, maskHeight, GL_ALPHA, GL_UNSIGNED_BYTE, mask.bits());
#else
        // glTexSubImage2D() might cause some garbage to appear in the texture if the mask width is
        // not a multiple of four bytes. The bug appeared on a computer with 32-bit Windows Vista
        // and nVidia GeForce 8500GT. GL_UNPACK_ALIGNMENT is set to four bytes, 'mask' has a
        // multiple of four bytes per line, and most of the glyph shows up correctly in the
        // texture, which makes me think that this is a driver bug.
        // One workaround is to make sure the mask width is a multiple of four bytes, for instance
        // by converting it to a format with four bytes per pixel. Another is to copy one line at a
        // time.

        for (uint i = 0; i < maskHeight; ++i)
            glTexSubImage2D(GL_TEXTURE_2D, 0, c.x, c.y + i, maskWidth, 1, GL_ALPHA, GL_UNSIGNED_BYTE, mask.scanLine(i));
#endif
    }
}

int QGLTextureGlyphCache::glyphMargin() const
{
#ifdef Q_WS_MAC
    return 2;
#else
    return m_type == QFontEngineGlyphCache::Raster_RGBMask ? 2 : 0;
#endif
}

extern QImage qt_imageForBrush(int brushStyle, bool invert);

////////////////////////////////// Private Methods //////////////////////////////////////////

QGL2PaintEngineExPrivate::~QGL2PaintEngineExPrivate()
{
    delete shaderManager;
}

void QGL2PaintEngineExPrivate::updateTextureFilter(GLenum target, GLenum wrapMode, bool smoothPixmapTransform, GLuint id)
{
//    glActiveTexture(GL_TEXTURE0 + QT_BRUSH_TEXTURE_UNIT); //### Is it always this texture unit?
    if (id != GLuint(-1) && id == lastTexture)
        return;

    lastTexture = id;

    if (smoothPixmapTransform) {
        glTexParameterf(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    } else {
        glTexParameterf(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterf(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
    glTexParameterf(target, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameterf(target, GL_TEXTURE_WRAP_T, wrapMode);
}


QColor QGL2PaintEngineExPrivate::premultiplyColor(QColor c, GLfloat opacity)
{
    qreal alpha = c.alphaF() * opacity;
    c.setAlphaF(alpha);
    c.setRedF(c.redF() * alpha);
    c.setGreenF(c.greenF() * alpha);
    c.setBlueF(c.blueF() * alpha);
    return c;
}


void QGL2PaintEngineExPrivate::setBrush(const QBrush* brush)
{
    currentBrush = brush;
    brushTextureDirty = true;
    brushUniformsDirty = true;
    if (currentBrush->style() == Qt::TexturePattern
        && qHasPixmapTexture(*brush) && brush->texture().isQBitmap())
    {
        shaderManager->setSrcPixelType(QGLEngineShaderManager::TextureSrcWithPattern);
    } else {
        shaderManager->setSrcPixelType(currentBrush->style());
    }
    shaderManager->optimiseForBrushTransform(currentBrush->transform());
}


// Unless this gets used elsewhere, it's probably best to merge it into fillStencilWithVertexArray
void QGL2PaintEngineExPrivate::useSimpleShader()
{
    shaderManager->simpleProgram()->enable();
    shaderManager->setDirty();

    if (matrixDirty)
        updateMatrix();

    if (simpleShaderMatrixUniformDirty) {
        shaderManager->simpleProgram()->setUniformValue("pmvMatrix", pmvMatrix);
        simpleShaderMatrixUniformDirty = false;
    }

    if (simpleShaderDepthUniformDirty) {
        shaderManager->simpleProgram()->setUniformValue("depth", normalizedDeviceDepth(q->state()->currentDepth));
        simpleShaderDepthUniformDirty = false;
    }
}

void QGL2PaintEngineExPrivate::updateBrushTexture()
{
    Q_Q(QGL2PaintEngineEx);
//     qDebug("QGL2PaintEngineExPrivate::updateBrushTexture()");
    Qt::BrushStyle style = currentBrush->style();

    if ( (style >= Qt::Dense1Pattern) && (style <= Qt::DiagCrossPattern) ) {
        // Get the image data for the pattern
        QImage texImage = qt_imageForBrush(style, false);

        glActiveTexture(GL_TEXTURE0 + QT_BRUSH_TEXTURE_UNIT);
        ctx->d_func()->bindTexture(texImage, GL_TEXTURE_2D, GL_RGBA, true, QGLContext::InternalBindOption);
        updateTextureFilter(GL_TEXTURE_2D, GL_REPEAT, q->state()->renderHints & QPainter::SmoothPixmapTransform);
    }
    else if (style >= Qt::LinearGradientPattern && style <= Qt::ConicalGradientPattern) {
        // Gradiant brush: All the gradiants use the same texture

        const QGradient* g = currentBrush->gradient();

        // We apply global opacity in the fragment shaders, so we always pass 1.0
        // for opacity to the cache.
        GLuint texId = QGL2GradientCache::cacheForContext(ctx)->getBuffer(*g, 1.0);

        glActiveTexture(GL_TEXTURE0 + QT_BRUSH_TEXTURE_UNIT);
        glBindTexture(GL_TEXTURE_2D, texId);

        if (g->spread() == QGradient::RepeatSpread || g->type() == QGradient::ConicalGradient)
            updateTextureFilter(GL_TEXTURE_2D, GL_REPEAT, q->state()->renderHints & QPainter::SmoothPixmapTransform);
        else if (g->spread() == QGradient::ReflectSpread)
            updateTextureFilter(GL_TEXTURE_2D, GL_MIRRORED_REPEAT_IBM, q->state()->renderHints & QPainter::SmoothPixmapTransform);
        else
            updateTextureFilter(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, q->state()->renderHints & QPainter::SmoothPixmapTransform);
    }
    else if (style == Qt::TexturePattern) {
        const QPixmap& texPixmap = currentBrush->texture();

        glActiveTexture(GL_TEXTURE0 + QT_BRUSH_TEXTURE_UNIT);
        QGLTexture *tex = ctx->d_func()->bindTexture(texPixmap, GL_TEXTURE_2D, GL_RGBA, QGLContext::InternalBindOption);
        updateTextureFilter(GL_TEXTURE_2D, GL_REPEAT, q->state()->renderHints & QPainter::SmoothPixmapTransform);
        textureInvertedY = tex->options & QGLContext::InvertedYBindOption ? -1 : 1;
    }
    brushTextureDirty = false;
}


void QGL2PaintEngineExPrivate::updateBrushUniforms()
{
//     qDebug("QGL2PaintEngineExPrivate::updateBrushUniforms()");
    Qt::BrushStyle style = currentBrush->style();

    if (style == Qt::NoBrush)
        return;

    QTransform brushQTransform = currentBrush->transform();

    if (style == Qt::SolidPattern) {
        QColor col = premultiplyColor(currentBrush->color(), (GLfloat)q->state()->opacity);
        shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::FragmentColor), col);
    }
    else {
        // All other brushes have a transform and thus need the translation point:
        QPointF translationPoint;

        if (style <= Qt::DiagCrossPattern) {
            translationPoint = q->state()->brushOrigin;

            QColor col = premultiplyColor(currentBrush->color(), (GLfloat)q->state()->opacity);

            shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::PatternColor), col);

            QVector2D halfViewportSize(width*0.5, height*0.5);
            shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::HalfViewportSize), halfViewportSize);
        }
        else if (style == Qt::LinearGradientPattern) {
            const QLinearGradient *g = static_cast<const QLinearGradient *>(currentBrush->gradient());

            QPointF realStart = g->start();
            QPointF realFinal = g->finalStop();
            translationPoint = realStart;

            QPointF l = realFinal - realStart;

            QVector3D linearData(
                l.x(),
                l.y(),
                1.0f / (l.x() * l.x() + l.y() * l.y())
            );

            shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::LinearData), linearData);

            QVector2D halfViewportSize(width*0.5, height*0.5);
            shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::HalfViewportSize), halfViewportSize);
        }
        else if (style == Qt::ConicalGradientPattern) {
            const QConicalGradient *g = static_cast<const QConicalGradient *>(currentBrush->gradient());
            translationPoint   = g->center();

            GLfloat angle = -(g->angle() * 2 * Q_PI) / 360.0;

            shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::Angle), angle);

            QVector2D halfViewportSize(width*0.5, height*0.5);
            shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::HalfViewportSize), halfViewportSize);
        }
        else if (style == Qt::RadialGradientPattern) {
            const QRadialGradient *g = static_cast<const QRadialGradient *>(currentBrush->gradient());
            QPointF realCenter = g->center();
            QPointF realFocal  = g->focalPoint();
            qreal   realRadius = g->radius();
            translationPoint   = realFocal;

            QPointF fmp = realCenter - realFocal;
            shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::Fmp), fmp);

            GLfloat fmp2_m_radius2 = -fmp.x() * fmp.x() - fmp.y() * fmp.y() + realRadius*realRadius;
            shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::Fmp2MRadius2), fmp2_m_radius2);
            shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::Inverse2Fmp2MRadius2),
                                                             GLfloat(1.0 / (2.0*fmp2_m_radius2)));

            QVector2D halfViewportSize(width*0.5, height*0.5);
            shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::HalfViewportSize), halfViewportSize);
        }
        else if (style == Qt::TexturePattern) {
            translationPoint = q->state()->brushOrigin;

            const QPixmap& texPixmap = currentBrush->texture();

            if (qHasPixmapTexture(*currentBrush) && currentBrush->texture().isQBitmap()) {
                QColor col = premultiplyColor(currentBrush->color(), (GLfloat)q->state()->opacity);
                shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::PatternColor), col);
            }

            QSizeF invertedTextureSize(1.0 / texPixmap.width(), 1.0 * textureInvertedY / texPixmap.height());
            shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::InvertedTextureSize), invertedTextureSize);

            QVector2D halfViewportSize(width*0.5, height*0.5);
            shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::HalfViewportSize), halfViewportSize);
        }
        else
            qWarning("QGL2PaintEngineEx: Unimplemented fill style");

        QTransform translate(1, 0, 0, 1, -translationPoint.x(), -translationPoint.y());
        QTransform gl_to_qt(1, 0, 0, -1, 0, height);
        QTransform inv_matrix = gl_to_qt * (brushQTransform * q->state()->matrix).inverted() * translate;

        shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::BrushTransform), inv_matrix);
        shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::BrushTexture), QT_BRUSH_TEXTURE_UNIT);
    }
    brushUniformsDirty = false;
}


// This assumes the shader manager has already setup the correct shader program
void QGL2PaintEngineExPrivate::updateMatrix()
{
//     qDebug("QGL2PaintEngineExPrivate::updateMatrix()");

    // We setup the Projection matrix to be the equivilant of glOrtho(0, w, h, 0, -1, 1):
    GLfloat P[4][4] = {
        {2.0/width,  0.0,        0.0, -1.0},
        {0.0,       -2.0/height, 0.0,  1.0},
        {0.0,        0.0,       -1.0,  0.0},
        {0.0,        0.0,        0.0,  1.0}
    };

    const QTransform& transform = q->state()->matrix;

    if (mode == TextDrawingMode) {
        // Text drawing mode is only used for non-scaling transforms
        for (int row = 0; row < 4; ++row)
            for (int col = 0; col < 4; ++col)
                pmvMatrix[col][row] = P[row][col];

        pmvMatrix[3][0] += P[0][0] * qRound(transform.dx());
        pmvMatrix[3][1] += P[1][1] * qRound(transform.dy());

        inverseScale = 1;
    } else {
        // Use the (3x3) transform for the Model~View matrix:
        GLfloat MV[4][4] = {
            {transform.m11(), transform.m21(), 0.0, transform.dx()},
            {transform.m12(), transform.m22(), 0.0, transform.dy()},
            {0.0,             0.0,             1.0, 0.0},
            {transform.m13(), transform.m23(), 0.0, transform.m33()}
        };

        // NOTE: OpenGL ES works with column-major matrices, so when we multiply the matrices,
        //       we also transpose them ready for GL.
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                pmvMatrix[col][row] = 0.0;

                // P[row][n] is 0.0 for n < row
                for (int n = row; n < 4; ++n)
                    pmvMatrix[col][row] += P[row][n] * MV[n][col];
            }
        }

        // 1/10000 == 0.0001, so we have good enough res to cover curves
        // that span the entire widget...
        inverseScale = qMax(1 / qMax( qMax(qAbs(transform.m11()), qAbs(transform.m22())),
                    qMax(qAbs(transform.m12()), qAbs(transform.m21())) ),
                qreal(0.0001));
    }

    matrixDirty = false;

    // The actual data has been updated so both shader program's uniforms need updating
    simpleShaderMatrixUniformDirty = true;
    shaderMatrixUniformDirty = true;
}


void QGL2PaintEngineExPrivate::updateCompositionMode()
{
    // NOTE: The entire paint engine works on pre-multiplied data - which is why some of these
    //       composition modes look odd.
//     qDebug() << "QGL2PaintEngineExPrivate::updateCompositionMode() - Setting GL composition mode for " << q->state()->composition_mode;
    switch(q->state()->composition_mode) {
    case QPainter::CompositionMode_SourceOver:
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        break;
    case QPainter::CompositionMode_DestinationOver:
        glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
        break;
    case QPainter::CompositionMode_Clear:
        glBlendFunc(GL_ZERO, GL_ZERO);
        break;
    case QPainter::CompositionMode_Source:
        glBlendFunc(GL_ONE, GL_ZERO);
        break;
    case QPainter::CompositionMode_Destination:
        glBlendFunc(GL_ZERO, GL_ONE);
        break;
    case QPainter::CompositionMode_SourceIn:
        glBlendFunc(GL_DST_ALPHA, GL_ZERO);
        break;
    case QPainter::CompositionMode_DestinationIn:
        glBlendFunc(GL_ZERO, GL_SRC_ALPHA);
        break;
    case QPainter::CompositionMode_SourceOut:
        glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ZERO);
        break;
    case QPainter::CompositionMode_DestinationOut:
        glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
        break;
    case QPainter::CompositionMode_SourceAtop:
        glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        break;
    case QPainter::CompositionMode_DestinationAtop:
        glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_SRC_ALPHA);
        break;
    case QPainter::CompositionMode_Xor:
        glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        break;
    case QPainter::CompositionMode_Plus:
        glBlendFunc(GL_ONE, GL_ONE);
        break;
    default:
        qWarning("Unsupported composition mode");
        break;
    }

    compositionModeDirty = false;
}

static inline void setCoords(GLfloat *coords, const QGLRect &rect)
{
    coords[0] = rect.left;
    coords[1] = rect.top;
    coords[2] = rect.right;
    coords[3] = rect.top;
    coords[4] = rect.right;
    coords[5] = rect.bottom;
    coords[6] = rect.left;
    coords[7] = rect.bottom;
}

void QGL2PaintEngineExPrivate::drawTexture(const QGLRect& dest, const QGLRect& src, const QSize &textureSize, bool opaque, bool pattern)
{
    // Setup for texture drawing
    shaderManager->setSrcPixelType(pattern ? QGLEngineShaderManager::PatternSrc : QGLEngineShaderManager::ImageSrc);
    if (prepareForDraw(opaque))
        shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::ImageTexture), QT_IMAGE_TEXTURE_UNIT);

    if (pattern) {
        QColor col = premultiplyColor(q->state()->pen.color(), (GLfloat)q->state()->opacity);
        shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::PatternColor), col);
    }

    GLfloat dx = 1.0 / textureSize.width();
    GLfloat dy = 1.0 / textureSize.height();

    QGLRect srcTextureRect(src.left*dx, src.top*dy, src.right*dx, src.bottom*dy);

    setCoords(staticVertexCoordinateArray, dest);
    setCoords(staticTextureCoordinateArray, srcTextureRect);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void QGL2PaintEngineEx::beginNativePainting()
{
    Q_D(QGL2PaintEngineEx);
    ensureActive();
    d->transferMode(BrushDrawingMode);

    QGLContext *ctx = d->ctx;
    glUseProgram(0);

#ifndef QT_OPENGL_ES_2
    // be nice to people who mix OpenGL 1.x code with QPainter commands
    // by setting modelview and projection matrices to mirror the GL 1
    // paint engine
    const QTransform& mtx = state()->matrix;

    float mv_matrix[4][4] =
    {
        { mtx.m11(), mtx.m12(),     0, mtx.m13() },
        { mtx.m21(), mtx.m22(),     0, mtx.m23() },
        {         0,         0,     1,         0 },
        {  mtx.dx(),  mtx.dy(),     0, mtx.m33() }
    };

    const QSize sz = d->device->size();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, sz.width(), sz.height(), 0, -999999, 999999);

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(&mv_matrix[0][0]);
#endif

    d->lastTexture = GLuint(-1);
    d->resetGLState();

    d->needsSync = true;
}

void QGL2PaintEngineExPrivate::resetGLState()
{
    glDisable(GL_BLEND);
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glDepthMask(true);
    glClearDepth(1);
}

void QGL2PaintEngineEx::endNativePainting()
{
    Q_D(QGL2PaintEngineEx);
    d->needsSync = true;
}

const QGLContext *QGL2PaintEngineEx::context()
{
    Q_D(QGL2PaintEngineEx);
    return d->ctx;
}

void QGL2PaintEngineExPrivate::transferMode(EngineMode newMode)
{
    if (newMode == mode)
        return;

    if (mode == TextDrawingMode || mode == ImageDrawingMode) {
        glDisableVertexAttribArray(QT_TEXTURE_COORDS_ATTR);
        glDisableVertexAttribArray(QT_VERTEX_COORDS_ATTR);

        lastTexture = GLuint(-1);
    }

    if (mode == TextDrawingMode)
        matrixDirty = true;

    if (newMode == TextDrawingMode) {
        glEnableVertexAttribArray(QT_VERTEX_COORDS_ATTR);
        glEnableVertexAttribArray(QT_TEXTURE_COORDS_ATTR);

        glVertexAttribPointer(QT_VERTEX_COORDS_ATTR, 2, GL_FLOAT, GL_FALSE, 0, vertexCoordinateArray.data());
        glVertexAttribPointer(QT_TEXTURE_COORDS_ATTR, 2, GL_FLOAT, GL_FALSE, 0, textureCoordinateArray.data());

        matrixDirty = true;
    }

    if (newMode == ImageDrawingMode) {
        glEnableVertexAttribArray(QT_VERTEX_COORDS_ATTR);
        glEnableVertexAttribArray(QT_TEXTURE_COORDS_ATTR);

        glVertexAttribPointer(QT_VERTEX_COORDS_ATTR, 2, GL_FLOAT, GL_FALSE, 0, staticVertexCoordinateArray);
        glVertexAttribPointer(QT_TEXTURE_COORDS_ATTR, 2, GL_FLOAT, GL_FALSE, 0, staticTextureCoordinateArray);
    }

    // This needs to change when we implement high-quality anti-aliasing...
    if (newMode != TextDrawingMode)
        shaderManager->setMaskType(QGLEngineShaderManager::NoMask);

    mode = newMode;
}

void QGL2PaintEngineExPrivate::drawOutline(const QVectorPath& path)
{
    transferMode(BrushDrawingMode);

    // Might need to call updateMatrix to re-calculate inverseScale
    if (matrixDirty)
        updateMatrix();

    vertexCoordinateArray.clear();
    vertexCoordinateArray.addPath(path, inverseScale);

    if (path.hasImplicitClose()) {
        // Close the path's outline
        vertexCoordinateArray.lineToArray(path.points()[0], path.points()[1]);
        vertexCoordinateArray.stops().last() += 1;
    }

    prepareForDraw(currentBrush->isOpaque());
    drawVertexArrays(vertexCoordinateArray, GL_LINE_STRIP);
}


// Assumes everything is configured for the brush you want to use
void QGL2PaintEngineExPrivate::fill(const QVectorPath& path)
{
    transferMode(BrushDrawingMode);

    // Might need to call updateMatrix to re-calculate inverseScale
    if (matrixDirty)
        updateMatrix();

    const QPointF* const points = reinterpret_cast<const QPointF*>(path.points());

    // Check to see if there's any hints
    if (path.shape() == QVectorPath::RectangleHint) {
        QGLRect rect(points[0].x(), points[0].y(), points[2].x(), points[2].y());
        prepareForDraw(currentBrush->isOpaque());

        composite(rect);
    }
    else if (path.shape() == QVectorPath::EllipseHint) {
        vertexCoordinateArray.clear();
        vertexCoordinateArray.addPath(path, inverseScale);
        prepareForDraw(currentBrush->isOpaque());
        drawVertexArrays(vertexCoordinateArray, GL_TRIANGLE_FAN);
    }
    else {
        // The path is too complicated & needs the stencil technique
        vertexCoordinateArray.clear();
        vertexCoordinateArray.addPath(path, inverseScale);

        fillStencilWithVertexArray(vertexCoordinateArray, path.hasWindingFill());

        // Stencil the brush onto the dest buffer
        glStencilFunc(GL_NOTEQUAL, 0, 0xFFFF); // Pass if stencil buff value != 0
        glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

        glEnable(GL_STENCIL_TEST);
        prepareForDraw(currentBrush->isOpaque());

#ifndef QT_OPENGL_ES_2
        if (inRenderText)
            shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::Depth), zValueForRenderText());
#endif
        composite(vertexCoordinateArray.boundingRect());
        glDisable(GL_STENCIL_TEST);

        glStencilMask(0);
    }
}


void QGL2PaintEngineExPrivate::fillStencilWithVertexArray(QGL2PEXVertexArray& vertexArray, bool useWindingFill)
{
//     qDebug("QGL2PaintEngineExPrivate::fillStencilWithVertexArray()");
    glStencilMask(0xFFFF); // Enable stencil writes

    if (stencilBufferDirty) {
        // Clear the stencil buffer to zeros
        glDisable(GL_STENCIL_TEST);
        glClearStencil(0); // Clear to zero
        glClear(GL_STENCIL_BUFFER_BIT);
        stencilBufferDirty = false;
    }

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Disable color writes
    glStencilFunc(GL_ALWAYS, 0, 0xFFFF); // Always pass the stencil test

    // Setup the stencil op:
    if (useWindingFill) {
        glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR_WRAP); // Inc. for front-facing triangle
        glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR_WRAP); //Dec. for back-facing "holes"
    } else
        glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT); // Simply invert the stencil bit

    // No point in using a fancy gradient shader for writing into the stencil buffer!
    useSimpleShader();

    glEnable(GL_STENCIL_TEST); // For some reason, this has to happen _after_ the simple shader is use()'d
    glDisable(GL_BLEND);

#ifndef QT_OPENGL_ES_2
    if (inRenderText) {
        glPushAttrib(GL_ENABLE_BIT);
        glDisable(GL_DEPTH_TEST);
    }
#endif

    // Draw the vertecies into the stencil buffer:
    drawVertexArrays(vertexArray, GL_TRIANGLE_FAN);

#ifndef QT_OPENGL_ES_2
    if (inRenderText)
        glPopAttrib();
#endif

    // Enable color writes & disable stencil writes
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

bool QGL2PaintEngineExPrivate::prepareForDraw(bool srcPixelsAreOpaque)
{
    if (brushTextureDirty && mode != ImageDrawingMode)
        updateBrushTexture();

    if (compositionModeDirty)
        updateCompositionMode();

    if (matrixDirty)
        updateMatrix();

    const bool stateHasOpacity = q->state()->opacity < 0.99f;
    if (q->state()->composition_mode == QPainter::CompositionMode_Source
        || (q->state()->composition_mode == QPainter::CompositionMode_SourceOver
            && srcPixelsAreOpaque && !stateHasOpacity))
    {
        glDisable(GL_BLEND);
    } else {
        glEnable(GL_BLEND);
    }

    bool useGlobalOpacityUniform = stateHasOpacity;
    if (stateHasOpacity && (mode != ImageDrawingMode)) {
        // Using a brush
        bool brushIsPattern = (currentBrush->style() >= Qt::Dense1Pattern) &&
                              (currentBrush->style() <= Qt::DiagCrossPattern);

        if ((currentBrush->style() == Qt::SolidPattern) || brushIsPattern)
            useGlobalOpacityUniform = false; // Global opacity handled by srcPixel shader
    }
    shaderManager->setUseGlobalOpacity(useGlobalOpacityUniform);

    bool changed = shaderManager->useCorrectShaderProg();
    // If the shader program needs changing, we change it and mark all uniforms as dirty
    if (changed) {
        // The shader program has changed so mark all uniforms as dirty:
        brushUniformsDirty = true;
        shaderMatrixUniformDirty = true;
        depthUniformDirty = true;
        opacityUniformDirty = true;
    }

    if (brushUniformsDirty && mode != ImageDrawingMode)
        updateBrushUniforms();

    if (shaderMatrixUniformDirty) {
        shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::PmvMatrix), pmvMatrix);
        shaderMatrixUniformDirty = false;
    }

    if (depthUniformDirty) {
        shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::Depth), normalizedDeviceDepth(q->state()->currentDepth));
        depthUniformDirty = false;
    }

    if (useGlobalOpacityUniform && opacityUniformDirty) {
        shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::GlobalOpacity), (GLfloat)q->state()->opacity);
        opacityUniformDirty = false;
    }

    return changed;
}

void QGL2PaintEngineExPrivate::composite(const QGLRect& boundingRect)
{
    // Setup a vertex array for the bounding rect:
    GLfloat rectVerts[] = {
        boundingRect.left, boundingRect.top,
        boundingRect.left, boundingRect.bottom,
        boundingRect.right, boundingRect.bottom,
        boundingRect.right, boundingRect.top
    };

    glEnableVertexAttribArray(QT_VERTEX_COORDS_ATTR);
    glVertexAttribPointer(QT_VERTEX_COORDS_ATTR, 2, GL_FLOAT, GL_FALSE, 0, rectVerts);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisableVertexAttribArray(QT_VERTEX_COORDS_ATTR);
}

// Draws the vertex array as a set of <vertexArrayStops.size()> triangle fans.
void QGL2PaintEngineExPrivate::drawVertexArrays(QGL2PEXVertexArray& vertexArray, GLenum primitive)
{
    // Now setup the pointer to the vertex array:
    glEnableVertexAttribArray(QT_VERTEX_COORDS_ATTR);
    glVertexAttribPointer(QT_VERTEX_COORDS_ATTR, 2, GL_FLOAT, GL_FALSE, 0, vertexArray.data());

    int previousStop = 0;
    foreach(int stop, vertexArray.stops()) {
/*
        qDebug("Drawing triangle fan for vertecies %d -> %d:", previousStop, stop-1);
        for (int i=previousStop; i<stop; ++i)
            qDebug("   %02d: [%.2f, %.2f]", i, vertexArray.data()[i].x, vertexArray.data()[i].y);
*/
        glDrawArrays(primitive, previousStop, stop - previousStop);
        previousStop = stop;
    }
    glDisableVertexAttribArray(QT_VERTEX_COORDS_ATTR);
}

float QGL2PaintEngineExPrivate::zValueForRenderText() const
{
#ifndef QT_OPENGL_ES_2
    // Get the z translation value from the model view matrix and
    // transform it using the ortogonal projection with z-near = 0,
    // and z-far = 1, which is used in QGLWidget::renderText()
    GLdouble model[4][4];
    glGetDoublev(GL_MODELVIEW_MATRIX, &model[0][0]);
    return -2 * model[3][2] - 1;
#else
    return 0;
#endif
}

/////////////////////////////////// Public Methods //////////////////////////////////////////

QGL2PaintEngineEx::QGL2PaintEngineEx()
    : QPaintEngineEx(*(new QGL2PaintEngineExPrivate(this)))
{
}

QGL2PaintEngineEx::~QGL2PaintEngineEx()
{
}

void QGL2PaintEngineEx::fill(const QVectorPath &path, const QBrush &brush)
{
    Q_D(QGL2PaintEngineEx);

    if (qbrush_style(brush) == Qt::NoBrush)
        return;
    if (!d->inRenderText)
        ensureActive();
    d->setBrush(&brush);
    d->fill(path);
}

void QGL2PaintEngineEx::stroke(const QVectorPath &path, const QPen &pen)
{
    Q_D(QGL2PaintEngineEx);

    Qt::PenStyle penStyle = qpen_style(pen);
    const QBrush &penBrush = qpen_brush(pen);
    if (penStyle == Qt::NoPen || qbrush_style(penBrush) == Qt::NoBrush)
        return;

    ensureActive();

    qreal penWidth = qpen_widthf(pen);
    if ( (pen.isCosmetic() && (penStyle == Qt::SolidLine)) && (penWidth < 2.5f) )
    {
        // We only handle solid, cosmetic pens with a width of 1 pixel
        const QBrush& brush = pen.brush();
        d->setBrush(&brush);

        if (penWidth < 0.01f)
            glLineWidth(1.0);
        else
            glLineWidth(penWidth);

        d->drawOutline(path);
    } else
        return QPaintEngineEx::stroke(path, pen);
}

void QGL2PaintEngineEx::penChanged() { }
void QGL2PaintEngineEx::brushChanged() { }
void QGL2PaintEngineEx::brushOriginChanged() { }

void QGL2PaintEngineEx::opacityChanged()
{
//    qDebug("QGL2PaintEngineEx::opacityChanged()");
    Q_D(QGL2PaintEngineEx);

    Q_ASSERT(d->shaderManager);
    d->brushUniformsDirty = true;
    d->opacityUniformDirty = true;
}

void QGL2PaintEngineEx::compositionModeChanged()
{
//     qDebug("QGL2PaintEngineEx::compositionModeChanged()");
    Q_D(QGL2PaintEngineEx);
    d->compositionModeDirty = true;
}

void QGL2PaintEngineEx::renderHintsChanged()
{
#if !defined(QT_OPENGL_ES_2)
    if ((state()->renderHints & QPainter::Antialiasing)
        || (state()->renderHints & QPainter::HighQualityAntialiasing))
        glEnable(GL_MULTISAMPLE);
    else
        glDisable(GL_MULTISAMPLE);
#endif

    Q_D(QGL2PaintEngineEx);
    d->lastTexture = GLuint(-1);
    d->brushTextureDirty = true;
//    qDebug("QGL2PaintEngineEx::renderHintsChanged() not implemented!");
}

void QGL2PaintEngineEx::transformChanged()
{
    Q_D(QGL2PaintEngineEx);
    d->matrixDirty = true;
}


void QGL2PaintEngineEx::drawPixmap(const QRectF& dest, const QPixmap & pixmap, const QRectF & src)
{
    Q_D(QGL2PaintEngineEx);
    ensureActive();
    d->transferMode(ImageDrawingMode);

    QGLContext *ctx = d->ctx;
    glActiveTexture(GL_TEXTURE0 + QT_IMAGE_TEXTURE_UNIT);
    QGLTexture *texture =
        ctx->d_func()->bindTexture(pixmap, GL_TEXTURE_2D, GL_RGBA,
                                   QGLContext::InternalBindOption
                                   | QGLContext::CanFlipNativePixmapBindOption);

    GLfloat top = texture->options & QGLContext::InvertedYBindOption ? (pixmap.height() - src.top()) : src.top();
    GLfloat bottom = texture->options & QGLContext::InvertedYBindOption ? (pixmap.height() - src.bottom()) : src.bottom();
    QGLRect srcRect(src.left(), top, src.right(), bottom);

    bool isBitmap = pixmap.isQBitmap();
    bool isOpaque = !isBitmap && !pixmap.hasAlphaChannel();

    d->updateTextureFilter(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE,
                           state()->renderHints & QPainter::SmoothPixmapTransform, texture->id);
    d->drawTexture(dest, srcRect, pixmap.size(), isOpaque, isBitmap);
}

void QGL2PaintEngineEx::drawImage(const QRectF& dest, const QImage& image, const QRectF& src,
                        Qt::ImageConversionFlags)
{
    Q_D(QGL2PaintEngineEx);
    ensureActive();
    d->transferMode(ImageDrawingMode);

    QGLContext *ctx = d->ctx;
    glActiveTexture(GL_TEXTURE0 + QT_IMAGE_TEXTURE_UNIT);
    QGLTexture *texture = ctx->d_func()->bindTexture(image, GL_TEXTURE_2D, GL_RGBA, QGLContext::InternalBindOption);
    GLuint id = texture->id;

    d->updateTextureFilter(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE,
                           state()->renderHints & QPainter::SmoothPixmapTransform, id);
    d->drawTexture(dest, src, image.size(), !image.hasAlphaChannel());
}

void QGL2PaintEngineEx::drawTexture(const QRectF &dest, GLuint textureId, const QSize &size, const QRectF &src)
{
    Q_D(QGL2PaintEngineEx);
    ensureActive();
    d->transferMode(ImageDrawingMode);

    QGLContext *ctx = d->ctx;
    glActiveTexture(GL_TEXTURE0 + QT_IMAGE_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_2D, textureId);

    QGLRect srcRect(src.left(), src.bottom(), src.right(), src.top());

    d->updateTextureFilter(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE,
                           state()->renderHints & QPainter::SmoothPixmapTransform, textureId);
    d->drawTexture(dest, srcRect, size, false);
}

void QGL2PaintEngineEx::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
    Q_D(QGL2PaintEngineEx);

    if (!d->inRenderText)
        ensureActive();
    QOpenGL2PaintEngineState *s = state();

    const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);

    bool drawCached = true;

    if (s->matrix.type() > QTransform::TxTranslate)
        drawCached = false;

    // don't try to cache huge fonts
    if (ti.fontEngine->fontDef.pixelSize * qSqrt(s->matrix.determinant()) >= 64)
        drawCached = false;

    if (d->glyphCacheType == QFontEngineGlyphCache::Raster_RGBMask
        && state()->composition_mode != QPainter::CompositionMode_Source
        && state()->composition_mode != QPainter::CompositionMode_SourceOver) {
        drawCached = false;
    }

    if (drawCached) {
        d->drawCachedGlyphs(p, ti);
        return;
    }

    QPaintEngineEx::drawTextItem(p, ti);
}

void QGL2PaintEngineExPrivate::drawCachedGlyphs(const QPointF &p, const QTextItemInt &ti)
{
    Q_Q(QGL2PaintEngineEx);
    QOpenGL2PaintEngineState *s = q->state();

    QVarLengthArray<QFixedPoint> positions;
    QVarLengthArray<glyph_t> glyphs;
    QTransform matrix = QTransform::fromTranslate(p.x(), p.y());
    ti.fontEngine->getGlyphPositions(ti.glyphs, matrix, ti.flags, glyphs, positions);

    QFontEngineGlyphCache::Type glyphType = ti.fontEngine->glyphFormat >= 0
        ? QFontEngineGlyphCache::Type(ti.fontEngine->glyphFormat)
        : glyphCacheType;

    QGLTextureGlyphCache *cache =
        (QGLTextureGlyphCache *) ti.fontEngine->glyphCache(ctx, s->matrix);
    if (!cache) {
        cache = new QGLTextureGlyphCache(ctx, glyphType, s->matrix);
        ti.fontEngine->setGlyphCache(ctx, cache);
    }

    cache->setPaintEnginePrivate(this);
    cache->populate(ti, glyphs, positions);

    if (cache->width() == 0 || cache->height() == 0)
        return;

    if (inRenderText)
        transferMode(BrushDrawingMode);
    transferMode(TextDrawingMode);

    int margin = cache->glyphMargin();

    GLfloat dx = 1.0 / cache->width();
    GLfloat dy = 1.0 / cache->height();

    QGLPoint *oldVertexCoordinateDataPtr = vertexCoordinateArray.data();
    QGLPoint *oldTextureCoordinateDataPtr = textureCoordinateArray.data();

    vertexCoordinateArray.clear();
    textureCoordinateArray.clear();

    for (int i=0; i<glyphs.size(); ++i) {
        const QTextureGlyphCache::Coord &c = cache->coords.value(glyphs[i]);
        int x = positions[i].x.toInt() + c.baseLineX - margin;
        int y = positions[i].y.toInt() - c.baseLineY - margin;

        vertexCoordinateArray.addRect(QRectF(x, y, c.w, c.h));
        textureCoordinateArray.addRect(QRectF(c.x*dx, c.y*dy, c.w * dx, c.h * dy));
    }

    if (vertexCoordinateArray.data() != oldVertexCoordinateDataPtr)
        glVertexAttribPointer(QT_VERTEX_COORDS_ATTR, 2, GL_FLOAT, GL_FALSE, 0, vertexCoordinateArray.data());
    if (textureCoordinateArray.data() != oldTextureCoordinateDataPtr)
        glVertexAttribPointer(QT_TEXTURE_COORDS_ATTR, 2, GL_FLOAT, GL_FALSE, 0, textureCoordinateArray.data());

    QBrush pensBrush = q->state()->pen.brush();
    setBrush(&pensBrush);

    if (glyphType == QFontEngineGlyphCache::Raster_A8) {

        // Greyscale antialiasing

        shaderManager->setMaskType(QGLEngineShaderManager::PixelMask);
        prepareForDraw(false); // Text always causes src pixels to be transparent
    } else if (glyphType == QFontEngineGlyphCache::Raster_RGBMask) {

        // Subpixel antialiasing without gamma correction

        QPainter::CompositionMode compMode = q->state()->composition_mode;
        Q_ASSERT(compMode == QPainter::CompositionMode_Source
            || compMode == QPainter::CompositionMode_SourceOver);

        shaderManager->setMaskType(QGLEngineShaderManager::SubPixelMaskPass1);

        if (pensBrush.style() == Qt::SolidPattern) {
            // Solid patterns can get away with only one pass.
            QColor c = pensBrush.color();
            qreal oldOpacity = q->state()->opacity;
            if (compMode == QPainter::CompositionMode_Source) {
                c = premultiplyColor(c, q->state()->opacity);
                q->state()->opacity = 1;
                opacityUniformDirty = true;
            }

            compositionModeDirty = false; // I can handle this myself, thank you very much
            prepareForDraw(false); // Text always causes src pixels to be transparent

            // prepareForDraw() have set the opacity on the current shader, so the opacity state can now be reset.
            if (compMode == QPainter::CompositionMode_Source) {
                q->state()->opacity = oldOpacity;
                opacityUniformDirty = true;
            }

            glEnable(GL_BLEND);
            glBlendFunc(GL_CONSTANT_COLOR, GL_ONE_MINUS_SRC_COLOR);
            glBlendColor(c.redF(), c.greenF(), c.blueF(), c.alphaF());
        } else {
            // Other brush styles need two passes.

            qreal oldOpacity = q->state()->opacity;
            if (compMode == QPainter::CompositionMode_Source) {
                q->state()->opacity = 1;
                opacityUniformDirty = true;
                pensBrush = Qt::white;
                setBrush(&pensBrush);
            }

            compositionModeDirty = false; // I can handle this myself, thank you very much
            prepareForDraw(false); // Text always causes src pixels to be transparent
            glEnable(GL_BLEND);
            glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);

            glActiveTexture(GL_TEXTURE0 + QT_MASK_TEXTURE_UNIT);
            glBindTexture(GL_TEXTURE_2D, cache->texture());
            updateTextureFilter(GL_TEXTURE_2D, GL_REPEAT, false);

#ifndef QT_OPENGL_ES_2
            if (inRenderText)
                shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::Depth), zValueForRenderText());
#endif
            shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::MaskTexture), QT_MASK_TEXTURE_UNIT);
            glDrawArrays(GL_TRIANGLES, 0, 6 * glyphs.size());

            shaderManager->setMaskType(QGLEngineShaderManager::SubPixelMaskPass2);

            if (compMode == QPainter::CompositionMode_Source) {
                q->state()->opacity = oldOpacity;
                opacityUniformDirty = true;
                pensBrush = q->state()->pen.brush();
                setBrush(&pensBrush);
            }

            compositionModeDirty = false;
            prepareForDraw(false); // Text always causes src pixels to be transparent
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
        }
        compositionModeDirty = true;
    }
    //### TODO: Gamma correction

    glActiveTexture(GL_TEXTURE0 + QT_MASK_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_2D, cache->texture());
    updateTextureFilter(GL_TEXTURE_2D, GL_REPEAT, false);

#ifndef QT_OPENGL_ES_2
    if (inRenderText)
        shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::Depth), zValueForRenderText());
#endif
    shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::MaskTexture), QT_MASK_TEXTURE_UNIT);
    glDrawArrays(GL_TRIANGLES, 0, 6 * glyphs.size());
}

bool QGL2PaintEngineEx::begin(QPaintDevice *pdev)
{
    Q_D(QGL2PaintEngineEx);

//     qDebug("QGL2PaintEngineEx::begin()");
    if (pdev->devType() == QInternal::OpenGL)
        d->device = static_cast<QGLPaintDevice*>(pdev);
    else
        d->device = QGLPaintDevice::getDevice(pdev);

    if (!d->device)
        return false;

    d->ctx = d->device->context();
    d->ctx->d_ptr->active_engine = this;

    const QSize sz = d->device->size();
    d->width = sz.width();
    d->height = sz.height();
    d->last_created_state = 0;
    d->mode = BrushDrawingMode;
    d->brushTextureDirty = true;
    d->brushUniformsDirty = true;
    d->matrixDirty = true;
    d->compositionModeDirty = true;
    d->stencilBufferDirty = true;
    d->simpleShaderDepthUniformDirty = true;
    d->depthUniformDirty = true;
    d->opacityUniformDirty = true;
    d->needsSync = true;
    d->use_system_clip = !systemClip().isEmpty();

    // Calling begin paint should make the correct context current. So, any
    // code which calls into GL or otherwise needs a current context *must*
    // go after beginPaint:
    d->device->beginPaint();

#if !defined(QT_OPENGL_ES_2)
    bool success = qt_resolve_version_2_0_functions(d->ctx);
    Q_ASSERT(success);
#endif

    d->shaderManager = new QGLEngineShaderManager(d->ctx);

    if (!d->inRenderText) {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_SCISSOR_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(false);
    }

#if !defined(QT_OPENGL_ES_2)
    glDisable(GL_MULTISAMPLE);
#endif

    if (d->device->format().alpha()
#if defined(Q_WS_WIN)
        || !qt_cleartype_enabled
#endif
        ) {
        d->glyphCacheType = QFontEngineGlyphCache::Raster_A8;
    } else {
        d->glyphCacheType = QFontEngineGlyphCache::Raster_RGBMask;
    }

    d->systemStateChanged();
    return true;
}

bool QGL2PaintEngineEx::end()
{
    Q_D(QGL2PaintEngineEx);
    QGLContext *ctx = d->ctx;

    glUseProgram(0);
    d->transferMode(BrushDrawingMode);
    d->device->endPaint();

#if defined(Q_WS_X11)
    // On some (probably all) drivers, deleting an X pixmap which has been bound to a texture
    // before calling glFinish/swapBuffers renders garbage. Presumably this is because X deletes
    // the pixmap behind the driver's back before it's had a chance to use it. To fix this, we
    // reference all QPixmaps which have been bound to stop them being deleted and only deref
    // them here, after swapBuffers, where they can be safely deleted.
    ctx->d_func()->boundPixmaps.clear();
#endif
    d->ctx->d_ptr->active_engine = 0;

    d->resetGLState();

    delete d->shaderManager;
    d->shaderManager = 0;

    return false;
}

void QGL2PaintEngineEx::ensureActive()
{
    Q_D(QGL2PaintEngineEx);
    QGLContext *ctx = d->ctx;

    if (isActive() && ctx->d_ptr->active_engine != this) {
        ctx->d_ptr->active_engine = this;
        d->needsSync = true;
    }

    d->device->ensureActiveTarget();

    if (d->needsSync) {
        d->transferMode(BrushDrawingMode);
        glViewport(0, 0, d->width, d->height);
        glDepthMask(false);
        glDepthFunc(GL_LESS);
        d->needsSync = false;
        setState(state());
    }
}

void QGL2PaintEngineExPrivate::updateDepthScissorTest()
{
    Q_Q(QGL2PaintEngineEx);
    if (q->state()->depthTestEnabled)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);

    if (q->state()->scissorTestEnabled) {
        QRect bounds = q->state()->rectangleClip;
        if (bounds.isNull() || !q->painter()->hasClipping()) {
            if (use_system_clip)
                bounds = systemClip.boundingRect();
            else
                bounds = QRect(0, 0, width, height);
        }

        glEnable(GL_SCISSOR_TEST);
        setScissor(bounds);
    } else {
        glDisable(GL_SCISSOR_TEST);
    }
}

void QGL2PaintEngineExPrivate::setScissor(const QRect &rect)
{
    const int left = rect.left();
    const int width = rect.width();
    const int bottom = height - (rect.top() + rect.height());
    const int height = rect.height();

    glScissor(left, bottom, width, height);
}

void QGL2PaintEngineEx::clipEnabledChanged()
{
    Q_D(QGL2PaintEngineEx);

    d->simpleShaderDepthUniformDirty = true;
    d->depthUniformDirty = true;

    if (painter()->hasClipping()) {
        d->regenerateDepthClip();
    } else {
        if (d->use_system_clip) {
            state()->currentDepth = 0;
        } else {
            state()->depthTestEnabled = false;
        }

        d->updateDepthScissorTest();
    }
}

void QGL2PaintEngineExPrivate::writeClip(const QVectorPath &path, uint depth)
{
    transferMode(BrushDrawingMode);

    if (matrixDirty)
        updateMatrix();

    if (q->state()->needsDepthBufferClear) {
        glDepthMask(true);
        glClearDepth(rawDepth(2));
        glClear(GL_DEPTH_BUFFER_BIT);
        q->state()->needsDepthBufferClear = false;
        glDepthMask(false);
    }

    if (path.isEmpty())
        return;

    glDisable(GL_BLEND);
    glDepthMask(false);

    vertexCoordinateArray.clear();
    vertexCoordinateArray.addPath(path, inverseScale);

    glDepthMask(GL_FALSE);
    fillStencilWithVertexArray(vertexCoordinateArray, path.hasWindingFill());

    // Stencil the clip onto the clip buffer
    glColorMask(false, false, false, false);
    glDepthMask(true);

    shaderManager->simpleProgram()->setUniformValue("depth", normalizedDeviceDepth(depth));
    simpleShaderDepthUniformDirty = true;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);

    glStencilFunc(GL_NOTEQUAL, 0, 0xFFFF); // Pass if stencil buff value != 0
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

    glEnable(GL_STENCIL_TEST);
    composite(vertexCoordinateArray.boundingRect());
    glDisable(GL_STENCIL_TEST);

    glStencilMask(0);

    glColorMask(true, true, true, true);
    glDepthMask(false);
}

void QGL2PaintEngineEx::clip(const QVectorPath &path, Qt::ClipOperation op)
{
//     qDebug("QGL2PaintEngineEx::clip()");
    Q_D(QGL2PaintEngineEx);

    ensureActive();

    if (op == Qt::ReplaceClip && !d->hasClipOperations())
        op = Qt::IntersectClip;

    if (!path.isEmpty() && op == Qt::IntersectClip && (path.shape() == QVectorPath::RectangleHint)) {
        const QPointF* const points = reinterpret_cast<const QPointF*>(path.points());
        QRectF rect(points[0], points[2]);

        if (state()->matrix.type() <= QTransform::TxScale) {
            rect = state()->matrix.mapRect(rect);

            if ((d->use_system_clip && rect.contains(d->systemClip.boundingRect()))
                || rect.contains(QRect(0, 0, d->width, d->height)))
                return;

            if (state()->rectangleClip.isValid()) {
                state()->rectangleClip = state()->rectangleClip.intersected(rect.toRect());

                state()->hasRectangleClip = true;
                state()->scissorTestEnabled = true;

                glEnable(GL_SCISSOR_TEST);
                d->setScissor(state()->rectangleClip);

                return;
            }
        }
    }

    if (!state()->hasRectangleClip)
        state()->rectangleClip = QRect();

    if (state()->rectangleClip.isValid() && op != Qt::NoClip && op != Qt::ReplaceClip) {
        QPainterPath path;
        path.addRect(state()->rectangleClip);
        path = state()->matrix.inverted().map(path);

        state()->rectangleClip = QRect();
        d->updateDepthScissorTest();

        glDepthFunc(GL_ALWAYS);

        state()->maxDepth = 4;
        d->writeClip(qtVectorPathForPath(path), state()->maxDepth);
        state()->currentDepth = 3;
        state()->depthTestEnabled = true;

        glDepthFunc(GL_LESS);
        glEnable(GL_DEPTH_TEST);
    }

    switch (op) {
    case Qt::NoClip:
        if (d->use_system_clip) {
            glEnable(GL_DEPTH_TEST);
            state()->depthTestEnabled = true;
            state()->currentDepth = 0;
        } else {
            glDisable(GL_DEPTH_TEST);
            state()->depthTestEnabled = false;
        }
        state()->canRestoreClip = false;
        break;
    case Qt::IntersectClip:
        ++state()->maxDepth;
        d->writeClip(path, state()->maxDepth);
        state()->currentDepth = state()->maxDepth - 1;
        state()->depthTestEnabled = true;
        break;
    case Qt::ReplaceClip:
        d->systemStateChanged();
        state()->rectangleClip = QRect();
        state()->maxDepth = 4;
        glDepthFunc(GL_ALWAYS);
        d->writeClip(path, state()->maxDepth);
        state()->currentDepth = 3;
        state()->canRestoreClip = false;
        state()->depthTestEnabled = true;
        break;
    case Qt::UniteClip:
        glDepthFunc(GL_ALWAYS);
        d->writeClip(path, state()->maxDepth);
        state()->canRestoreClip = false;
        state()->depthTestEnabled = true;
        break;
    }

    glDepthFunc(GL_LESS);
    if (state()->depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
        d->simpleShaderDepthUniformDirty = true;
        d->depthUniformDirty = true;
    }
}

void QGL2PaintEngineExPrivate::regenerateDepthClip()
{
    systemStateChanged();
    replayClipOperations();
}

void QGL2PaintEngineExPrivate::systemStateChanged()
{
    Q_Q(QGL2PaintEngineEx);

    if (systemClip.isEmpty()) {
        use_system_clip = false;
    } else {
        if (q->paintDevice()->devType() == QInternal::Widget && currentClipWidget) {
            QWidgetPrivate *widgetPrivate = qt_widget_private(currentClipWidget->window());
            use_system_clip = widgetPrivate->extra && widgetPrivate->extra->inRenderWithPainter;
        } else {
            use_system_clip = true;
        }
    }

    glDisable(GL_DEPTH_TEST);
    q->state()->depthTestEnabled = false;
    q->state()->scissorTestEnabled = false;
    q->state()->needsDepthBufferClear = true;
    q->state()->hasRectangleClip = false;

    glDisable(GL_SCISSOR_TEST);

    q->state()->currentDepth = 1;
    q->state()->maxDepth = 4;

    q->state()->rectangleClip = QRect(0, 0, width, height);

    if (use_system_clip) {
        if (systemClip.numRects() == 1) {
            QRect bounds = systemClip.boundingRect();
            if (bounds == QRect(0, 0, width, height)) {
                use_system_clip = false;
                return;
            }

            q->state()->rectangleClip = bounds;
            q->state()->scissorTestEnabled = true;
            updateDepthScissorTest();
        } else {
            q->state()->rectangleClip = QRect();
            q->state()->scissorTestEnabled = true;
            updateDepthScissorTest();

            QTransform transform = q->state()->matrix;
            q->state()->matrix = QTransform();
            q->transformChanged();

            q->state()->needsDepthBufferClear = false;

            glDepthMask(true);

            glClearDepth(0);
            glClear(GL_DEPTH_BUFFER_BIT);

            QPainterPath path;
            path.addRegion(systemClip);

            glDepthFunc(GL_ALWAYS);
            writeClip(qtVectorPathForPath(path), 2);
            glDepthFunc(GL_LESS);

            glEnable(GL_DEPTH_TEST);
            q->state()->depthTestEnabled = true;

            q->state()->matrix = transform;
            q->transformChanged();
        }

        q->state()->currentDepth = 1;
        simpleShaderDepthUniformDirty = true;
        depthUniformDirty = true;
    }
}

void QGL2PaintEngineEx::setState(QPainterState *new_state)
{
    //     qDebug("QGL2PaintEngineEx::setState()");

    Q_D(QGL2PaintEngineEx);

    QOpenGL2PaintEngineState *s = static_cast<QOpenGL2PaintEngineState *>(new_state);
    QOpenGL2PaintEngineState *old_state = state();

    QPaintEngineEx::setState(s);

    if (s == d->last_created_state) {
        d->last_created_state = 0;
        return;
    }

    renderHintsChanged();

    d->matrixDirty = true;
    d->compositionModeDirty = true;
    d->simpleShaderDepthUniformDirty = true;
    d->depthUniformDirty = true;
    d->simpleShaderMatrixUniformDirty = true;
    d->shaderMatrixUniformDirty = true;
    d->opacityUniformDirty = true;

    d->shaderManager->setDirty();

    if (old_state && old_state != s && old_state->canRestoreClip) {
        d->updateDepthScissorTest();
        glDepthMask(false);
        glDepthFunc(GL_LESS);
        s->maxDepth = old_state->maxDepth;
    } else {
        d->regenerateDepthClip();
    }
}

QPainterState *QGL2PaintEngineEx::createState(QPainterState *orig) const
{
    Q_D(const QGL2PaintEngineEx);

    if (orig)
        const_cast<QGL2PaintEngineEx *>(this)->ensureActive();

    QOpenGL2PaintEngineState *s;
    if (!orig)
        s = new QOpenGL2PaintEngineState();
    else
        s = new QOpenGL2PaintEngineState(*static_cast<QOpenGL2PaintEngineState *>(orig));

    d->last_created_state = s;
    return s;
}

void QGL2PaintEngineEx::setRenderTextActive(bool active)
{
    Q_D(QGL2PaintEngineEx);
    d->inRenderText = active;
}

QOpenGL2PaintEngineState::QOpenGL2PaintEngineState(QOpenGL2PaintEngineState &other)
    : QPainterState(other)
{
    needsDepthBufferClear = other.needsDepthBufferClear;
    depthTestEnabled = other.depthTestEnabled;
    scissorTestEnabled = other.scissorTestEnabled;
    currentDepth = other.currentDepth;
    maxDepth = other.maxDepth;
    canRestoreClip = other.canRestoreClip;
    rectangleClip = other.rectangleClip;
    hasRectangleClip = other.hasRectangleClip;
}

QOpenGL2PaintEngineState::QOpenGL2PaintEngineState()
{
    needsDepthBufferClear = true;
    depthTestEnabled = false;
    scissorTestEnabled = false;
    currentDepth = 1;
    maxDepth = 4;
    canRestoreClip = true;
    hasRectangleClip = false;
}

QOpenGL2PaintEngineState::~QOpenGL2PaintEngineState()
{
}

QT_END_NAMESPACE

#include "qpaintengineex_opengl2.moc"
