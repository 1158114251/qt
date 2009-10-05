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

#include "qfxrect.h"
#include "qfxrect_p.h"

#include <QPainter>

QT_BEGIN_NAMESPACE
QML_DEFINE_TYPE(Qt,4,6,(QT_VERSION&0x00ff00)>>8,Pen,QFxPen)
QML_DEFINE_TYPE(Qt,4,6,(QT_VERSION&0x00ff00)>>8,GradientStop,QFxGradientStop)
QML_DEFINE_TYPE(Qt,4,6,(QT_VERSION&0x00ff00)>>8,Gradient,QFxGradient)

/*!
    \internal
    \class QFxPen
    \brief The QFxPen class provides a pen used for drawing rectangle borders on a QmlView.

    By default, the pen is invalid and nothing is drawn. You must either set a color (then the default
    width is 1) or a width (then the default color is black).

    A width of 1 indicates is a single-pixel line on the border of the item being painted.

    Example:
    \qml
    Rectangle { border.width: 2; border.color: "red" ... }
    \endqml
*/

void QFxPen::setColor(const QColor &c)
{
    _color = c;
    _valid = _color.alpha() ? true : false;
    emit penChanged();
}

void QFxPen::setWidth(int w)
{
    if (_width == w)
        return;

    _width = w;
    _valid = (_width < 1) ? false : true;
    emit penChanged();
}


/*!
    \qmlclass GradientStop QFxGradientStop
    \brief The GradientStop item defines the color at a position in a Gradient

    \sa Gradient
*/

/*!
    \qmlproperty real GradientStop::position
    \qmlproperty color GradientStop::color

    Sets a \e color at a \e position in a gradient.
*/

void QFxGradientStop::updateGradient()
{
    if (QFxGradient *grad = qobject_cast<QFxGradient*>(parent()))
        grad->doUpdate();
}

/*!
    \qmlclass Gradient QFxGradient
    \brief The Gradient item defines a gradient fill.

    A gradient is defined by two or more colors, which will be blended seemlessly.  The
    colors are specified at their position in the range 0.0 - 1.0 via
    the GradientStop item.  For example, the following code paints a
    rectangle with a gradient starting with red, blending to yellow at 1/3 of the
    size of the rectangle, and ending with Green:

    \table
    \row
    \o \image gradient.png
    \o \quotefile doc/src/snippets/declarative/gradient.qml
    \endtable

    \sa GradientStop
*/

/*!
    \qmlproperty list<GradientStop> Gradient::stops
    This property holds the gradient stops describing the gradient.
*/

const QGradient *QFxGradient::gradient() const
{
    if (!m_gradient && !m_stops.isEmpty()) {
        m_gradient = new QLinearGradient(0,0,0,1.0);
        for (int i = 0; i < m_stops.count(); ++i) {
            const QFxGradientStop *stop = m_stops.at(i);
            m_gradient->setCoordinateMode(QGradient::ObjectBoundingMode);
            m_gradient->setColorAt(stop->position(), stop->color());
        }
    }

    return m_gradient;
}

void QFxGradient::doUpdate()
{
    delete m_gradient;
    m_gradient = 0;
    emit updated();
}

QML_DEFINE_TYPE(Qt,4,6,(QT_VERSION&0x00ff00)>>8,Rectangle,QFxRect)

/*!
    \qmlclass Rectangle QFxRect
    \brief The Rectangle item allows you to add rectangles to a scene.
    \inherits Item

    A Rectangle is painted having a solid fill (color) and an optional border.
    You can also create rounded rectangles using the radius property.

    \qml
    Rectangle {
        width: 100
        height: 100
        color: "red"
        border.color: "black"
        border.width: 5
        radius: 10
    }
    \endqml

    \image declarative-rect.png
*/

/*!
    \internal
    \class QFxRect
    \brief The QFxRect class provides a rectangle item that you can add to a QmlView.
*/
QFxRect::QFxRect(QFxItem *parent)
  : QFxItem(*(new QFxRectPrivate), parent)
{
    Q_D(QFxRect);
    d->init();
    setFlag(QGraphicsItem::ItemHasNoContents, false);
}

QFxRect::QFxRect(QFxRectPrivate &dd, QFxItem *parent)
  : QFxItem(dd, parent)
{
    Q_D(QFxRect);
    d->init();
    setFlag(QGraphicsItem::ItemHasNoContents, false);
}

void QFxRect::doUpdate()
{
    Q_D(QFxRect);
    d->rectImage = QPixmap();
    const int pw = d->pen && d->pen->isValid() ? d->pen->width() : 0;
    d->setPaintMargin((pw+1)/2);
    update();
}

/*!
    \qmlproperty int Rectangle::border.width
    \qmlproperty color Rectangle::border.color

    The width and color used to draw the border of the rectangle.

    A width of 1 creates a thin line. For no line, use a width of 0 or a transparent color.

    To keep the border smooth (rather than blurry), odd widths cause the rectangle to be painted at
    a half-pixel offset;
*/
QFxPen *QFxRect::border()
{
    Q_D(QFxRect);
    return d->getPen();
}

/*!
    \qmlproperty Gradient Rectangle::gradient

    The gradient to use to fill the rectangle.

    This property allows for the construction of simple vertical gradients.
    Other gradients may by formed by adding rotation to the rectangle.

    \table
    \row
    \o \image declarative-rect_gradient.png
    \o
    \qml
    Rectangle { y: 0; width: 80; height: 80; color: "lightsteelblue" }
    Rectangle { y: 100; width: 80; height: 80
        gradient: Gradient {
            GradientStop { position: 0.0; color: "lightsteelblue" }
            GradientStop { position: 1.0; color: "blue" }
        }
    }
    Rectangle { rotation: 90; x: 80; y: 200; width: 80; height: 80
        gradient: Gradient {
            GradientStop { position: 0.0; color: "lightsteelblue" }
            GradientStop { position: 1.0; color: "blue" }
        }
    }
    // The x offset is needed because the rotation is from the top left corner
    \endqml
    \endtable

    If both a gradient and a color are specified, the gradient will be used.

    \sa Gradient, color
*/
QFxGradient *QFxRect::gradient() const
{
    Q_D(const QFxRect);
    return d->gradient;
}

void QFxRect::setGradient(QFxGradient *gradient)
{
    Q_D(QFxRect);
    if (d->gradient == gradient)
        return;
    if (d->gradient)
        disconnect(d->gradient, SIGNAL(updated()), this, SLOT(doUpdate()));
    d->gradient = gradient;
    if (d->gradient)
        connect(d->gradient, SIGNAL(updated()), this, SLOT(doUpdate()));
    update();
}


/*!
    \qmlproperty real Rectangle::radius
    This property holds the corner radius used to draw a rounded rectangle.

    If radius is non-zero, the rectangle will be painted as a rounded rectangle, otherwise it will be
    painted as a normal rectangle. The same radius is used by all 4 corners; there is currently
    no way to specify different radii for different corners.
*/
qreal QFxRect::radius() const
{
    Q_D(const QFxRect);
    return d->radius;
}

void QFxRect::setRadius(qreal radius)
{
    Q_D(QFxRect);
    if (d->radius == radius)
        return;

    d->radius = radius;
    d->rectImage = QPixmap();
    update();
    emit radiusChanged();
}

/*!
    \qmlproperty color Rectangle::color
    This property holds the color used to fill the rectangle.

    \qml
    // green rectangle using hexidecimal notation
    Rectangle { color: "#00FF00" }

    // steelblue rectangle using SVG color name
    Rectangle { color: "steelblue" }
    \endqml

    The default color is white.

    If both a gradient and a color are specified, the gradient will be used.
*/
QColor QFxRect::color() const
{
    Q_D(const QFxRect);
    return d->color;
}

void QFxRect::setColor(const QColor &c)
{
    Q_D(QFxRect);
    if (d->color == c)
        return;

    d->color = c;
    d->rectImage = QPixmap();
    update();
    emit colorChanged();
}

void QFxRect::generateRoundedRect()
{
    Q_D(QFxRect);
    if (d->rectImage.isNull()) {
        const int pw = d->pen && d->pen->isValid() ? d->pen->width() : 0;
        d->rectImage = QPixmap(d->radius*2 + 3 + pw*2, d->radius*2 + 3 + pw*2);
        d->rectImage.fill(Qt::transparent);
        QPainter p(&(d->rectImage));
        p.setRenderHint(QPainter::Antialiasing);
        if (d->pen && d->pen->isValid()) {
            QPen pn(QColor(d->pen->color()), d->pen->width());
            p.setPen(pn);
        } else {
            p.setPen(Qt::NoPen);
        }
        p.setBrush(d->color);
        if (pw%2)
            p.drawRoundedRect(QRectF(qreal(pw)/2+1, qreal(pw)/2+1, d->rectImage.width()-(pw+1), d->rectImage.height()-(pw+1)), d->radius, d->radius);
        else
            p.drawRoundedRect(QRectF(qreal(pw)/2, qreal(pw)/2, d->rectImage.width()-pw, d->rectImage.height()-pw), d->radius, d->radius);
    }
}

void QFxRect::generateBorderedRect()
{
    Q_D(QFxRect);
    if (d->rectImage.isNull()) {
        const int pw = d->pen && d->pen->isValid() ? d->pen->width() : 0;
        d->rectImage = QPixmap(d->getPen()->width()*2 + 3 + pw*2, d->getPen()->width()*2 + 3 + pw*2);
        d->rectImage.fill(Qt::transparent);
        QPainter p(&(d->rectImage));
        p.setRenderHint(QPainter::Antialiasing);
        if (d->pen && d->pen->isValid()) {
            QPen pn(QColor(d->pen->color()), d->pen->width());
            pn.setJoinStyle(Qt::MiterJoin);
            p.setPen(pn);
        } else {
            p.setPen(Qt::NoPen);
        }
        p.setBrush(d->color);
        if (pw%2)
            p.drawRect(QRectF(qreal(pw)/2+1, qreal(pw)/2+1, d->rectImage.width()-(pw+1), d->rectImage.height()-(pw+1)));
        else
            p.drawRect(QRectF(qreal(pw)/2, qreal(pw)/2, d->rectImage.width()-pw, d->rectImage.height()-pw));
    }
}

void QFxRect::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *)
{
    Q_D(QFxRect);
    if (d->radius > 0 || (d->pen && d->pen->isValid())
        || (d->gradient && d->gradient->gradient()) ) {
        drawRect(*p);
    }
    else {
        bool oldAA = p->testRenderHint(QPainter::Antialiasing);
        if (d->smooth)
            p->setRenderHints(QPainter::Antialiasing, true);
        p->fillRect(QRectF(0, 0, width(), height()), d->color);
        if (d->smooth)
            p->setRenderHint(QPainter::Antialiasing, oldAA);
    }
}

void QFxRect::drawRect(QPainter &p)
{
    Q_D(QFxRect);
    if (d->gradient && d->gradient->gradient()) {
        // XXX This path is still slower than the image path
        // Image path won't work for gradients though
        bool oldAA = p.testRenderHint(QPainter::Antialiasing);
        if (d->smooth)
            p.setRenderHint(QPainter::Antialiasing);
        if (d->pen && d->pen->isValid()) {
            QPen pn(QColor(d->pen->color()), d->pen->width());
            p.setPen(pn);
        } else {
            p.setPen(Qt::NoPen);
        }
        p.setBrush(*d->gradient->gradient());
        if (d->radius > 0.)
            p.drawRoundedRect(0, 0, width(), height(), d->radius, d->radius);
        else
            p.drawRect(0, 0, width(), height());
        if (d->smooth)
            p.setRenderHint(QPainter::Antialiasing, oldAA);
    } else {
        bool oldAA = p.testRenderHint(QPainter::Antialiasing);
        bool oldSmooth = p.testRenderHint(QPainter::SmoothPixmapTransform);
        if (d->smooth)
            p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform, d->smooth);

        int offset = 0;
        const int pw = d->pen && d->pen->isValid() ? (d->pen->width()+1)/2*2 : 0;
        const int realpw = d->pen && d->pen->isValid() ? d->pen->width() : 0;

        if (d->radius > 0) {
            generateRoundedRect();
            //### implicit conversion to int
            offset = int(d->radius+realpw+1);
        } else {
            generateBorderedRect();
            offset = realpw+1;
        }

        //basically same code as QFxImage uses to paint sci images
        int w = width()+pw;
        int h = height()+pw;
        int xOffset = offset;
        int xSide = xOffset * 2;
        bool xMiddles=true;
        if (xSide > w) {
            xMiddles=false;
            xOffset = w/2 + 1;
            xSide = xOffset * 2;
        }
        int yOffset = offset;
        int ySide = yOffset * 2;
        bool yMiddles=true;
        if (ySide > h) {
            yMiddles = false;
            yOffset = h/2 + 1;
            ySide = yOffset * 2;
        }

        Q_ASSERT(d->rectImage.width() >= 2*xOffset + 1);
        Q_ASSERT(d->rectImage.height() >= 2*yOffset + 1);
        QMargins margins(xOffset, yOffset, xOffset, yOffset);
        QTileRules rules(Qt::StretchTile, Qt::StretchTile);
        qDrawBorderPixmap(&p, QRect(-pw/2, -pw/2, width()+pw, height()+pw), margins, d->rectImage, d->rectImage.rect(), margins, rules);

        if (d->smooth) {
            p.setRenderHint(QPainter::Antialiasing, oldAA);
            p.setRenderHint(QPainter::SmoothPixmapTransform, oldSmooth);
        }
    }
}

/*!
    \qmlproperty bool Rectangle::smooth

    Set this property if you want the item to be smoothly scaled or
    transformed.  Smooth filtering gives better visual quality, but is slower.  If
    the item is displayed at its natural size, this property has no visual or
    performance effect.

    \note Generally scaling artifacts are only visible if the item is stationary on
    the screen.  A common pattern when animating an item is to disable smooth
    filtering at the beginning of the animation and reenable it at the conclusion.
*/

QRectF QFxRect::boundingRect() const
{
    Q_D(const QFxRect);
    return QRectF(-d->paintmargin, -d->paintmargin, d->width+d->paintmargin*2, d->height+d->paintmargin*2);
}

QT_END_NAMESPACE
