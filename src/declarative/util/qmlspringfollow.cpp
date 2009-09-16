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

#include <limits.h>
#include <math.h>
#include <QtCore/qdebug.h>
#include "private/qobject_p.h"
#include "qmlspringfollow.h"
#include "private/qmlanimation_p.h"

QT_BEGIN_NAMESPACE

QML_DEFINE_TYPE(Qt,4,6,(QT_VERSION&0x00ff00)>>8,SpringFollow,QmlSpringFollow)

class QmlSpringFollowPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QmlSpringFollow)
public:
    QmlSpringFollowPrivate()
        : sourceValue(0), maxVelocity(0), lastTime(0)
        , mass(1.0), useMass(false), spring(0.), damping(0.), velocity(0), epsilon(0.01)
        , modulus(0.0), haveModulus(false), enabled(true), mode(Track), clock(this) {}

    QmlMetaProperty property;
    qreal currentValue;
    qreal sourceValue;
    qreal maxVelocity;
    qreal velocityms;
    int lastTime;
    qreal mass;
    bool useMass;
    qreal spring;
    qreal damping;
    qreal velocity;
    qreal epsilon;
    qreal modulus;
    bool haveModulus;
    bool enabled;

    enum Mode {
        Track,
        Velocity,
        Spring
    };
    Mode mode;

    void tick(int);
    void updateMode();
    void start();
    void stop();

    QTickAnimationProxy<QmlSpringFollowPrivate, &QmlSpringFollowPrivate::tick> clock;
};

void QmlSpringFollowPrivate::tick(int time)
{
    Q_Q(QmlSpringFollow);

    int elapsed = time - lastTime;
    if (!elapsed)
        return;
    qreal srcVal = sourceValue;
    if (haveModulus) {
        currentValue = fmod(currentValue, modulus);
        srcVal = fmod(srcVal, modulus);
    }
    if (mode == Spring) {
        if (elapsed < 16) // capped at 62fps.
            return;
        // Real men solve the spring DEs using RK4.
        // We'll do something much simpler which gives a result that looks fine.
        int count = elapsed / 16;
        for (int i = 0; i < count; ++i) {
            qreal diff = srcVal - currentValue;
            if (haveModulus && qAbs(diff) > modulus / 2) {
                if (diff < 0)
                    diff += modulus;
                else
                    diff -= modulus;
            }
            if (useMass)
                velocity = velocity + (spring * diff - damping * velocity) / mass;
            else
                velocity = velocity + spring * diff - damping * velocity;
            if (maxVelocity > 0.) {
                // limit velocity
                if (velocity > maxVelocity)
                    velocity = maxVelocity;
                else if (velocity < -maxVelocity)
                    velocity = -maxVelocity;
            }
            currentValue += velocity * 16.0 / 1000.0;
            if (haveModulus) {
                currentValue = fmod(currentValue, modulus);
                if (currentValue < 0.0)
                    currentValue += modulus;
            }
        }
        if (qAbs(velocity) < epsilon && qAbs(srcVal - currentValue) < epsilon) {
            velocity = 0.0;
            currentValue = srcVal;
            clock.stop();
        }
        lastTime = time - (elapsed - count * 16);
    } else {
        qreal moveBy = elapsed * velocityms;
        qreal diff = srcVal - currentValue;
        if (haveModulus && qAbs(diff) > modulus / 2) {
            if (diff < 0)
                diff += modulus;
            else
                diff -= modulus;
        }
        if (diff > 0) {
            currentValue += moveBy;
            if (haveModulus)
                currentValue = fmod(currentValue, modulus);
            if (currentValue > sourceValue) {
                currentValue = sourceValue;
                clock.stop();
            }
        } else {
            currentValue -= moveBy;
            if (haveModulus && currentValue < 0.0)
                currentValue = fmod(currentValue, modulus) + modulus;
            if (currentValue < sourceValue) {
                currentValue = sourceValue;
                clock.stop();
            }
        }
        lastTime = time;
    }
    property.write(currentValue);
    emit q->valueChanged(currentValue);
    if (clock.state() != QAbstractAnimation::Running)
        emit q->syncChanged();
}

void QmlSpringFollowPrivate::updateMode()
{
    if (spring == 0. && maxVelocity == 0.)
        mode = Track;
    else if (spring > 0.)
        mode = Spring;
    else
        mode = Velocity;
}

void QmlSpringFollowPrivate::start()
{
    if (!enabled)
        return;

    Q_Q(QmlSpringFollow);
    if (mode == QmlSpringFollowPrivate::Track) {
        currentValue = sourceValue;
        property.write(currentValue);
    } else if (sourceValue != currentValue && clock.state() != QAbstractAnimation::Running) {
        lastTime = 0;
        currentValue = property.read().toDouble();
        clock.start(); // infinity??
        emit q->syncChanged();
    }
}

void QmlSpringFollowPrivate::stop()
{
    clock.stop();
}

/*!
    \qmlclass SpringFollow QmlSpringFollow
    \brief The SpringFollow element allows a property to track a value.

    In example below, Rect2 will follow Rect1 moving with a velocity of up to 200:
    \code
    Rectangle {
        id: Rect1
        width: 20; height: 20
        color: "#00ff00"
        y: 200  //initial value
        y: SequentialAnimation {
            running: true
            repeat: true
            NumberAnimation {
                to: 200
                easing: "easeOutBounce(amplitude:100)"
                duration: 2000
            }
            PauseAnimation { duration: 1000 }
        }
    }
    Rectangle {
        id: Rect2
        x: Rect1.width
        width: 20; height: 20
        color: "#ff0000"
        y: SpringFollow { source: Rect1.y; velocity: 200 }
    }
    \endcode

    \sa EaseFollow
*/

QmlSpringFollow::QmlSpringFollow(QObject *parent)
: QObject(*(new QmlSpringFollowPrivate),parent)
{
}

QmlSpringFollow::~QmlSpringFollow()
{
}

void QmlSpringFollow::setTarget(const QmlMetaProperty &property)
{
    Q_D(QmlSpringFollow);
    d->property = property;
    d->currentValue = property.read().toDouble();
}

qreal QmlSpringFollow::sourceValue() const
{
    Q_D(const QmlSpringFollow);
    return d->sourceValue;
}

/*!
    \qmlproperty qreal SpringFollow::source
    This property holds the source value which will be tracked.

    Bind to a property in order to track its changes.
*/

void QmlSpringFollow::setSourceValue(qreal value)
{
    Q_D(QmlSpringFollow);
    if (d->sourceValue != value) {
        d->sourceValue = value;
        d->start();
    }
}

/*!
    \qmlproperty qreal SpringFollow::velocity
    This property holds the maximum velocity allowed when tracking the source.
*/

qreal QmlSpringFollow::velocity() const
{
    Q_D(const QmlSpringFollow);
    return d->maxVelocity;
}

void QmlSpringFollow::setVelocity(qreal velocity)
{
    Q_D(QmlSpringFollow);
    d->maxVelocity = velocity;
    d->velocityms = velocity / 1000.0;
    d->updateMode();
}

/*!
    \qmlproperty qreal SpringFollow::spring
    This property holds the spring constant

    The spring constant describes how strongly the target is pulled towards the
    source.  Setting spring to 0 turns off spring tracking.  Useful values 0 - 5.0

    When a spring constant is set and the velocity property is greater than 0,
    velocity limits the maximum speed.
*/
qreal QmlSpringFollow::spring() const
{
    Q_D(const QmlSpringFollow);
    return d->spring;
}

void QmlSpringFollow::setSpring(qreal spring)
{
    Q_D(QmlSpringFollow);
    d->spring = spring;
    d->updateMode();
}

/*!
    \qmlproperty qreal SpringFollow::damping
    This property holds the spring damping constant

    The damping constant describes how quickly a sprung follower comes to rest.
    Useful range is 0 - 1.0
*/
qreal QmlSpringFollow::damping() const
{
    Q_D(const QmlSpringFollow);
    return d->damping;
}

void QmlSpringFollow::setDamping(qreal damping)
{
    Q_D(QmlSpringFollow);
    if (damping > 1.)
        damping = 1.;

    d->damping = damping;
}


/*!
    \qmlproperty qreal SpringFollow::epsilon
    This property holds the spring epsilon

    The epsilon is the rate and amount of change in the value which is close enough
    to 0 to be considered equal to zero. This will depend on the usage of the value.
    For pixel positions, 0.25 would suffice. For scale, 0.005 will suffice.

    The default is 0.01. Tuning this value can provide small performance improvements.
*/
qreal QmlSpringFollow::epsilon() const
{
    Q_D(const QmlSpringFollow);
    return d->epsilon;
}

void QmlSpringFollow::setEpsilon(qreal epsilon)
{
    Q_D(QmlSpringFollow);
    d->epsilon = epsilon;
}

/*!
    \qmlproperty qreal SpringFollow::modulus
    This property holds the modulus value.

    Setting a \a modulus forces the target value to "wrap around" at the modulus.
    For example, setting the modulus to 360 will cause a value of 370 to wrap around to 10.
*/
qreal QmlSpringFollow::modulus() const
{
    Q_D(const QmlSpringFollow);
    return d->modulus;
}

void QmlSpringFollow::setModulus(qreal modulus)
{
    Q_D(QmlSpringFollow);
    if (d->modulus != modulus) {
        d->haveModulus = modulus != 0.0;
        d->modulus = modulus;
        emit modulusChanged();
    }
}

/*!
    \qmlproperty qreal SpringFollow::mass
    This property holds the "mass" of the property being moved.

    mass is 1.0 by default.  Setting a different mass changes the dynamics of
    a \l spring follow.
*/
qreal QmlSpringFollow::mass() const
{
    Q_D(const QmlSpringFollow);
    return d->mass;
}

void QmlSpringFollow::setMass(qreal mass)
{
    Q_D(QmlSpringFollow);
    if (d->mass != mass && mass > 0.0) {
        d->useMass = mass != 1.0;
        d->mass = mass;
        emit massChanged();
    }
}

/*!
    \qmlproperty qreal SpringFollow::value
    The current value.
*/

/*!
    \qmlproperty bool SpringFollow::enabled
    This property holds whether the target will track the source.
*/
bool QmlSpringFollow::enabled() const
{
    Q_D(const QmlSpringFollow);
    return d->enabled;
}

void QmlSpringFollow::setEnabled(bool enabled)
{
    Q_D(QmlSpringFollow);
    d->enabled = enabled;
    if (enabled)
        d->start();
    else
        d->stop();
}

/*!
    \qmlproperty bool SpringFollow::inSync
    This property is true when target is equal to the source; otherwise
    false.  If inSync is true the target is not being animated.

    If \l enabled is false then inSync will also be false.
*/
bool QmlSpringFollow::inSync() const
{
    Q_D(const QmlSpringFollow);
    return d->enabled && d->clock.state() != QAbstractAnimation::Running;
}

qreal QmlSpringFollow::value() const
{
    Q_D(const QmlSpringFollow);
    return d->currentValue;
}

QT_END_NAMESPACE
