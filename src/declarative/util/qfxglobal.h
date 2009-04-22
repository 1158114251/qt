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

#ifndef QFXGLOBAL_H
#define QFXGLOBAL_H

#include <qglobal.h>
#include <QObject>


QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)
#if defined(QT_OPENGL_ES_1)
#define QFX_CONFIGURATION_OPENGL1
#elif defined(QT_OPENGL_ES_2)
#define QFX_CONFIGURATION_OPENGL2
#else
#define QFX_CONFIGURATION_SOFTWARE
#endif

/*
    The choices of renderer are:
        QFX_RENDER_QPAINTER
        QFX_RENDER_OPENGL1
        QFX_RENDER_OPENGL2
    To simplify code, if either of the OpenGL renderers are used, 
    QFX_RENDER_OPENGL is also defined.
*/
    
#if defined(QFX_CONFIGURATION_OPENGL2)

#define QFX_RENDER_OPENGL
#define QFX_RENDER_OPENGL2

#elif defined(QFX_CONFIGURATION_OPENGL1)

#define QFX_RENDER_OPENGL
#define QFX_RENDER_OPENGL1

#elif defined(QFX_CONFIGURATION_SOFTWARE)

#define QFX_RENDER_QPAINTER

#endif

#define DEFINE_BOOL_CONFIG_OPTION(name, var) \
    static bool name() \
    { \
        static enum { Yes, No, Unknown } status = Unknown; \
        if(status == Unknown) { \
            QByteArray v = qgetenv(#var); \
            bool value = !v.isEmpty() && v != "0" && v != "false"; \
            if(value) status = Yes; \
            else status = No; \
        } \
        return status == Yes; \
    }

struct QFx_DerivedObject : public QObject
{
    void setParent_noEvent(QObject *parent) {
        bool sce = d_ptr->sendChildEvents;
        d_ptr->sendChildEvents = false;
        setParent(parent);
        d_ptr->sendChildEvents = sce;
    }
};

/*!
    Makes the \a object a child of \a parent.  Note that when using this method,
    neither \a parent nor the object's previous parent (if it had one) will
    receive ChildRemoved or ChildAdded events.
*/
inline void QFx_setParent_noEvent(QObject *object, QObject *parent)
{
    static_cast<QFx_DerivedObject *>(object)->setParent_noEvent(parent);
}


QT_END_NAMESPACE

QT_END_HEADER
#endif // QFXGLOBAL_H
