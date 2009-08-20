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

#ifndef QGRAPHICSSHADEREFFECT_H
#define QGRAPHICSSHADEREFFECT_H

#include <QtGui/qgraphicseffect.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(OpenGL)

#if !defined(QT_NO_GRAPHICSVIEW) || (QT_EDITION & QT_MODULE_GRAPHICSVIEW) != QT_MODULE_GRAPHICSVIEW

class QGLShaderProgram;
class QGLCustomShaderEffectStage;
class QGraphicsShaderEffectPrivate;

class Q_OPENGL_EXPORT QGraphicsShaderEffect : public QGraphicsEffect
{
    Q_OBJECT
public:
    QGraphicsShaderEffect(QObject *parent = 0);
    virtual ~QGraphicsShaderEffect();

    QByteArray pixelShaderFragment() const;
    void setPixelShaderFragment(const QByteArray& code);

protected:
    void draw(QPainter *painter, QGraphicsEffectSource *source);
    void setUniformsDirty();
    virtual void setUniforms(QGLShaderProgram *program);

private:
    Q_DECLARE_PRIVATE(QGraphicsShaderEffect)
    Q_DISABLE_COPY(QGraphicsShaderEffect)

    friend class QGLCustomShaderEffectStage;
};

#endif // QT_NO_GRAPHICSVIEW

QT_END_NAMESPACE

QT_END_HEADER

#endif // QGRAPHICSSHADEREFFECT_H
