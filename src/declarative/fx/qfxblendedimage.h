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

#ifndef QFXBLENDEDIMAGE_H
#define QFXBLENDEDIMAGE_H

#include <QtDeclarative/qfxitem.h>
#if defined(QFX_RENDER_OPENGL2)
#include <gltexture.h>
#endif

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)

class Q_DECLARATIVE_EXPORT QFxBlendedImage : public QFxItem
{
    Q_OBJECT

    Q_PROPERTY(QString primaryUrl READ primaryUrl WRITE setPrimaryUrl)
    Q_PROPERTY(QString secondaryUrl READ secondaryUrl WRITE setSecondaryUrl)
    Q_PROPERTY(qreal blend READ blend WRITE setBlend)
    Q_PROPERTY(bool smooth READ smoothTransform WRITE setSmoothTransform)
public:
    QFxBlendedImage(QFxItem *parent=0);

    QString primaryUrl() const;
    void setPrimaryUrl(const QString &);

    QString secondaryUrl() const;
    void setSecondaryUrl(const QString &);

    qreal blend() const;
    void setBlend(qreal);

    bool smoothTransform() const;
    void setSmoothTransform(bool);

#if defined(QFX_RENDER_QPAINTER) 
    void paintContents(QPainter &painter);
#elif defined(QFX_RENDER_OPENGL2)
    void paintGLContents(GLPainter &);
#endif

private Q_SLOTS:
    void primaryLoaded();
    void secondaryLoaded();

private:
    QString primSrc;
    QString secSrc;
    QUrl primUrl;
    QUrl secUrl;

    qreal _blend;
    bool _smooth;
    bool dirty;
#if defined(QFX_RENDER_OPENGL2)
    GLTexture prim;
    GLTexture sec;
#endif
    QFxPixmap primPix;
    QFxPixmap secPix;
};
QML_DECLARE_TYPE(QFxBlendedImage);


QT_END_NAMESPACE

QT_END_HEADER
#endif // QFXBLENDEDIMAGE_H
