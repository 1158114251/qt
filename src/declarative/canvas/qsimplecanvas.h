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

#ifndef QSIMPLECANVAS_H
#define QSIMPLECANVAS_H

#include <qfxglobal.h>

#ifdef QFX_RENDER_OPENGL
#include <QtGui/qmatrix4x4.h>
#endif

#include <QTransform>
#include <QPainter>
#include <QDebug>
#include <QWidget>
#include <QImage>
#include <QKeyEvent>


QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)
namespace QSimpleCanvasConfig
{
    enum ImageType { Opaque, Translucent };

#ifdef QFX_RENDER_OPENGL
    typedef QMatrix4x4 Matrix;
    typedef QImage Image;

    inline Matrix transformToMatrix(const QTransform &)
    { 
        return Matrix(); // XXX 
    }
    inline QTransform matrixToTransform(const Matrix &)
    { 
        return QTransform(); // XXX 
    }
    inline bool needConvert(ImageType, const Image &) 
    { return false; }
    inline Image convert(ImageType, const Image &i)
    { return i; }
    inline Image create(const QSize &s) 
    { return QImage(s, QImage::Format_ARGB32); }
    inline const Image &toImage(const QImage &i)
    { return i; }

#elif defined(QFX_RENDER_QPAINTER)
    typedef QTransform Matrix;
    typedef QImage Image;

    inline Matrix transformToMatrix(const QTransform &t)
    { return t; }
    inline QTransform matrixToTransform(const Matrix &t)
    { return t; }
    inline bool needConvert(ImageType type, const Image &img) {
        QImage::Format f = img.format();
        return !((type == Opaque && f == QImage::Format_RGB16) ||
                 (type == Translucent && f == QImage::Format_ARGB32_Premultiplied));
    }
    inline Image convert(ImageType type, const Image &img) {
        if (type == Opaque)
            return img.convertToFormat(QImage::Format_RGB16);
        else
            return img.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    }
    inline Image create(const QSize &s) 
    { return QImage(s, QImage::Format_ARGB32_Premultiplied); }
    inline const Image &toImage(const QImage &i)
    { return i; }
#endif
}

class QSimpleCanvas;
class QSimpleCanvasLayer;

class QGraphicsSceneMouseEvent;
class GLBasicShaders;
class QSimpleCanvasItem;
class QSimpleCanvasPrivate;
class Q_DECLARATIVE_EXPORT QSimpleCanvas : public QWidget
{
Q_OBJECT
public:
    typedef QSimpleCanvasConfig::Matrix Matrix;

    enum CanvasMode { GraphicsView, SimpleCanvas };

    QSimpleCanvas(QWidget *parent = 0);
    QSimpleCanvas(CanvasMode, QWidget *parent = 0);
    virtual ~QSimpleCanvas();

    CanvasMode canvasMode() const;

    QSimpleCanvasItem *root();

    // Debugging
    void dumpTiming();
    void dumpItems();
    void checkState();

    QSimpleCanvasItem *focusItem() const;
    QSimpleCanvasItem *activeFocusPanel() const;
    QImage asImage() const;

Q_SIGNALS:
    void framePainted();

protected:
    virtual bool event(QEvent *);
    virtual void paintEvent(QPaintEvent *);
    virtual void keyPressEvent(QKeyEvent *);
    virtual void keyReleaseEvent(QKeyEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void focusInEvent(QFocusEvent *event);
    virtual void focusOutEvent(QFocusEvent *event);
    virtual bool focusNextPrevChild(bool next);
    virtual bool eventFilter(QObject *obj, QEvent *event);
    virtual void inputMethodEvent(QInputMethodEvent *event);
    virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const;

    virtual void showEvent(QShowEvent *);
    virtual void resizeEvent(QResizeEvent *);
private:

    friend class QSimpleCanvasRootLayer;
    friend class QSimpleCanvasPrivate;
    friend class QSimpleCanvasItem;
    friend class QSimpleCanvasItemPrivate;
    friend class QSimpleCanvasFilter;
    friend class QSimpleGraphicsItem;

    void queueUpdate();
    QSimpleCanvasPrivate *d;
    void addDirty(QSimpleCanvasItem *);
    void remDirty(QSimpleCanvasItem *);
};


QT_END_NAMESPACE

QT_END_HEADER
#endif
