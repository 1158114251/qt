/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the $MODULE$ of the Qt Toolkit.
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

#include "qvector2d.h"
#include "qvector3d.h"
#include "qvector4d.h"
#include <QtCore/qdebug.h>
#include <QtCore/qmath.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_VECTOR2D

/*!
    \class QVector2D
    \brief The QVector2D class represents a vector or vertex in 2D space.
    \since 4.6

    The QVector2D class can also be used to represent vertices in 2D space.
    We therefore do not need to provide a separate vertex class.

    The coordinates are stored internally using the most efficient
    representation for the GL rendering engine, which will be either
    floating-point or fixed-point.
*/

/*!
    \fn QVector2D::QVector2D()

    Constructs a null vector, i.e. with coordinates (0, 0, 0).
*/

/*!
    \fn QVector2D::QVector2D(qreal xpos, qreal ypos)

    Constructs a vector with coordinates (\a xpos, \a ypos).
*/

/*!
    \fn QVector2D::QVector2D(const QPoint& point)

    Constructs a vector with x and y coordinates from a 2D \a point.
*/

/*!
    \fn QVector2D::QVector2D(const QPointF& point)

    Constructs a vector with x and y coordinates from a 2D \a point.
*/

#ifndef QT_NO_VECTOR3D

/*!
    Constructs a vector with x and y coordinates from a 3D \a vector.
    The z coordinate of \a vector is dropped.

    \sa toVector3D()
*/
QVector2D::QVector2D(const QVector3D& vector)
{
    xp = vector.xp;
    yp = vector.yp;
}

#endif

#ifndef QT_NO_VECTOR4D

/*!
    Constructs a vector with x and y coordinates from a 3D \a vector.
    The z and w coordinates of \a vector are dropped.

    \sa toVector4D()
*/
QVector2D::QVector2D(const QVector4D& vector)
{
    xp = vector.xp;
    yp = vector.yp;
}

#endif

/*!
    \fn bool QVector2D::isNull() const

    Returns true if the x and y coordinates are set to 0.0,
    otherwise returns false.
*/

/*!
    \fn qreal QVector2D::x() const

    Returns the x coordinate of this point.

    \sa setX(), y()
*/

/*!
    \fn qreal QVector2D::y() const

    Returns the y coordinate of this point.

    \sa setY(), x()
*/

/*!
    \fn void QVector2D::setX(qreal x)

    Sets the x coordinate of this point to the given \a x coordinate.

    \sa x(), setY()
*/

/*!
    \fn void QVector2D::setY(qreal y)

    Sets the y coordinate of this point to the given \a y coordinate.

    \sa y(), setX()
*/

/*!
    Returns the length of the vector from the origin.

    \sa lengthSquared(), normalized()
*/
qreal QVector2D::length() const
{
    return qSqrt(xp * xp + yp * yp);
}

/*!
    Returns the squared length of the vector from the origin.
    This is equivalent to the dot product of the vector with itself.

    \sa length(), dotProduct()
*/
qreal QVector2D::lengthSquared() const
{
    return xp * xp + yp * yp;
}

/*!
    Returns the normalized unit vector form of this vector.  If this vector
    is not null, the returned vector is guaranteed to be 1.0 in length.
    If this vector is null, then a null vector is returned.

    \sa length(), normalize()
*/
QVector2D QVector2D::normalized() const
{
    qreal len = length();
    if (!qIsNull(len))
        return *this / len;
    else
        return QVector2D();
}

/*!
    Normalizes the currect vector in place.  Nothing happens if this
    vector is a null vector.

    \sa length(), normalized()
*/
void QVector2D::normalize()
{
    qreal len = length();
    if (qIsNull(len))
        return;

    xp /= len;
    yp /= len;
}

/*!
    \fn QVector2D &QVector2D::operator+=(const QVector2D &vector)

    Adds the given \a vector to this vector and returns a reference to
    this vector.

    \sa operator-=()
*/

/*!
    \fn QVector2D &QVector2D::operator-=(const QVector2D &vector)

    Subtracts the given \a vector from this vector and returns a reference to
    this vector.

    \sa operator+=()
*/

/*!
    \fn QVector2D &QVector2D::operator*=(qreal factor)

    Multiplies this vector's coordinates by the given \a factor, and
    returns a reference to this vector.

    \sa operator/=()
*/

/*!
    \fn QVector2D &QVector2D::operator*=(const QVector2D &vector)

    Multiplies the components of this vector by the corresponding
    components in \a vector.
*/

/*!
    \fn QVector2D &QVector2D::operator/=(qreal divisor)

    Divides this vector's coordinates by the given \a divisor, and
    returns a reference to this vector.

    \sa operator*=()
*/

/*!
    Returns the dot product of \a v1 and \a v2.
*/
qreal QVector2D::dotProduct(const QVector2D& v1, const QVector2D& v2)
{
    return v1.xp * v2.xp + v1.yp * v2.yp;
}

/*!
    \fn bool operator==(const QVector2D &v1, const QVector2D &v2)
    \relates QVector2D

    Returns true if \a v1 is equal to \a v2; otherwise returns false.
    This operator uses an exact floating-point comparison.
*/

/*!
    \fn bool operator!=(const QVector2D &v1, const QVector2D &v2)
    \relates QVector2D

    Returns true if \a v1 is not equal to \a v2; otherwise returns false.
    This operator uses an exact floating-point comparison.
*/

/*!
    \fn const QVector2D operator+(const QVector2D &v1, const QVector2D &v2)
    \relates QVector2D

    Returns a QVector2D object that is the sum of the given vectors, \a v1
    and \a v2; each component is added separately.

    \sa QVector2D::operator+=()
*/

/*!
    \fn const QVector2D operator-(const QVector2D &v1, const QVector2D &v2)
    \relates QVector2D

    Returns a QVector2D object that is formed by subtracting \a v2 from \a v1;
    each component is subtracted separately.

    \sa QVector2D::operator-=()
*/

/*!
    \fn const QVector2D operator*(qreal factor, const QVector2D &vector)
    \relates QVector2D

    Returns a copy of the given \a vector,  multiplied by the given \a factor.

    \sa QVector2D::operator*=()
*/

/*!
    \fn const QVector2D operator*(const QVector2D &vector, qreal factor)
    \relates QVector2D

    Returns a copy of the given \a vector,  multiplied by the given \a factor.

    \sa QVector2D::operator*=()
*/

/*!
    \fn const QVector2D operator*(const QVector2D &v1, const QVector2D &v2)
    \relates QVector2D

    Multiplies the components of \a v1 by the corresponding
    components in \a v2.
*/

/*!
    \fn const QVector2D operator-(const QVector2D &vector)
    \relates QVector2D
    \overload

    Returns a QVector2D object that is formed by changing the sign of
    the components of the given \a vector.

    Equivalent to \c {QVector2D(0,0) - vector}.
*/

/*!
    \fn const QVector2D operator/(const QVector2D &vector, qreal divisor)
    \relates QVector2D

    Returns the QVector2D object formed by dividing all three components of
    the given \a vector by the given \a divisor.

    \sa QVector2D::operator/=()
*/

/*!
    \fn bool qFuzzyCompare(const QVector2D& v1, const QVector2D& v2)
    \relates QVector2D

    Returns true if \a v1 and \a v2 are equal, allowing for a small
    fuzziness factor for floating-point comparisons; false otherwise.
*/

#ifndef QT_NO_VECTOR3D

/*!
    Returns the 3D form of this 2D vector, with the z coordinate set to zero.

    \sa toVector4D(), toPoint()
*/
QVector3D QVector2D::toVector3D() const
{
    return QVector3D(xp, yp, 0.0f, 1);
}

#endif

#ifndef QT_NO_VECTOR4D

/*!
    Returns the 4D form of this 2D vector, with the z and w coordinates set to zero.

    \sa toVector3D(), toPoint()
*/
QVector4D QVector2D::toVector4D() const
{
    return QVector4D(xp, yp, 0.0f, 0.0f, 1);
}

#endif

/*!
    \fn QPoint QVector2D::toPoint() const

    Returns the QPoint form of this 2D vector.

    \sa toPointF(), toVector3D()
*/

/*!
    \fn QPointF QVector2D::toPointF() const

    Returns the QPointF form of this 2D vector.

    \sa toPoint(), toVector3D()
*/

#ifndef QT_NO_DEBUG_STREAM

QDebug operator<<(QDebug dbg, const QVector2D &vector)
{
    dbg.nospace() << "QVector2D(" << vector.x() << ", " << vector.y() << ')';
    return dbg.space();
}

#endif

#endif

QT_END_NAMESPACE
