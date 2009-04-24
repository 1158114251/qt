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
#include <QtCore/qdebug.h>
#include <gfxtimeline.h>
#include "private/qobject_p.h"
#include "qmlfollow.h"
#include "private/qmlanimation_p.h"


QT_BEGIN_NAMESPACE
QML_DEFINE_TYPE(QmlFollow,Follow);

class QmlFollowPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QmlFollow)
public:
    QmlFollowPrivate()
        : sourceValue(0), maxVelocity(0), lastTime(0)
        , mass(1.0), spring(0.), damping(0.), velocity(0), enabled(true), mode(Track), clock(this) {}

    QmlMetaProperty property;
    qreal currentValue;
    qreal sourceValue;
    qreal maxVelocity;
    qreal velocityms;
    int lastTime;
    qreal mass;
    qreal spring;
    qreal damping;
    qreal velocity;
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

    QTickAnimationProxy<QmlFollowPrivate, &QmlFollowPrivate::tick> clock;
};

void QmlFollowPrivate::tick(int time)
{
    Q_Q(QmlFollow);

    int elapsed = time - lastTime;
    if (!elapsed)
        return;
    if (mode == Spring) {
        if (elapsed < 10) // capped at 100fps.
            return;
        // Real men solve the spring DEs using RK4.
        // We'll do something much simpler which gives a result that looks fine.
        int count = (elapsed+5) / 10;
        for (int i = 0; i < count; ++i) {
            qreal diff = sourceValue - currentValue;
            velocity = velocity + spring * diff - damping * velocity;
            // The following line supports mass.  Not sure its worth the extra divisions.
            // velocity = velocity + spring / mass * diff - damping / mass * velocity;
            if (maxVelocity > 0.) {
                // limit velocity
                if (velocity > maxVelocity)
                    velocity = maxVelocity;
                else if (velocity < -maxVelocity)
                    velocity = -maxVelocity;
            }
            currentValue += velocity * 10.0 / 1000.0;
        }
        if (qAbs(velocity) < 0.5 && qAbs(sourceValue - currentValue) < 0.5) {
            velocity = 0.0;
            currentValue = sourceValue;
            clock.stop();
        }
        lastTime = time - (elapsed - count * 10);
    } else {
        qreal moveBy = elapsed * velocityms;
        qreal diff = sourceValue - currentValue;
        if (diff > 0) {
            currentValue += moveBy;
            if (currentValue > sourceValue) {
                currentValue = sourceValue;
                clock.stop();
            }
        } else {
            currentValue -= moveBy;
            if (currentValue < sourceValue) {
                currentValue = sourceValue;
                clock.stop();
            }
        }
        lastTime = time;
    }
    emit q->valueChanged(currentValue);
    property.write(currentValue);
}

void QmlFollowPrivate::updateMode()
{
    if (spring == 0. && maxVelocity == 0.)
        mode = Track;
    else if (spring > 0.)
        mode = Spring;
    else
        mode = Velocity;
}

void QmlFollowPrivate::start()
{
    if (!enabled)
        return;

    if (mode == QmlFollowPrivate::Track) {
        currentValue = sourceValue;
        property.write(currentValue);
    } else if (sourceValue != currentValue && clock.state() != QAbstractAnimation::Running) {
        lastTime = 0;
        clock.start(); // infinity??
    }
}

void QmlFollowPrivate::stop()
{
    clock.stop();
}

/*!
    \qmlclass Follow QmlFollow
    \brief The Follow element allows a property to track a value.

    In example below, Rect2 will follow Rect1 moving with a velocity of up to 200:
    \code
    <Rect id="Rect1" y="{200}" width="20" height="20" color="#00ff00">
        <y>
            <SequentialAnimation running="true" repeat="true">
                <NumericAnimation to="{200}" easing="easeOutBounce(amplitude:100)" duration="2000" />
                <PauseAnimation duration="1000" />
            </SequentialAnimation>
        </y>
    </Rect>
    <Rect id="Rect2" x="{Rect1.width}" width="20" height="20" color="#ff0000">
        <y>
            <Follow source="{Rect1.y}" velocity="200"/>
        </y>
    </Rect>
    \endcode
*/

QmlFollow::QmlFollow(QObject *parent)
: QmlPropertyValueSource(*(new QmlFollowPrivate),parent)
{
}

QmlFollow::~QmlFollow()
{
}

void QmlFollow::setTarget(const QmlMetaProperty &property)
{
    Q_D(QmlFollow);
    d->property = property;
    d->currentValue = property.read().toDouble();
}

qreal QmlFollow::sourceValue() const
{
    Q_D(const QmlFollow);
    return d->sourceValue;
}

/*!
    \qmlproperty qreal Follow::source
    This property holds the source value which will be tracked.

    Bind to a property in order to track its changes.
*/

void QmlFollow::setSourceValue(qreal value)
{
    Q_D(QmlFollow);
    d->sourceValue = value;
    d->start();
}

/*!
    \qmlproperty qreal Follow::velocity
    This property holds the maximum velocity allowed when tracking the source.
*/

qreal QmlFollow::velocity() const
{
    Q_D(const QmlFollow);
    return d->maxVelocity;
}

void QmlFollow::setVelocity(qreal velocity)
{
    Q_D(QmlFollow);
    d->maxVelocity = velocity;
    d->velocityms = velocity / 1000.0;
    d->updateMode();
}

/*!
    \qmlproperty qreal Follow::spring
    This property holds the spring constant

    The spring constant describes how strongly the target is pulled towards the
    source.  Setting spring to 0 turns off spring tracking.  Useful values 0 - 5.0

    When a spring constant is set and the velocity property is greater than 0,
    velocity limits the maximum speed.
*/
qreal QmlFollow::spring() const
{
    Q_D(const QmlFollow);
    return d->spring;
}

void QmlFollow::setSpring(qreal spring)
{
    Q_D(QmlFollow);
    d->spring = spring;
    d->updateMode();
}

/*!
    \qmlproperty qreal Follow::damping
    This property holds the spring damping constant

    The damping constant describes how quickly a sprung follower comes to rest.
    Useful range is 0 - 1.0
*/
qreal QmlFollow::damping() const
{
    Q_D(const QmlFollow);
    return d->damping;
}

void QmlFollow::setDamping(qreal damping)
{
    Q_D(QmlFollow);
    if (damping > 1.)
        damping = 1.;

    d->damping = damping;
}

/*!
    \qmlproperty qreal Follow::followValue
    The current value.
*/

/*!
    \qmlproperty bool Follow::enabled
    This property holds whether the target will track the source.
*/
bool QmlFollow::enabled() const
{
    Q_D(const QmlFollow);
    return d->enabled;
}

void QmlFollow::setEnabled(bool enabled)
{
    Q_D(QmlFollow);
    d->enabled = enabled;
    if (enabled)
        d->start();
    else
        d->stop();
}

qreal QmlFollow::value() const
{
    Q_D(const QmlFollow);
    return d->currentValue;
}

QT_END_NAMESPACE
