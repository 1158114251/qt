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
** contact the sales department at http://qt.nokia.com/contact.
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QGL_P_H
#define QGL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QGLWidget class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include "QtOpenGL/qgl.h"
#include "QtOpenGL/qglcolormap.h"
#include "QtCore/qmap.h"
#include "QtCore/qthread.h"
#include "QtCore/qthreadstorage.h"
#include "QtCore/qhash.h"
#include "private/qwidget_p.h"
#include "qcache.h"

#ifndef QT_OPENGL_ES_1_CL
#define q_vertexType float
#define q_vertexTypeEnum GL_FLOAT
#define f2vt(f)     (f)
#define vt2f(x)     (x)
#define i2vt(i)     (float(i))
#else
#define FLOAT2X(f)      (int( (f) * (65536)))
#define X2FLOAT(x)      (float(x) / 65536.0f)
#define f2vt(f)     FLOAT2X(f)
#define i2vt(i)     ((i)*65536)
#define vt2f(x)     X2FLOAT(x)
#define q_vertexType GLfixed
#define q_vertexTypeEnum GL_FIXED
#endif //QT_OPENGL_ES_1_CL

#ifdef QT_OPENGL_ES
QT_BEGIN_INCLUDE_NAMESPACE
#if defined(QT_OPENGL_ES_2)
#include <EGL/egl.h>
#else
#include <GLES/egl.h>
#endif
QT_END_INCLUDE_NAMESPACE
#endif

QT_BEGIN_NAMESPACE

class QGLContext;
class QGLOverlayWidget;
class QPixmap;
class QPixmapFilter;
#ifdef Q_WS_MAC
# ifdef qDebug
#   define old_qDebug qDebug
#   undef qDebug
# endif
QT_BEGIN_INCLUDE_NAMESPACE
#ifndef QT_MAC_USE_COCOA
# include <AGL/agl.h>
#endif
QT_END_INCLUDE_NAMESPACE
# ifdef old_qDebug
#   undef qDebug
#   define qDebug QT_QDEBUG_MACRO
#   undef old_qDebug
# endif
class QMacWindowChangeEvent;
#endif

#ifdef Q_WS_QWS
class QWSGLWindowSurface;
#endif

#if defined(QT_OPENGL_ES)
class QEglContext;
#endif

QT_BEGIN_INCLUDE_NAMESPACE
#include <QtOpenGL/private/qglextensions_p.h>
QT_END_INCLUDE_NAMESPACE

class QGLFormatPrivate
{
public:
    QGLFormatPrivate() {
        opts = QGL::DoubleBuffer | QGL::DepthBuffer | QGL::Rgba | QGL::DirectRendering | QGL::StencilBuffer;
#if defined(QT_OPENGL_ES_2)
        opts |= QGL::SampleBuffers;
#endif
        pln = 0;
        depthSize = accumSize = stencilSize = redSize = greenSize = blueSize = alphaSize = -1;
        numSamples = -1;
        swapInterval = -1;
    }
    QGL::FormatOptions opts;
    int pln;
    int depthSize;
    int accumSize;
    int stencilSize;
    int redSize;
    int greenSize;
    int blueSize;
    int alphaSize;
    int numSamples;
    int swapInterval;
};

class QGLWidgetPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QGLWidget)
public:
    QGLWidgetPrivate() : QWidgetPrivate()
#ifdef Q_WS_QWS
                       , wsurf(0)
#endif
#if defined(Q_WS_X11) && defined(QT_OPENGL_ES)
                       , eglSurfaceWindowId(0)
#endif
        {}

    ~QGLWidgetPrivate() {}

    void init(QGLContext *context, const QGLWidget* shareWidget);
    void initContext(QGLContext *context, const QGLWidget* shareWidget);
    bool renderCxPm(QPixmap *pixmap);
    void cleanupColormaps();

    QGLContext *glcx;
    bool autoSwap;

    QGLColormap cmap;
    QMap<QString, int> displayListCache;

#if defined(Q_WS_WIN)
    void updateColormap();
    QGLContext *olcx;
#elif defined(Q_WS_X11)
    QGLOverlayWidget *olw;
#if defined(QT_OPENGL_ES)
    void recreateEglSurface(bool force);
    WId eglSurfaceWindowId;
#endif
#elif defined(Q_WS_MAC)
    QGLContext *olcx;
    void updatePaintDevice();
#elif defined(Q_WS_QWS)
    QWSGLWindowSurface *wsurf;
#endif
};

struct QGLContextGroupResources
{
    QGLContextGroupResources() : refs(1) { }
    QGLExtensionFuncs extensionFuncs;
    QAtomicInt refs;
};

class QGLTexture;

class QGLContextPrivate
{
    Q_DECLARE_PUBLIC(QGLContext)
public:
    explicit QGLContextPrivate(QGLContext *context) : internal_context(false), q_ptr(context) {groupResources = new QGLContextGroupResources;}
    ~QGLContextPrivate() {if (!groupResources->refs.deref()) delete groupResources;}
    QGLTexture *bindTexture(const QImage &image, GLenum target, GLint format, bool clean);
    QGLTexture *bindTexture(const QImage &image, GLenum target, GLint format, const qint64 key,
                       bool clean = false);
    QGLTexture *bindTexture(const QPixmap &pixmap, GLenum target, GLint format, bool clean, bool canInvert = false);
    QGLTexture *textureCacheLookup(const qint64 key, GLenum target);
    void init(QPaintDevice *dev, const QGLFormat &format);
    QImage convertToGLFormat(const QImage &image, bool force_premul, GLenum texture_format);
    int maxTextureSize();

    void cleanup();

#if defined(Q_WS_WIN)
    HGLRC rc;
    HDC dc;
    WId        win;
    int pixelFormatId;
    QGLCmap* cmap;
    HBITMAP hbitmap;
    HDC hbitmap_hdc;
#endif
#if defined(QT_OPENGL_ES)
    QEglContext *eglContext;
#elif defined(Q_WS_X11) || defined(Q_WS_MAC)
    void* cx;
#endif
#if defined(Q_WS_X11) || defined(Q_WS_MAC)
    void* vi;
#endif
#if defined(Q_WS_X11)
    void* pbuf;
    quint32 gpm;
    int screen;
    QHash<QPixmapData*, QPixmap> boundPixmaps;
    QGLTexture *bindTextureFromNativePixmap(QPixmapData*, const qint64 key, bool canInvert);
    static void destroyGlSurfaceForPixmap(QPixmapData*);
    static void unbindPixmapFromTexture(QPixmapData*);
#endif
#if defined(Q_WS_MAC)
    bool update;
    void *tryFormat(const QGLFormat &format);
    void clearDrawable();
#endif
    QGLFormat glFormat;
    QGLFormat reqFormat;
    GLuint pbo;
    GLuint fbo;

    uint valid : 1;
    uint sharing : 1;
    uint initDone : 1;
    uint crWin : 1;
    uint clear_on_painter_begin : 1;
    uint internal_context : 1;
    uint version_flags_cached : 1;
    QPaintDevice *paintDevice;
    QColor transpColor;
    QGLContext *q_ptr;
    QGLFormat::OpenGLVersionFlags version_flags;

    QGLContextGroupResources *groupResources;
    GLint max_texture_size;

    GLuint current_fbo;
    QPaintEngine *active_engine;

#ifdef Q_WS_WIN
    static inline QGLExtensionFuncs& qt_get_extension_funcs(const QGLContext *ctx) { return ctx->d_ptr->groupResources->extensionFuncs; }
#endif

#if defined(Q_WS_X11) || defined(Q_WS_MAC) || defined(Q_WS_QWS)
    static QGLExtensionFuncs qt_extensionFuncs;
    static inline QGLExtensionFuncs& qt_get_extension_funcs(const QGLContext *) { return qt_extensionFuncs; }
#endif

    QPixmapFilter *createPixmapFilter(int type) const;
};

// ### make QGLContext a QObject in 5.0 and remove the proxy stuff
class Q_OPENGL_EXPORT QGLSignalProxy : public QObject
{
    Q_OBJECT
public:
    QGLSignalProxy() : QObject() {}
    void emitAboutToDestroyContext(const QGLContext *context) {
        emit aboutToDestroyContext(context);
    }
    static QGLSignalProxy *instance();
Q_SIGNALS:
    void aboutToDestroyContext(const QGLContext *context);
};

class QGLPixelBuffer;
class QGLFramebufferObject;
class QWSGLWindowSurface;
class QGLWindowSurface;
class QGLPixmapData;
class QGLDrawable {
public:
    QGLDrawable() : widget(0), buffer(0), fbo(0)
#if defined(Q_WS_QWS) || (!defined(QT_OPENGL_ES_1) && !defined(QT_OPENGL_ES_1_CL))
                  , wsurf(0)
#endif
#if !defined(QT_OPENGL_ES_1) && !defined(QT_OPENGL_ES_1_CL)
                  , pixmapData(0)
#endif
        {}
    void setDevice(QPaintDevice *pdev);
    void swapBuffers();
    void makeCurrent();
    void doneCurrent();
    QSize size() const;
    QGLFormat format() const;
    GLuint bindTexture(const QImage &image, GLenum target = GL_TEXTURE_2D, GLint format = GL_RGBA);
    GLuint bindTexture(const QPixmap &pixmap, GLenum target = GL_TEXTURE_2D, GLint format = GL_RGBA);
    QColor backgroundColor() const;
    QGLContext *context() const;
    bool autoFillBackground() const;
    bool hasTransparentBackground() const;

#if !defined(QT_OPENGL_ES_1) && !defined(QT_OPENGL_ES_1_CL)
    QGLPixmapData *copyOnBegin() const;
#endif

private:
    bool wasBound;
    QGLWidget *widget;
    QGLPixelBuffer *buffer;
    QGLFramebufferObject *fbo;
#ifdef Q_WS_QWS
    QWSGLWindowSurface *wsurf;
#elif !defined(QT_OPENGL_ES_1) && !defined(QT_OPENGL_ES_1_CL)
    QGLWindowSurface *wsurf;
#endif

#if !defined(QT_OPENGL_ES_1) && !defined(QT_OPENGL_ES_1_CL)
    QGLPixmapData *pixmapData;
#endif
    int previous_fbo;
};

// GL extension definitions
class QGLExtensions {
public:
    enum Extension {
        TextureRectangle        = 0x00000001,
        SampleBuffers           = 0x00000002,
        GenerateMipmap          = 0x00000004,
        TextureCompression      = 0x00000008,
        FragmentProgram         = 0x00000010,
        MirroredRepeat          = 0x00000020,
        FramebufferObject       = 0x00000040,
        StencilTwoSide          = 0x00000080,
        StencilWrap             = 0x00000100,
        PackedDepthStencil      = 0x00000200,
        NVFloatBuffer           = 0x00000400,
        PixelBufferObject       = 0x00000800,
        FramebufferBlit         = 0x00001000,
        NPOTTextures            = 0x00002000
    };
    Q_DECLARE_FLAGS(Extensions, Extension)

    static Extensions glExtensions;
    static bool nvidiaFboNeedsFinish;
    static void init(); // sys dependent
    static void init_extensions(); // general: called by init()
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QGLExtensions::Extensions)


struct QGLThreadContext {
    QGLContext *context;
};
extern QThreadStorage<QGLThreadContext *> qgl_context_storage;

class QGLShareRegister
{
public:
    QGLShareRegister() {}
    ~QGLShareRegister() { reg.clear(); }

    bool checkSharing(const QGLContext *context1, const QGLContext *context2);
    void addShare(const QGLContext *context, const QGLContext *share);
    QList<const QGLContext *> shares(const QGLContext *context);
    void removeShare(const QGLContext *context);
private:
    // Use a context's 'groupResources' pointer to uniquely identify a group.
    typedef QList<const QGLContext *> ContextList;
    typedef QHash<const QGLContextGroupResources *, ContextList> SharingHash;
    SharingHash reg;
};

extern Q_OPENGL_EXPORT QGLShareRegister* qgl_share_reg();

class QGLTexture {
public:
    QGLTexture(QGLContext *ctx = 0, GLuint tx_id = 0, GLenum tx_target = GL_TEXTURE_2D,
               bool _clean = false, bool _yInverted = false)
        : context(ctx), id(tx_id), target(tx_target), clean(_clean), yInverted(_yInverted)
#if defined(Q_WS_X11)
            , boundPixmap(0)
#endif
    {}

    ~QGLTexture() {
        if (clean) {
            QGLContext *current = const_cast<QGLContext *>(QGLContext::currentContext());
            QGLContext *ctx = const_cast<QGLContext *>(context);
            Q_ASSERT(ctx);
            bool switch_context = current != ctx && !qgl_share_reg()->checkSharing(current, ctx);
            if (switch_context)
                ctx->makeCurrent();
#if defined(Q_WS_X11)
            // Although glXReleaseTexImage is a glX call, it must be called while there
            // is a current context - the context the pixmap was bound to a texture in.
            // Otherwise the release doesn't do anything and you get BadDrawable errors
            // when you come to delete the context.
            if (boundPixmap)
                QGLContextPrivate::unbindPixmapFromTexture(boundPixmap);
#endif
            glDeleteTextures(1, &id);
            if (switch_context && current)
                current->makeCurrent();
        }
     }

    QGLContext *context;
    GLuint id;
    GLenum target;
    bool clean;
    bool yInverted; // NOTE: Y-Inverted textures are for internal use only!
#if defined(Q_WS_X11)
    QPixmapData* boundPixmap;
#endif

};

class QGLTextureCache {
public:
    QGLTextureCache();
    ~QGLTextureCache();

    void insert(QGLContext *ctx, qint64 key, QGLTexture *texture, int cost);
    void remove(quint64 key) { m_cache.remove(key); }
    bool remove(QGLContext *ctx, GLuint textureId);
    void removeContextTextures(QGLContext *ctx);
    int size() { return m_cache.size(); }
    void setMaxCost(int newMax) { m_cache.setMaxCost(newMax); }
    int maxCost() {return m_cache.maxCost(); }
    QGLTexture* getTexture(quint64 key) { return m_cache.object(key); }

    static QGLTextureCache *instance();
    static void deleteIfEmpty();
    static void imageCleanupHook(qint64 cacheKey);
    static void pixmapCleanupHook(QPixmap* pixmap);

private:
    QCache<qint64, QGLTexture> m_cache;
};


#ifdef Q_WS_QWS
extern QPaintEngine* qt_qgl_paint_engine();

extern EGLDisplay qt_qgl_egl_display();
#endif

inline bool qt_gl_preferGL2Engine()
{
#if defined(QT_OPENGL_ES_2)
    return true;
#else
    return (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_2_0)
           && qgetenv("QT_GL_USE_OPENGL1ENGINE").isEmpty();
#endif
}

inline GLenum qt_gl_preferredTextureFormat()
{
    return QSysInfo::ByteOrder == QSysInfo::BigEndian ? GL_RGBA : GL_BGRA;
}

inline GLenum qt_gl_preferredTextureTarget()
{
#if defined(QT_OPENGL_ES_2)
    return GL_TEXTURE_2D;
#else
    return (QGLExtensions::glExtensions & QGLExtensions::TextureRectangle)
           && !qt_gl_preferGL2Engine()
           ? GL_TEXTURE_RECTANGLE_NV
           : GL_TEXTURE_2D;
#endif
}

// One resource per group of shared contexts.
class QGLContextResource : public QObject
{
    Q_OBJECT
public:
    typedef void (*FreeFunc)(void *);
    QGLContextResource(FreeFunc f, QObject *parent = 0);
    ~QGLContextResource();
    // Set resource 'value' for 'key' and all its shared contexts.
    void insert(const QGLContext *key, void *value);
    // Return resource for 'key' or a shared context.
    void *value(const QGLContext *key);
    // Free resource for 'key' and all its shared contexts.
    void remove(const QGLContext *key);
private slots:
    // Remove entry 'key' from cache and delete resource if there are no shared contexts.
    void aboutToDestroyContext(const QGLContext *key);
private:
    typedef QHash<const QGLContext *, void *> ResourceHash;
    ResourceHash m_resources;
    FreeFunc free;
};

QT_END_NAMESPACE

#endif // QGL_P_H
