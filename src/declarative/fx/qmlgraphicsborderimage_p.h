/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
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

#ifndef QMLGRAPHICSBORDERIMAGE_H
#define QMLGRAPHICSBORDERIMAGE_H

#include <QtNetwork/qnetworkreply.h>
#include "qmlgraphicsimagebase_p.h"

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)

class QmlGraphicsScaleGrid;
class QmlGraphicsGridScaledImage;
class QmlGraphicsBorderImagePrivate;
class Q_DECLARATIVE_EXPORT QmlGraphicsBorderImage : public QmlGraphicsImageBase
{
    Q_OBJECT
    Q_ENUMS(TileMode)

    Q_PROPERTY(QmlGraphicsScaleGrid *border READ border CONSTANT)
    Q_PROPERTY(TileMode horizontalTileMode READ horizontalTileMode WRITE setHorizontalTileMode NOTIFY horizontalTileModeChanged)
    Q_PROPERTY(TileMode verticalTileMode READ verticalTileMode WRITE setVerticalTileMode NOTIFY verticalTileModeChanged)

public:
    QmlGraphicsBorderImage(QmlGraphicsItem *parent=0);
    ~QmlGraphicsBorderImage();

    QmlGraphicsScaleGrid *border();

    enum TileMode { Stretch = Qt::StretchTile, Repeat = Qt::RepeatTile, Round = Qt::RoundTile };

    TileMode horizontalTileMode() const;
    void setHorizontalTileMode(TileMode);

    TileMode verticalTileMode() const;
    void setVerticalTileMode(TileMode);

    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);
    void setSource(const QUrl &url);

Q_SIGNALS:
    void horizontalTileModeChanged();
    void verticalTileModeChanged();

protected:
    QmlGraphicsBorderImage(QmlGraphicsBorderImagePrivate &dd, QmlGraphicsItem *parent);

private:
    void setGridScaledImage(const QmlGraphicsGridScaledImage& sci);

private Q_SLOTS:
    void requestFinished();
    void sciRequestFinished();

private:
    Q_DISABLE_COPY(QmlGraphicsBorderImage)
    Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QmlGraphicsBorderImage)
};

QT_END_NAMESPACE
QML_DECLARE_TYPE(QmlGraphicsBorderImage)
QT_END_HEADER

#endif // QMLGRAPHICSBORDERIMAGE_H
