/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
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

#include "qgraphicssystem_minimaldfb.h"
#include "qwindowsurface_minimaldfb.h"
#include <QtGui/private/qpixmap_raster_p.h>
#include <QCoreApplication>
#include <directfb.h>

QT_BEGIN_NAMESPACE

QDirectFbGraphicsSystemScreen::QDirectFbGraphicsSystemScreen(IDirectFB *dfb, int display)
{
    DFBResult result  = dfb->GetDisplayLayer(dfb, DLID_PRIMARY, &m_layer);
    if (result != DFB_OK) {
        DirectFBError("QDirectFbGraphicsSystemScreen::connect: "
                      "Unable to get primary display layer!", result);
    }

    IDirectFBSurface *topLevelSurface;
    m_layer->GetSurface(m_layer, &topLevelSurface);
    m_format = QDirectFbGraphicsSystem::imageFormatFromSurface(topLevelSurface);

    result = m_layer->GetScreen(m_layer,&m_screen);
    if (result != DFB_OK) {
        DirectFBError("QDirectFbGraphicsSystemScreen: Failed to get screen", result);
    }

    int w(0),h(0);
    m_screen->GetSize(m_screen,&w,&h);
    m_geometry = QRect(0,0,w,h);
    const int dpi = 72;
    const qreal inch = 25.4;
    m_depth = 32;
    m_physicalSize = QSize(qRound(w * inch / dpi), qRound(h *inch / dpi));
}

QDirectFbGraphicsSystemScreen::~QDirectFbGraphicsSystemScreen()
{
}

IDirectFBWindow *QDirectFbGraphicsSystemScreen::createWindow(const QRect &rect)
{
    IDirectFBWindow *window;

    DFBWindowDescription description;
    memset(&description,0,sizeof(DFBWindowDescription));
//    description.flags = DWDESC_SURFACE_CAPS;
    description.width = rect.width();
    description.height = rect.height();
    description.posx = rect.x();
    description.posy = rect.y();
//    description.flags |= DWDESC_OPTIONS;
//    description.options = DWOP_GHOST|DWOP_ALPHACHANNEL;
//    description.caps = (DFBWindowDescription) (DWCAPS_NODECORATION|DWCAPS_DOUBLEBUFFER);
//    description.surface_caps = DSCAPS_PREMULTIPLIED;

    DFBResult result = m_layer->CreateWindow(m_layer,&description,&window);
    if (result != DFB_OK) {
        DirectFBError("QDirectFbGraphicsSystemScreen: failed to create window",result);
    }
    return window;
}

QDirectFbGraphicsSystem::QDirectFbGraphicsSystem()
{
    DFBResult result = DFB_OK;

    {   // pass command line arguments to DirectFB
        const QStringList args = QCoreApplication::arguments();
        int argc = args.size();
        char **argv = new char*[argc];

        for (int i = 0; i < argc; ++i)
            argv[i] = qstrdup(args.at(i).toLocal8Bit().constData());

        result = DirectFBInit(&argc, &argv);
        if (result != DFB_OK) {
            DirectFBError("QDirectFBScreen: error initializing DirectFB",
                          result);
        }
        delete[] argv;
    }

    result = DirectFBCreate(&dfb);
    if (result != DFB_OK) {
        DirectFBError("QDirectFBScreen: error creating DirectFB interface",
                      result);
    }

    mPrimaryScreen = new QDirectFbGraphicsSystemScreen(dfb,0);
    mScreens.append(mPrimaryScreen);
}

QPixmapData *QDirectFbGraphicsSystem::createPixmapData(QPixmapData::PixelType type) const
{
    return new QRasterPixmapData(type);
}

QWindowSurface *QDirectFbGraphicsSystem::createWindowSurface(QWidget *widget) const
{
    return new QDirectFbWindowSurface (mPrimaryScreen, widget);
}

QImage::Format QDirectFbGraphicsSystem::imageFormatFromSurface(IDirectFBSurface *surface)
{
    DFBSurfacePixelFormat format;
    surface->GetPixelFormat(surface, &format);

    switch (format) {
    case DSPF_LUT8:
        return QImage::Format_Indexed8;
    case DSPF_RGB24:
        return QImage::Format_RGB888;
    case DSPF_ARGB4444:
        return QImage::Format_ARGB4444_Premultiplied;
    case DSPF_RGB444:
        return QImage::Format_RGB444;
    case DSPF_RGB555:
    case DSPF_ARGB1555:
        return QImage::Format_RGB555;
    case DSPF_RGB16:
        return QImage::Format_RGB16;
    case DSPF_ARGB6666:
        return QImage::Format_ARGB6666_Premultiplied;
    case DSPF_RGB18:
        return QImage::Format_RGB666;
    case DSPF_RGB32:
        return QImage::Format_RGB32;
    case DSPF_ARGB: {
            DFBSurfaceCapabilities caps;
            const DFBResult result = surface->GetCapabilities(surface, &caps);
            Q_ASSERT(result == DFB_OK);
            Q_UNUSED(result);
            return (caps & DSCAPS_PREMULTIPLIED
                    ? QImage::Format_ARGB32_Premultiplied
                        : QImage::Format_ARGB32); }
    default:
        break;
    }
    return QImage::Format_Invalid;

}

QT_END_NAMESPACE
