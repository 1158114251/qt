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

#include <private/qglpaintdevice_qws_p.h>
#include <private/qgl_p.h>
#include <private/qpaintengine_opengl_p.h>
#include <private/qglwindowsurface_qws_p.h>

QT_BEGIN_NAMESPACE

class QWSGLPaintDevicePrivate
{
public:
    QWidget *widget;
};

QWSGLPaintDevice::QWSGLPaintDevice(QWidget *widget) :
    d_ptr(new QWSGLPaintDevicePrivate)
{
    Q_D(QWSGLPaintDevice);
    d->widget = widget;
}

QWSGLPaintDevice::~QWSGLPaintDevice()
{
}

QPaintEngine* QWSGLPaintDevice::paintEngine() const
{
    return qt_qgl_paint_engine();
}

int QWSGLPaintDevice::metric(PaintDeviceMetric m) const
{
    Q_D(const QWSGLPaintDevice);
    Q_ASSERT(d->widget);

    return qt_paint_device_metric(d->widget, m);
}

QWSGLWindowSurface* QWSGLPaintDevice::windowSurface() const
{
     Q_D(const QWSGLPaintDevice);
     return static_cast<QWSGLWindowSurface*>(d->widget->windowSurface());
}

QT_END_NAMESPACE
