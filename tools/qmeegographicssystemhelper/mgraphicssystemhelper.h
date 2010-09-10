/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/

#ifndef MGRAPHICSSYSTEMHELPER_H
#define MGRAPHICSSYSTEMHELPER_H

#include <QPixmap>
#include <QImage>
#include "mlivepixmap.h"

class QLibrary;

//! The base class for accressing special meego graphics system features. 
/*!
 This class is a helper class with static-only methods for accessing various
 meego graphics system functionalities. The way it works is that the helper
 dynamically calls-in to the loaded graphicssystem plugin... therefore, you're 
 expected to make sure that you're indeed running with 'meego' before using any
 of the specialized methods. 

 In example:

 \code
 QPixmap p;
 if (MGraphicsSystemHelper::isRunningMeeGo()) {
    p = MGraphicsSystemHelper::pixmapWithGLTexture(64, 64);
 } else {
    p = QPixmap(64, 64);
 }
 \endcode

 Calling any of the meego-specific features while not running meego might
 give unpredictable results. The only functions safe to call at all times are:

 \code
 MGraphicsSystemHelper::isRunningMeeGo();
 MGraphicsSystemHelper::runningGraphicsSystemName(); 
 MGraphicsSystemHelper::switchToMeeGo();
 MGraphicsSystemHelper::switchToRaster();
 \endcode
*/

class MGraphicsSystemHelper 
{
public:
    //! Returns true if running meego.
    /*! 
     Returns true if the currently active (running) system is 'meego' with OpenGL. 
     This returns both true if the app was started with 'meego' or was started with
     'runtime' graphics system and the currently active system through the runtime
     switching is 'meego'.
    */
    static bool isRunningMeeGo();

    //! Switches to meego graphics system. 
    /*!
     When running with the 'runtime' graphics system, sets the currently active 
     system to 'meego'. The window surface and all the resources are automatically
     migrated to OpenGL. Will fail if the active graphics system is not 'runtime'.
    */
    static void switchToMeeGo();

    //! Switches to raster graphics system
    /*!
     When running with the 'runtime' graphics system, sets the currently active
     system to 'raster'. The window surface and the graphics resources (including the 
     EGL shared image resources) are automatically migrated back to the CPU. All OpenGL 
     resources (surface, context, cache, font cache) are automaticall anihilated.
    */
    static void switchToRaster();

    //! Returns the name of the active graphics system
    /*!
     Returns the name of the currently active system. If running with 'runtime' graphics 
     system, returns the name of the active system inside the runtime graphics system
    */
    static QString runningGraphicsSystemName();

    //! Creates a new EGL shared image.
    /*! 
     Creates a new EGL shared image from the given image. The EGL shared image wraps
     a GL texture in the native format and can be easily accessed from other processes.
    */
    static Qt::HANDLE imageToEGLSharedImage(const QImage &image);

    //! Creates a QPixmap from an EGL shared image
    /*!
     Creates a new QPixmap from the given EGL shared image handle. The QPixmap can be 
     used for painting like any other pixmap. The softImage should point to an alternative, 
     software version of the graphical resource -- ie. obtained from theme daemon. The 
     softImage can be allocated on a QSharedMemory slice for easy sharing across processes
     too. When the application is migrated ToRaster, this softImage will replace the
     contents of the sharedImage.
     
     It's ok to call this function too when not running 'meego' graphics system. In this
     case it'll create a QPixmap backed with a raster data (from softImage)... but when 
     the system is switched back to 'meego', the QPixmap will be migrated to a EGL-shared image
     backed storage (handle).
    */
    static QPixmap pixmapFromEGLSharedImage(Qt::HANDLE handle, const QImage &softImage);

    //! Destroys an EGL shared image. 
    /*!
     Destroys an EGLSharedImage previously created with an ::imageToEGLSharedImage call.
     Returns true if the image was found and the destruction was successfull. Notice that
     this destroys the image for all processes using it. 
    */
    static bool destroyEGLSharedImage(Qt::HANDLE handle);
    
    //! Updates the QPixmap backed with an EGLShared image. 
    /*! 
     This function re-reads the softImage that was specified when creating the pixmap with
     ::pixmapFromEGLSharedImage and updates the EGL Shared image contents. It can be used
     to share cross-proccess mutable EGLShared images. 
    */
    static void updateEGLSharedImagePixmap(QPixmap *p);

    //! Create a new QPixmap with a GL texture.
    /*!
     Creates a new QPixmap which is backed by an OpenGL local texture. Drawing to this
     QPixmap will be accelerated by hardware -- unlike the normal (new QPixmap()) pixmaps, 
     which are backed by a software engine and only migrated to GPU when used. Migrating those
     GL-backed pixmaps when going ToRaster is expsensive (they need to be downloaded from 
     GPU to CPU) so use wisely. 
    */
    static QPixmap pixmapWithGLTexture(int w, int h);
    
    //! Sets translucency (alpha) on the base window surface.
    /*!
     This function needs to be called *before* any widget/content is created. 
     When called with true, the base window surface will be translucent and initialized
     with QGLFormat.alpha == true.
    */
    static void setTranslucent(bool translucent);
};

#endif
