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

#include "qmlanimation_p.h"
#include "qvariant.h"
#include "qcolor.h"
#include "qfile.h"
#include "qmlpropertyvaluesource.h"
#include "qml.h"
#include "qmlinfo.h"
#include "qmlanimation_p_p.h"
#include <private/qmlbehavior_p.h>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QtCore/qset.h>
#include <QtCore/qrect.h>
#include <QtCore/qpoint.h>
#include <QtCore/qsize.h>
#include <QtDeclarative/qmlexpression.h>
#include <private/qmlstateoperations_p.h>
#include <private/qmlstringconverters_p.h>
#include <private/qvariantanimation_p.h>

QT_BEGIN_NAMESPACE

QEasingCurve stringToCurve(const QString &curve)
{
    QEasingCurve easingCurve;

    QString normalizedCurve = curve;
    bool hasParams = curve.contains(QLatin1Char('('));
    QStringList props;

    if (hasParams) {
        QString easeName = curve.trimmed();
        if (!easeName.endsWith(QLatin1Char(')'))) {
            qWarning("QEasingCurve: Unmatched perenthesis in easing function '%s'",
                     qPrintable(curve));
            return easingCurve;
        }

        int idx = easeName.indexOf(QLatin1Char('('));
        QString prop_str =
            easeName.mid(idx + 1, easeName.length() - 1 - idx - 1);
        normalizedCurve = easeName.left(idx);
        if (!normalizedCurve.startsWith(QLatin1String("ease"))) {
            qWarning("QEasingCurve: Easing function '%s' must start with 'ease'",
                     qPrintable(curve));
        }

        props = prop_str.split(QLatin1Char(','));
    }

    if (normalizedCurve.startsWith(QLatin1String("ease")))
        normalizedCurve = normalizedCurve.mid(4);

    static int index = QEasingCurve::staticMetaObject.indexOfEnumerator("Type");
    static QMetaEnum me = QEasingCurve::staticMetaObject.enumerator(index);

    int value = me.keyToValue(normalizedCurve.toUtf8().constData());
    if (value < 0) {
        qWarning("QEasingCurve: Unknown easing curve '%s'",
                 qPrintable(curve));
        value = 0;
    }
    easingCurve.setType((QEasingCurve::Type)value);

    if (hasParams) {
        foreach(const QString &str, props) {
            int sep = str.indexOf(QLatin1Char(':'));

            if (sep == -1) {
                qWarning("QEasingCurve: Improperly specified property in easing function '%s'",
                         qPrintable(curve));
                return easingCurve;
            }

            QString propName = str.left(sep).trimmed();
            bool isOk;
            qreal propValue = str.mid(sep + 1).trimmed().toDouble(&isOk);

            if (propName.isEmpty() || !isOk) {
                qWarning("QEasingCurve: Improperly specified property in easing function '%s'",
                         qPrintable(curve));
                return easingCurve;
            }

            if (propName == QLatin1String("amplitude")) {
                easingCurve.setAmplitude(propValue);
            } else if (propName == QLatin1String("period")) {
                easingCurve.setPeriod(propValue);
            } else if (propName == QLatin1String("overshoot")) {
                easingCurve.setOvershoot(propValue);
            }
        }
    }

    return easingCurve;
}

QML_DEFINE_NOCREATE_TYPE(QmlAbstractAnimation)

/*!
    \qmlclass Animation
    \brief The Animation element is the base of all QML animations.

    The Animation element cannot be used directly in a QML file.  It exists
    to provide a set of common properties and methods, available across all the
    other animation types that inherit from it.  Attempting to use the Animation
    element directly will result in an error.
*/

/*!
    \class QmlAbstractAnimation
    \internal
*/

QmlAbstractAnimation::QmlAbstractAnimation(QObject *parent)
: QObject(*(new QmlAbstractAnimationPrivate), parent)
{
}

QmlAbstractAnimation::~QmlAbstractAnimation()
{
}

QmlAbstractAnimation::QmlAbstractAnimation(QmlAbstractAnimationPrivate &dd, QObject *parent)
: QObject(dd, parent)
{
}

/*!
    \qmlproperty bool Animation::running
    This property holds whether the animation is currently running.

    The \c running property can be set to declaratively control whether or not
    an animation is running.  The following example will animate a rectangle
    whenever the \l MouseRegion is pressed.

    \code
    Rectangle {
        width: 100; height: 100
        x: NumberAnimation {
            running: myMouse.pressed
            from: 0; to: 100
        }
        MouseRegion { id: myMouse }
    }
    \endcode

    Likewise, the \c running property can be read to determine if the animation
    is running.  In the following example the text element will indicate whether
    or not the animation is running.

    \code
    NumberAnimation { id: myAnimation }
    Text { text: myAnimation.running ? "Animation is running" : "Animation is not running" }
    \endcode

    Animations can also be started and stopped imperatively from JavaScript
    using the \c start() and \c stop() methods.

    By default, animations are not running.
*/
bool QmlAbstractAnimation::isRunning() const
{
    Q_D(const QmlAbstractAnimation);
    return d->running;
}

void QmlAbstractAnimationPrivate::commence()
{
    Q_Q(QmlAbstractAnimation);

    q->prepare(userProperty.value);
    q->qtAnimation()->start();
    if (q->qtAnimation()->state() != QAbstractAnimation::Running) {
        running = false;
        emit q->completed();
    }
}

//### make static?
QmlMetaProperty QmlAbstractAnimationPrivate::createProperty(QObject *obj, const QString &str)
{
    Q_Q(QmlAbstractAnimation);
    QmlMetaProperty prop = QmlMetaProperty::createProperty(obj, str);
    if (!prop.isValid()) {
        qmlInfo(QmlAbstractAnimation::tr("Cannot animate non-existant property \"%1\"").arg(str), q);
        return QmlMetaProperty();
    } else if (!prop.isWritable()) {
        qmlInfo(QmlAbstractAnimation::tr("Cannot animate read-only property \"%1\"").arg(str), q);
        return QmlMetaProperty();
    }
    return prop;
}

void QmlAbstractAnimation::setRunning(bool r)
{
    Q_D(QmlAbstractAnimation);
    if (d->running == r)
        return;

    if (d->group) {
        qWarning("QmlAbstractAnimation: setRunning() cannot be used on non-root animation nodes");
        return;
    }

    d->running = r;
    if (d->running) {
        if (!d->connectedTimeLine) {
            QObject::connect(qtAnimation(), SIGNAL(finished()),
                             this, SLOT(timelineComplete()));
            d->connectedTimeLine = true;
        }
        if (d->componentComplete)
            d->commence();
        else
            d->startOnCompletion = true;
        emit started();
    } else {
        if (d->alwaysRunToEnd) {
            if (d->repeat)
                qtAnimation()->setLoopCount(qtAnimation()->currentLoop()+1);
        } else
            qtAnimation()->stop();

        emit completed();
    }

    emit runningChanged(d->running);
}

/*!
    \qmlproperty bool Animation::paused
    This property holds whether the animation is currently paused.

    The \c paused property can be set to declaratively control whether or not
    an animation is paused.

    Animations can also be paused and resumed imperatively from JavaScript
    using the \c pause() and \c resume() methods.

    By default, animations are not paused.
*/
bool QmlAbstractAnimation::isPaused() const
{
    Q_D(const QmlAbstractAnimation);
    return d->paused;
}

void QmlAbstractAnimation::setPaused(bool p)
{
    Q_D(QmlAbstractAnimation);
    if (d->paused == p)
        return;

    if (d->group) {
        qWarning("QmlAbstractAnimation: setPaused() cannot be used on non-root animation nodes");
        return;
    }

    d->paused = p;
    if (d->paused)
        qtAnimation()->pause();
    else
        qtAnimation()->resume();

    emit pausedChanged(d->running);
}

void QmlAbstractAnimation::classBegin()
{
    Q_D(QmlAbstractAnimation);
    d->componentComplete = false;
}

void QmlAbstractAnimation::componentComplete()
{
    Q_D(QmlAbstractAnimation);
    if (d->startOnCompletion)
        d->commence();
    d->componentComplete = true;
}

/*!
    \qmlproperty bool Animation::alwaysRunToEnd
    This property holds whether the animation should run to completion when it is stopped.

    If this true the animation will complete its current iteration when it
    is stopped - either by setting the \c running property to false, or by
    calling the \c stop() method.  The \c complete() method is not effected
    by this value.

    This behavior is most useful when the \c repeat property is set, as the
    animation will finish playing normally but not restart.

    By default, the alwaysRunToEnd property is not set.
*/
bool QmlAbstractAnimation::alwaysRunToEnd() const
{
    Q_D(const QmlAbstractAnimation);
    return d->alwaysRunToEnd;
}

void QmlAbstractAnimation::setAlwaysRunToEnd(bool f)
{
    Q_D(QmlAbstractAnimation);
    if (d->alwaysRunToEnd == f)
        return;

    d->alwaysRunToEnd = f;
    emit alwaysRunToEndChanged(f);
}

/*!
    \qmlproperty bool Animation::repeat
    This property holds whether the animation should repeat.

    If set, the animation will continuously repeat until it is explicitly
    stopped - either by setting the \c running property to false, or by calling
    the \c stop() method.

    In the following example, the rectangle will spin indefinately.

    \code
    Rectangle {
        rotation: NumberAnimation { running: true; repeat: true; from: 0 to: 360 }
    }
    \endcode
*/
bool QmlAbstractAnimation::repeat() const
{
    Q_D(const QmlAbstractAnimation);
    return d->repeat;
}

void QmlAbstractAnimation::setRepeat(bool r)
{
    Q_D(QmlAbstractAnimation);
    if (r == d->repeat)
        return;

    d->repeat = r;
    int lc = r ? -1 : 1;
    qtAnimation()->setLoopCount(lc);
    emit repeatChanged(r);
}

int QmlAbstractAnimation::currentTime()
{
    return qtAnimation()->currentTime();
}

void QmlAbstractAnimation::setCurrentTime(int time)
{
    qtAnimation()->setCurrentTime(time);
}

QmlAnimationGroup *QmlAbstractAnimation::group() const
{
    Q_D(const QmlAbstractAnimation);
    return d->group;
}

void QmlAbstractAnimation::setGroup(QmlAnimationGroup *g)
{
    Q_D(QmlAbstractAnimation);
    if (d->group == g)
        return;
    if (d->group)
        static_cast<QmlAnimationGroupPrivate *>(d->group->d_func())->animations.removeAll(this);

    d->group = g;

    if (d->group && !static_cast<QmlAnimationGroupPrivate *>(d->group->d_func())->animations.contains(this))
        static_cast<QmlAnimationGroupPrivate *>(d->group->d_func())->animations.append(this);

    if (d->group)
        ((QAnimationGroup*)d->group->qtAnimation())->addAnimation(qtAnimation());

    //if (g) //if removed from a group, then the group should no longer be the parent
        setParent(g);
}

/*!
    \qmlproperty Object PropertyAction::target
    This property holds an explicit target object to animate.

    The exact effect of the \c target property depends on how the animation
    is being used.  Refer to the \l animation documentation for details.
*/

/*!
    \qmlproperty Object PropertyAnimation::target
    This property holds an explicit target object to animate.

    The exact effect of the \c target property depends on how the animation
    is being used.  Refer to the \l animation documentation for details.
*/
QObject *QmlAbstractAnimation::target() const
{
    Q_D(const QmlAbstractAnimation);
    return d->target;
}

void QmlAbstractAnimation::setTarget(QObject *o)
{
    Q_D(QmlAbstractAnimation);
    if (d->target == o)
        return;

    d->target = o;
    if (d->target && !d->propertyName.isEmpty()) {
        d->userProperty = d->createProperty(d->target, d->propertyName);
    } else {
        d->userProperty.invalidate();
    }

    emit targetChanged(d->target, d->propertyName);
}

/*!
    \qmlproperty string PropertyAction::property
    This property holds an explicit property to animated.

    The exact effect of the \c property property depends on how the animation
    is being used.  Refer to the \l animation documentation for details.
*/

/*!
    \qmlproperty string PropertyAnimation::property
    This property holds an explicit property to animated.

    The exact effect of the \c property property depends on how the animation
    is being used.  Refer to the \l animation documentation for details.
*/
QString QmlAbstractAnimation::property() const
{
    Q_D(const QmlAbstractAnimation);
    return d->propertyName;
}

void QmlAbstractAnimation::setProperty(const QString &n)
{
    Q_D(QmlAbstractAnimation);
    if (d->propertyName == n)
        return;

    d->propertyName = n;
    if (d->target && !d->propertyName.isEmpty()) {
        d->userProperty = d->createProperty(d->target, d->propertyName);
    } else {
        d->userProperty.invalidate();
    }

    emit targetChanged(d->target, d->propertyName);
}

/*!
    \qmlmethod Animation::start()
    \brief Starts the animation.

    If the animation is already running, calling this method has no effect.  The
    \c running property will be true following a call to \c start().
*/
void QmlAbstractAnimation::start()
{
    setRunning(true);
}

/*!
    \qmlmethod Animation::pause()
    \brief Pauses the animation.

    If the animation is already paused, calling this method has no effect.  The
    \c paused property will be true following a call to \c pause().
*/
void QmlAbstractAnimation::pause()
{
    setPaused(true);
}

/*!
    \qmlmethod Animation::resume()
    \brief Resumes a paused animation.

    If the animation is not paused, calling this method has no effect.  The
    \c paused property will be false following a call to \c resume().
*/
void QmlAbstractAnimation::resume()
{
    setPaused(false);
}

/*!
    \qmlmethod Animation::stop()
    \brief Stops the animation.

    If the animation is not running, calling this method has no effect.  The
    \c running property will be false following a call to \c stop().

    Normally \c stop() stops the animation immediately, and the animation has
    no further influence on property values.  In this example animation
    \code
    Rectangle {
        x: NumberAnimation { from: 0; to: 100; duration: 500 }
    }
    \endcode
    was stopped at time 250ms, the \c x property will have a value of 50.

    However, if the \c alwaysRunToEnd property is set, the animation will
    continue running until it completes and then stop.  The \c running property
    will still become false immediately.
*/
void QmlAbstractAnimation::stop()
{
    setRunning(false);
}

/*!
    \qmlmethod Animation::restart()
    \brief Restarts the animation.

    This is a convenience method, and is equivalent to calling \c stop() and
    then \c start().
*/
void QmlAbstractAnimation::restart()
{
    stop();
    start();
}

/*!
    \qmlmethod Animation::complete()
    \brief Stops the animation, jumping to the final property values.

    If the animation is not running, calling this method has no effect.  The
    \c running property will be false following a call to \c complete().

    Unlike \c stop(), \c complete() immediately fast-forwards the animation to
    its end.  In the following example,
    \code
    Rectangle {
        x: NumberAnimation { from: 0; to: 100; duration: 500 }
    }
    \endcode
    calling \c stop() at time 250ms will result in the \c x property having
    a value of 50, while calling \c complete() will set the \c x property to
    100, exactly as though the animation had played the whole way through.
*/
void QmlAbstractAnimation::complete()
{
    if (isRunning()) {
         qtAnimation()->setCurrentTime(qtAnimation()->duration());
    }
}

void QmlAbstractAnimation::setTarget(const QmlMetaProperty &p)
{
    Q_D(QmlAbstractAnimation);
    if (d->userProperty.isNull)
        d->userProperty = p;
}

//prepare is called before an animation begins
//(when an animation is used as a simple animation, and not as part of a transition)
void QmlAbstractAnimation::prepare(QmlMetaProperty &)
{
}

void QmlAbstractAnimation::transition(QmlStateActions &actions,
                                      QmlMetaProperties &modified,
                                      TransitionDirection direction)
{
    Q_UNUSED(actions);
    Q_UNUSED(modified);
    Q_UNUSED(direction);
}

void QmlAbstractAnimation::timelineComplete()
{
    Q_D(QmlAbstractAnimation);
    setRunning(false);
    if (d->alwaysRunToEnd && d->repeat) {
        qtAnimation()->setLoopCount(-1);
    }
}

/*!
    \qmlclass PauseAnimation QmlPauseAnimation
    \inherits Animation
    \brief The PauseAnimation provides a pause for an animation.

    When used in a SequentialAnimation, PauseAnimation is a step when
    nothing happens, for a specified duration.

    A 500ms animation sequence, with a 100ms pause between two animations:
    \code
    SequentialAnimation {
        NumberAnimation { ... duration: 200 }
        PauseAnimation { duration: 100 }
        NumberAnimation { ... duration: 200 }
    }
    \endcode
*/
/*!
    \internal
    \class QmlPauseAnimation
*/

QML_DEFINE_TYPE(Qt,4,6,(QT_VERSION&0x00ff00)>>8,PauseAnimation,QmlPauseAnimation)
QmlPauseAnimation::QmlPauseAnimation(QObject *parent)
: QmlAbstractAnimation(*(new QmlPauseAnimationPrivate), parent)
{
    Q_D(QmlPauseAnimation);
    d->init();
}

QmlPauseAnimation::~QmlPauseAnimation()
{
}

void QmlPauseAnimationPrivate::init()
{
    Q_Q(QmlPauseAnimation);
    pa = new QPauseAnimation;
    QmlGraphics_setParent_noEvent(pa, q);
}

/*!
    \qmlproperty int PauseAnimation::duration
    This property holds the duration of the pause in milliseconds

    The default value is 250.
*/
int QmlPauseAnimation::duration() const
{
    Q_D(const QmlPauseAnimation);
    return d->pa->duration();
}

void QmlPauseAnimation::setDuration(int duration)
{
    if (duration < 0) {
        qWarning("QmlPauseAnimation: Cannot set a duration of < 0");
        return;
    }

    Q_D(QmlPauseAnimation);
    if (d->pa->duration() == duration)
        return;
    d->pa->setDuration(duration);
    emit durationChanged(duration);
}

QAbstractAnimation *QmlPauseAnimation::qtAnimation()
{
    Q_D(QmlPauseAnimation);
    return d->pa;
}

/*!
    \qmlclass ColorAnimation QmlColorAnimation
    \inherits PropertyAnimation
    \brief The ColorAnimation allows you to animate color changes.

    \code
    ColorAnimation { from: "white"; to: "#c0c0c0"; duration: 100 }
    \endcode

    When used in a transition, ColorAnimation will by default animate
    all properties of type color that are changing. If a property or properties
    are explicity set for the animation, then those will be used instead.
*/
/*!
    \internal
    \class QmlColorAnimation
*/

QmlColorAnimation::QmlColorAnimation(QObject *parent)
: QmlPropertyAnimation(parent)
{
    Q_D(QmlPropertyAnimation);
    d->interpolatorType = QMetaType::QColor;
    d->interpolator = QVariantAnimationPrivate::getInterpolator(d->interpolatorType);
    d->defaultToInterpolatorType = true;
}

QmlColorAnimation::~QmlColorAnimation()
{
}

/*!
    \qmlproperty color ColorAnimation::from
    This property holds the starting color.
*/
QColor QmlColorAnimation::from() const
{
    Q_D(const QmlPropertyAnimation);
    return d->from.value<QColor>();
}

void QmlColorAnimation::setFrom(const QColor &f)
{
    QmlPropertyAnimation::setFrom(f);
}

/*!
    \qmlproperty color ColorAnimation::from
    This property holds the ending color.
*/
QColor QmlColorAnimation::to() const
{
    Q_D(const QmlPropertyAnimation);
    return d->to.value<QColor>();
}

void QmlColorAnimation::setTo(const QColor &t)
{
    QmlPropertyAnimation::setTo(t);
}

QML_DEFINE_TYPE(Qt,4,6,(QT_VERSION&0x00ff00)>>8,ColorAnimation,QmlColorAnimation)

/*!
    \qmlclass ScriptAction QmlScriptAction
    \inherits Animation
    \brief The ScriptAction allows scripts to be run during an animation.

*/
/*!
    \internal
    \class QmlScriptAction
*/
QmlScriptAction::QmlScriptAction(QObject *parent)
    :QmlAbstractAnimation(*(new QmlScriptActionPrivate), parent)
{
    Q_D(QmlScriptAction);
    d->init();
}

QmlScriptAction::~QmlScriptAction()
{
}

void QmlScriptActionPrivate::init()
{
    Q_Q(QmlScriptAction);
    rsa = new QActionAnimation(&proxy);
    QmlGraphics_setParent_noEvent(rsa, q);
}

/*!
    \qmlproperty script ScriptAction::script
    This property holds the script to run.
*/
QmlScriptString QmlScriptAction::script() const
{
    Q_D(const QmlScriptAction);
    return d->script;
}

void QmlScriptAction::setScript(const QmlScriptString &script)
{
    Q_D(QmlScriptAction);
    d->script = script;
}

/*!
    \qmlproperty QString ScriptAction::stateChangeScriptName
    This property holds the the name of the StateChangeScript to run.

    This property is only valid when ScriptAction is used as part of a transition.
    If both script and stateChangeScriptName are set, stateChangeScriptName will be used.
*/
QString QmlScriptAction::stateChangeScriptName() const
{
    Q_D(const QmlScriptAction);
    return d->name;
}

void QmlScriptAction::setStateChangeScriptName(const QString &name)
{
    Q_D(QmlScriptAction);
    d->name = name;
}

void QmlScriptActionPrivate::execute()
{
    QmlScriptString scriptStr = hasRunScriptScript ? runScriptScript : script;

    const QString &str = scriptStr.script();
    if (!str.isEmpty()) {
        QmlExpression expr(scriptStr.context(), str, scriptStr.scopeObject());
        expr.setTrackChange(false);
        expr.value();
    }
}

void QmlScriptAction::transition(QmlStateActions &actions,
                                    QmlMetaProperties &modified,
                                    TransitionDirection direction)
{
    Q_D(QmlScriptAction);
    Q_UNUSED(modified);
    Q_UNUSED(direction);

    d->hasRunScriptScript = false;
    for (int ii = 0; ii < actions.count(); ++ii) {
        Action &action = actions[ii];

        if (action.event && action.event->typeName() == QLatin1String("StateChangeScript")
            && static_cast<QmlStateChangeScript*>(action.event)->name() == d->name) {
            //### how should we handle reverse direction?
            d->runScriptScript = static_cast<QmlStateChangeScript*>(action.event)->script();
            d->hasRunScriptScript = true;
            action.actionDone = true;
            break;  //assumes names are unique
        }
    }
}

QAbstractAnimation *QmlScriptAction::qtAnimation()
{
    Q_D(QmlScriptAction);
    return d->rsa;
}

QML_DEFINE_TYPE(Qt,4,6,(QT_VERSION&0x00ff00)>>8,ScriptAction,QmlScriptAction)

/*!
    \qmlclass PropertyAction QmlPropertyAction
    \inherits Animation
    \brief The PropertyAction allows immediate property changes during animation.

    Explicitly set \c theimage.smooth=true during a transition:
    \code
    PropertyAction { target: theimage; property: "smooth"; value: true }
    \endcode

    Set \c thewebview.url to the value set for the destination state:
    \code
    PropertyAction { target: thewebview; property: "url" }
    \endcode

    The PropertyAction is immediate -
    the target property is not animated to the selected value in any way.
*/
/*!
    \internal
    \class QmlPropertyAction
*/
QmlPropertyAction::QmlPropertyAction(QObject *parent)
: QmlAbstractAnimation(*(new QmlPropertyActionPrivate), parent)
{
    Q_D(QmlPropertyAction);
    d->init();
}

QmlPropertyAction::~QmlPropertyAction()
{
}

void QmlPropertyActionPrivate::init()
{
    Q_Q(QmlPropertyAction);
    spa = new QActionAnimation;
    QmlGraphics_setParent_noEvent(spa, q);
}

/*!
    \qmlproperty string PropertyAction::properties
    This property holds the properties to be immediately set, comma-separated.
*/
QString QmlPropertyAction::properties() const
{
    Q_D(const QmlPropertyAction);
    return d->properties;
}

void QmlPropertyAction::setProperties(const QString &p)
{
    Q_D(QmlPropertyAction);
    if (d->properties == p)
        return;
    d->properties = p;
    emit propertiesChanged(p);
}

/*!
    \qmlproperty list<Item> PropertyAction::targets
    This property holds the items selected to be affected by this animation (all if not set).
    \sa exclude
*/
QList<QObject *> *QmlPropertyAction::targets()
{
    Q_D(QmlPropertyAction);
    return &d->targets;
}

/*!
    \qmlproperty list<Item> PropertyAction::exclude
    This property holds the items not to be affected by this animation.
    \sa targets
*/
QList<QObject *> *QmlPropertyAction::exclude()
{
    Q_D(QmlPropertyAction);
    return &d->exclude;
}

/*!
    \qmlproperty any PropertyAction::value
    This property holds the value to be set on the property.
    If not set, then the value defined for the end state of the transition.
*/
QVariant QmlPropertyAction::value() const
{
    Q_D(const QmlPropertyAction);
    return d->value;
}

void QmlPropertyAction::setValue(const QVariant &v)
{
    Q_D(QmlPropertyAction);
    if (d->value.isNull || d->value != v) {
        d->value = v;
        emit valueChanged(v);
    }
}

void QmlPropertyActionPrivate::doAction()
{
    property.write(value, QmlMetaProperty::BypassInterceptor | QmlMetaProperty::DontRemoveBinding);
}

QAbstractAnimation *QmlPropertyAction::qtAnimation()
{
    Q_D(QmlPropertyAction);
    return d->spa;
}

void QmlPropertyAction::prepare(QmlMetaProperty &p)
{
    Q_D(QmlPropertyAction);

    if (d->userProperty.isNull)
        d->property = p;
    else
        d->property = d->userProperty;

    d->spa->setAnimAction(&d->proxy, QAbstractAnimation::KeepWhenStopped);
}

void QmlPropertyAction::transition(QmlStateActions &actions,
                                      QmlMetaProperties &modified,
                                      TransitionDirection direction)
{
    Q_D(QmlPropertyAction);
    Q_UNUSED(direction);

    struct QmlSetPropertyAnimationAction : public QAbstractAnimationAction
    {
        QmlStateActions actions;
        virtual void doAction()
        {
            for (int ii = 0; ii < actions.count(); ++ii) {
                const Action &action = actions.at(ii);
                action.property.write(action.toValue, QmlMetaProperty::BypassInterceptor | QmlMetaProperty::DontRemoveBinding);
            }
        }
    };

    QStringList props = d->properties.isEmpty() ? QStringList() : d->properties.split(QLatin1Char(','));
    for (int ii = 0; ii < props.count(); ++ii)
        props[ii] = props.at(ii).trimmed();
    if (!d->propertyName.isEmpty() && !props.contains(d->propertyName))
        props.append(d->propertyName);

    bool targetNeedsReset = false;
    if (d->userProperty.isValid() && props.isEmpty() && !target()) {
        props.append(d->userProperty.value.name());
        d->target = d->userProperty.value.object();
        targetNeedsReset = true;
   }

    QmlSetPropertyAnimationAction *data = new QmlSetPropertyAnimationAction;

    QSet<QObject *> objs;
    for (int ii = 0; ii < actions.count(); ++ii) {
        Action &action = actions[ii];

        QObject *obj = action.property.object();
        QString propertyName = action.property.name();
        QObject *sObj = action.specifiedObject;
        QString sPropertyName = action.specifiedProperty;
        bool same = (obj == sObj);

        if ((d->targets.isEmpty() || d->targets.contains(obj) || (!same && d->targets.contains(sObj))) &&
           (!d->exclude.contains(obj)) && (same || (!d->exclude.contains(sObj))) &&
           (props.contains(propertyName) || (!same && props.contains(sPropertyName))) &&
           (!target() || target() == obj || (!same && target() == sObj))) {
            objs.insert(obj);
            Action myAction = action;

            if (d->value.isValid())
                myAction.toValue = d->value;

            modified << action.property;
            data->actions << myAction;
            action.fromValue = myAction.toValue;
        }
    }

    if (d->value.isValid() && target() && !objs.contains(target())) {
        QObject *obj = target();
        for (int jj = 0; jj < props.count(); ++jj) {
            Action myAction;
            myAction.property = d->createProperty(obj, props.at(jj));
            if (!myAction.property.isValid())
                continue;
            myAction.toValue = d->value;
            data->actions << myAction;
        }
    }

    if (data->actions.count()) {
        d->spa->setAnimAction(data, QAbstractAnimation::DeleteWhenStopped);
    } else {
        delete data;
    }
    if (targetNeedsReset)
        d->target = 0;
}

QML_DEFINE_TYPE(Qt,4,6,(QT_VERSION&0x00ff00)>>8,PropertyAction,QmlPropertyAction)

/*!
    \qmlclass ParentAction QmlParentAction
    \inherits Animation
    \brief The ParentAction allows parent changes during transitions.

    The ParentAction is immediate - it is not animated in any way.
*/

QmlParentAction::QmlParentAction(QObject *parent)
: QmlAbstractAnimation(*(new QmlParentActionPrivate), parent)
{
    Q_D(QmlParentAction);
    d->init();
}

QmlParentAction::~QmlParentAction()
{
}

void QmlParentActionPrivate::init()
{
    Q_Q(QmlParentAction);
    cpa = new QActionAnimation;
    QmlGraphics_setParent_noEvent(cpa, q);
}

QmlGraphicsItem *QmlParentAction::object() const
{
    Q_D(const QmlParentAction);
    return d->pcTarget;
}

void QmlParentAction::setObject(QmlGraphicsItem *target)
{
    Q_D(QmlParentAction);
    d->pcTarget = target;
}

QmlGraphicsItem *QmlParentAction::parent() const
{
    Q_D(const QmlParentAction);
    return d->pcParent;
}

void QmlParentAction::setParent(QmlGraphicsItem *parent)
{
    Q_D(QmlParentAction);
    d->pcParent = parent;
}

void QmlParentActionPrivate::doAction()
{
    QmlParentChange pc;
    pc.setObject(pcTarget);
    pc.setParent(pcParent);
    pc.execute();
}

QAbstractAnimation *QmlParentAction::qtAnimation()
{
    Q_D(QmlParentAction);
    return d->cpa;
}

void QmlParentAction::transition(QmlStateActions &actions,
                                       QmlMetaProperties &modified,
                                       TransitionDirection direction)
{
    Q_D(QmlParentAction);
    Q_UNUSED(modified);
    Q_UNUSED(direction);

    struct QmlParentActionData : public QAbstractAnimationAction
    {
        QmlParentActionData(): pc(0) {}
        ~QmlParentActionData() { delete pc; }

        QmlStateActions actions;
        bool reverse;
        QmlParentChange *pc;
        virtual void doAction()
        {
            for (int ii = 0; ii < actions.count(); ++ii) {
                const Action &action = actions.at(ii);
                if (reverse)
                    action.event->reverse();
                else
                    action.event->execute();
            }
        }
    };

    QmlParentActionData *data = new QmlParentActionData;

    bool explicitMatchFound = false;

    for (int ii = 0; ii < actions.count(); ++ii) {
        Action &action = actions[ii];

        if (action.event && action.event->typeName() == QLatin1String("ParentChange")
            && (!d->target || static_cast<QmlParentChange*>(action.event)->object() == d->target)) {
            Action myAction = action;
            data->reverse = action.reverseEvent;
            if (d->pcParent) {
                QmlParentChange *pc = new QmlParentChange;
                pc->setObject(d->pcTarget);
                pc->setParent(d->pcParent);
                myAction.event = pc;
                data->pc = pc;
                data->actions << myAction;
                if (d->target) explicitMatchFound = true;
                break;  //only match one
            } else {
                action.actionDone = true;
                data->actions << myAction;
            }
        }
    }

    if (!explicitMatchFound && d->pcTarget && d->pcParent) {
        data->reverse = false;
        Action myAction;
        QmlParentChange *pc = new QmlParentChange;
        pc->setObject(d->pcTarget);
        pc->setParent(d->pcParent);
        myAction.event = pc;
        data->pc = pc;
        data->actions << myAction;
    }

    if (data->actions.count()) {
        d->cpa->setAnimAction(data, QAbstractAnimation::DeleteWhenStopped);
    } else {
        delete data;
    }
}

QML_DEFINE_TYPE(Qt,4,6,(QT_VERSION&0x00ff00)>>8,ParentAction,QmlParentAction)

/*!
    \qmlclass NumberAnimation QmlNumberAnimation
    \inherits PropertyAnimation
    \brief The NumberAnimation allows you to animate changes in properties of type qreal.

    Animate a set of properties over 200ms, from their values in the start state to
    their values in the end state of the transition:
    \code
    NumberAnimation { properties: "x,y,scale"; duration: 200 }
    \endcode
*/

/*!
    \internal
    \class QmlNumberAnimation
*/

QmlNumberAnimation::QmlNumberAnimation(QObject *parent)
: QmlPropertyAnimation(parent)
{
    Q_D(QmlPropertyAnimation);
    d->interpolatorType = QMetaType::QReal;
    d->interpolator = QVariantAnimationPrivate::getInterpolator(d->interpolatorType);
}

QmlNumberAnimation::~QmlNumberAnimation()
{
}

/*!
    \qmlproperty real NumberAnimation::from
    This property holds the starting value.
    If not set, then the value defined in the start state of the transition.
*/
qreal QmlNumberAnimation::from() const
{
    Q_D(const QmlPropertyAnimation);
    return d->from.toDouble();
}

void QmlNumberAnimation::setFrom(qreal f)
{
    QmlPropertyAnimation::setFrom(f);
}

/*!
    \qmlproperty real NumberAnimation::to
    This property holds the ending value.
    If not set, then the value defined in the end state of the transition.
*/
qreal QmlNumberAnimation::to() const
{
    Q_D(const QmlPropertyAnimation);
    return d->to.toDouble();
}

void QmlNumberAnimation::setTo(qreal t)
{
    QmlPropertyAnimation::setTo(t);
}

QML_DEFINE_TYPE(Qt,4,6,(QT_VERSION&0x00ff00)>>8,NumberAnimation,QmlNumberAnimation)

QmlAnimationGroup::QmlAnimationGroup(QObject *parent)
: QmlAbstractAnimation(*(new QmlAnimationGroupPrivate), parent)
{
}

QmlAnimationGroup::~QmlAnimationGroup()
{
}

QmlList<QmlAbstractAnimation *> *QmlAnimationGroup::animations()
{
    Q_D(QmlAnimationGroup);
    return &d->animations;
}

/*!
    \qmlclass SequentialAnimation QmlSequentialAnimation
    \inherits Animation
    \brief The SequentialAnimation allows you to run animations sequentially.

    Animations controlled in SequentialAnimation will be run one after the other.

    The following example chains two numeric animations together.  The \c MyItem
    object will animate from its current x position to 100, and then back to 0.

    \code
    SequentialAnimation {
        NumberAnimation { target: MyItem; property: "x"; to: 100 }
        NumberAnimation { target: MyItem; property: "x"; to: 0 }
    }
    \endcode

    \sa ParallelAnimation
*/

QmlSequentialAnimation::QmlSequentialAnimation(QObject *parent) :
    QmlAnimationGroup(parent)
{
    Q_D(QmlAnimationGroup);
    d->ag = new QSequentialAnimationGroup(this);
}

QmlSequentialAnimation::~QmlSequentialAnimation()
{
}

void QmlSequentialAnimation::prepare(QmlMetaProperty &p)
{
    Q_D(QmlAnimationGroup);
    if (d->userProperty.isNull)
        d->property = p;
    else
        d->property = d->userProperty;

    for (int i = 0; i < d->animations.size(); ++i)
        d->animations.at(i)->prepare(d->property);
}

QAbstractAnimation *QmlSequentialAnimation::qtAnimation()
{
    Q_D(QmlAnimationGroup);
    return d->ag;
}

void QmlSequentialAnimation::transition(QmlStateActions &actions,
                                    QmlMetaProperties &modified,
                                    TransitionDirection direction)
{
    Q_D(QmlAnimationGroup);

    int inc = 1;
    int from = 0;
    if (direction == Backward) {
        inc = -1;
        from = d->animations.count() - 1;
    }

    //needed for Behavior
    if (d->userProperty.isValid() && d->propertyName.isEmpty() && !target()) {
        for (int i = 0; i < d->animations.count(); ++i)
            d->animations.at(i)->setTarget(d->userProperty);
   }

    for (int ii = from; ii < d->animations.count() && ii >= 0; ii += inc) {
        d->animations.at(ii)->transition(actions, modified, direction);
    }
}

QML_DEFINE_TYPE(Qt,4,6,(QT_VERSION&0x00ff00)>>8,SequentialAnimation,QmlSequentialAnimation)

/*!
    \qmlclass ParallelAnimation QmlParallelAnimation
    \inherits Animation
    \brief The ParallelAnimation allows you to run animations in parallel.

    Animations contained in ParallelAnimation will be run at the same time.

    The following animation demonstrates animating the \c MyItem item
    to (100,100) by animating the x and y properties in parallel.

    \code
    ParallelAnimation {
        NumberAnimation { target: MyItem; property: "x"; to: 100 }
        NumberAnimation { target: MyItem; property: "y"; to: 100 }
    }
    \endcode

    \sa SequentialAnimation
*/
/*!
    \internal
    \class QmlParallelAnimation
*/

QmlParallelAnimation::QmlParallelAnimation(QObject *parent) :
    QmlAnimationGroup(parent)
{
    Q_D(QmlAnimationGroup);
    d->ag = new QParallelAnimationGroup(this);
}

QmlParallelAnimation::~QmlParallelAnimation()
{
}

void QmlParallelAnimation::prepare(QmlMetaProperty &p)
{
    Q_D(QmlAnimationGroup);
    if (d->userProperty.isNull)
        d->property = p;
    else
        d->property = d->userProperty;

    for (int i = 0; i < d->animations.size(); ++i)
        d->animations.at(i)->prepare(d->property);
}

QAbstractAnimation *QmlParallelAnimation::qtAnimation()
{
    Q_D(QmlAnimationGroup);
    return d->ag;
}

void QmlParallelAnimation::transition(QmlStateActions &actions,
                                      QmlMetaProperties &modified,
                                      TransitionDirection direction)
{
    Q_D(QmlAnimationGroup);

    //needed for Behavior
    if (d->userProperty.isValid() && d->propertyName.isEmpty() && !target()) {
        for (int i = 0; i < d->animations.count(); ++i)
            d->animations.at(i)->setTarget(d->userProperty);
    }

    for (int ii = 0; ii < d->animations.count(); ++ii) {
        d->animations.at(ii)->transition(actions, modified, direction);
    }
}

QML_DEFINE_TYPE(Qt,4,6,(QT_VERSION&0x00ff00)>>8,ParallelAnimation,QmlParallelAnimation)

//convert a variant from string type to another animatable type
void QmlPropertyAnimationPrivate::convertVariant(QVariant &variant, int type)
{
    if (variant.type() != QVariant::String) {
        variant.convert((QVariant::Type)type);
        return;
    }

    switch (type) {
    case QVariant::Rect: {
        variant.setValue(QmlStringConverters::rectFFromString(variant.toString()).toRect());
        break;
    }
    case QVariant::RectF: {
        variant.setValue(QmlStringConverters::rectFFromString(variant.toString()));
        break;
    }
    case QVariant::Point: {
        variant.setValue(QmlStringConverters::pointFFromString(variant.toString()).toPoint());
        break;
    }
    case QVariant::PointF: {
        variant.setValue(QmlStringConverters::pointFFromString(variant.toString()));
        break;
    }
    case QVariant::Size: {
        variant.setValue(QmlStringConverters::sizeFFromString(variant.toString()).toSize());
        break;
    }
    case QVariant::SizeF: {
        variant.setValue(QmlStringConverters::sizeFFromString(variant.toString()));
        break;
    }
    case QVariant::Color: {
        variant.setValue(QmlStringConverters::colorFromString(variant.toString()));
        break;
    }
    case QVariant::Vector3D: {
        variant.setValue(QmlStringConverters::vector3DFromString(variant.toString()));
        break;
    }
    default:
        if ((uint)type >= QVariant::UserType) {
            QmlMetaType::StringConverter converter = QmlMetaType::customStringConverter(type);
            if (converter)
                variant = converter(variant.toString());
        } else
            variant.convert((QVariant::Type)type);
        break;
    }
}

/*!
    \qmlclass PropertyAnimation QmlPropertyAnimation
    \inherits Animation
    \brief The PropertyAnimation allows you to animate property changes.

    Animate a size property over 200ms, from its current size to 20-by-20:
    \code
    VariantAnimation { property: "size"; to: "20x20"; duration: 200 }
    \endcode
*/

QmlPropertyAnimation::QmlPropertyAnimation(QObject *parent)
: QmlAbstractAnimation(*(new QmlPropertyAnimationPrivate), parent)
{
    Q_D(QmlPropertyAnimation);
    d->init();
}

QmlPropertyAnimation::~QmlPropertyAnimation()
{
}

void QmlPropertyAnimationPrivate::init()
{
    Q_Q(QmlPropertyAnimation);
    va = new QmlTimeLineValueAnimator;
    QmlGraphics_setParent_noEvent(va, q);

    va->setStartValue(QVariant(0.0f));
    va->setEndValue(QVariant(1.0f));
}

/*!
    \qmlproperty int PropertyAnimation::duration
    This property holds the duration of the transition, in milliseconds.

    The default value is 250.
*/
int QmlPropertyAnimation::duration() const
{
    Q_D(const QmlPropertyAnimation);
    return d->va->duration();
}

void QmlPropertyAnimation::setDuration(int duration)
{
    if (duration < 0) {
        qWarning("QmlPropertyAnimation: Cannot set a duration of < 0");
        return;
    }

    Q_D(QmlPropertyAnimation);
    if (d->va->duration() == duration)
        return;
    d->va->setDuration(duration);
    emit durationChanged(duration);
}

/*!
    \qmlproperty real PropertyAnimation::from
    This property holds the starting value.
    If not set, then the value defined in the start state of the transition.
*/
QVariant QmlPropertyAnimation::from() const
{
    Q_D(const QmlPropertyAnimation);
    return d->from;
}

void QmlPropertyAnimation::setFrom(const QVariant &f)
{
    Q_D(QmlPropertyAnimation);
    if (d->fromIsDefined && f == d->from)
        return;
    d->from = f;
    d->fromIsDefined = f.isValid();
    emit fromChanged(f);
}

/*!
    \qmlproperty real PropertyAnimation::to
    This property holds the ending value.
    If not set, then the value defined in the end state of the transition.
*/
QVariant QmlPropertyAnimation::to() const
{
    Q_D(const QmlPropertyAnimation);
    return d->to;
}

void QmlPropertyAnimation::setTo(const QVariant &t)
{
    Q_D(QmlPropertyAnimation);
    if (d->toIsDefined && t == d->to)
        return;
    d->to = t;
    d->toIsDefined = t.isValid();
    emit toChanged(t);
}

/*!
    \qmlproperty string PropertyAnimation::easing
    \brief the easing curve used for the transition.

    Available values are:

    \list
    \i \e easeNone - Easing equation function for a simple linear tweening, with no easing.
    \i \e easeInQuad - Easing equation function for a quadratic (t^2) easing in: accelerating from zero velocity.
    \i \e easeOutQuad - Easing equation function for a quadratic (t^2) easing out: decelerating to zero velocity.
    \i \e easeInOutQuad - Easing equation function for a quadratic (t^2) easing in/out: acceleration until halfway, then deceleration.
    \i \e easeOutInQuad - Easing equation function for a quadratic (t^2) easing out/in: deceleration until halfway, then acceleration.
    \i \e easeInCubic - Easing equation function for a cubic (t^3) easing in: accelerating from zero velocity.
    \i \e easeOutCubic - Easing equation function for a cubic (t^3) easing out: decelerating from zero velocity.
    \i \e easeInOutCubic - Easing equation function for a cubic (t^3) easing in/out: acceleration until halfway, then deceleration.
    \i \e easeOutInCubic - Easing equation function for a cubic (t^3) easing out/in: deceleration until halfway, then acceleration.
    \i \e easeInQuart - Easing equation function for a quartic (t^4) easing in: accelerating from zero velocity.
    \i \e easeOutQuart - Easing equation function for a quartic (t^4) easing out: decelerating from zero velocity.
    \i \e easeInOutQuart - Easing equation function for a quartic (t^4) easing in/out: acceleration until halfway, then deceleration.
    \i \e easeOutInQuart - Easing equation function for a quartic (t^4) easing out/in: deceleration until halfway, then acceleration.
    \i \e easeInQuint - Easing equation function for a quintic (t^5) easing in: accelerating from zero velocity.
    \i \e easeOutQuint - Easing equation function for a quintic (t^5) easing out: decelerating from zero velocity.
    \i \e easeInOutQuint - Easing equation function for a quintic (t^5) easing in/out: acceleration until halfway, then deceleration.
    \i \e easeOutInQuint - Easing equation function for a quintic (t^5) easing out/in: deceleration until halfway, then acceleration.
    \i \e easeInSine - Easing equation function for a sinusoidal (sin(t)) easing in: accelerating from zero velocity.
    \i \e easeOutSine - Easing equation function for a sinusoidal (sin(t)) easing out: decelerating from zero velocity.
    \i \e easeInOutSine - Easing equation function for a sinusoidal (sin(t)) easing in/out: acceleration until halfway, then deceleration.
    \i \e easeOutInSine - Easing equation function for a sinusoidal (sin(t)) easing out/in: deceleration until halfway, then acceleration.
    \i \e easeInExpo - Easing equation function for an exponential (2^t) easing in: accelerating from zero velocity.
    \i \e easeOutExpo - Easing equation function for an exponential (2^t) easing out: decelerating from zero velocity.
    \i \e easeInOutExpo - Easing equation function for an exponential (2^t) easing in/out: acceleration until halfway, then deceleration.
    \i \e easeOutInExpo - Easing equation function for an exponential (2^t) easing out/in: deceleration until halfway, then acceleration.
    \i \e easeInCirc - Easing equation function for a circular (sqrt(1-t^2)) easing in: accelerating from zero velocity.
    \i \e easeOutCirc - Easing equation function for a circular (sqrt(1-t^2)) easing out: decelerating from zero velocity.
    \i \e easeInOutCirc - Easing equation function for a circular (sqrt(1-t^2)) easing in/out: acceleration until halfway, then deceleration.
    \i \e easeOutInCirc - Easing equation function for a circular (sqrt(1-t^2)) easing out/in: deceleration until halfway, then acceleration.
    \i \e easeInElastic - Easing equation function for an elastic (exponentially decaying sine wave) easing in: accelerating from zero velocity.  The peak amplitude can be set with the \e amplitude parameter, and the period of decay by the \e period parameter.
    \i \e easeOutElastic - Easing equation function for an elastic (exponentially decaying sine wave) easing out: decelerating from zero velocity.  The peak amplitude can be set with the \e amplitude parameter, and the period of decay by the \e period parameter.
    \i \e easeInOutElastic - Easing equation function for an elastic (exponentially decaying sine wave) easing in/out: acceleration until halfway, then deceleration.
    \i \e easeOutInElastic - Easing equation function for an elastic (exponentially decaying sine wave) easing out/in: deceleration until halfway, then acceleration.
    \i \e easeInBack - Easing equation function for a back (overshooting cubic easing: (s+1)*t^3 - s*t^2) easing in: accelerating from zero velocity.
    \i \e easeOutBack - Easing equation function for a back (overshooting cubic easing: (s+1)*t^3 - s*t^2) easing out: decelerating from zero velocity.
    \i \e easeInOutBack - Easing equation function for a back (overshooting cubic easing: (s+1)*t^3 - s*t^2) easing in/out: acceleration until halfway, then deceleration.
    \i \e easeOutInBack - Easing equation function for a back (overshooting cubic easing: (s+1)*t^3 - s*t^2) easing out/in: deceleration until halfway, then acceleration.
    \i \e easeOutBounce - Easing equation function for a bounce (exponentially decaying parabolic bounce) easing out: decelerating from zero velocity.
    \i \e easeInBounce - Easing equation function for a bounce (exponentially decaying parabolic bounce) easing in: accelerating from zero velocity.
    \i \e easeInOutBounce - Easing equation function for a bounce (exponentially decaying parabolic bounce) easing in/out: acceleration until halfway, then deceleration.
    \i \e easeOutInBounce - Easing equation function for a bounce (exponentially decaying parabolic bounce) easing out/in: deceleration until halfway, then acceleration.
    \endlist
*/
QString QmlPropertyAnimation::easing() const
{
    Q_D(const QmlPropertyAnimation);
    return d->easing;
}

void QmlPropertyAnimation::setEasing(const QString &e)
{
    Q_D(QmlPropertyAnimation);
    if (d->easing == e)
        return;

    d->easing = e;
    d->va->setEasingCurve(stringToCurve(d->easing));
    emit easingChanged(e);
}

/*!
    \qmlproperty string PropertyAnimation::properties
    This property holds the properties this animation should be applied to.

    This is a comma-separated list of properties that should use
    this animation when they change.
*/
QString QmlPropertyAnimation::properties() const
{
    Q_D(const QmlPropertyAnimation);
    return d->properties;
}

void QmlPropertyAnimation::setProperties(const QString &prop)
{
    Q_D(QmlPropertyAnimation);
    if (d->properties == prop)
        return;

    d->properties = prop;
    emit propertiesChanged(prop);
}

/*!
    \qmlproperty list<Item> PropertyAnimation::targets
    This property holds the items selected to be affected by this animation (all if not set).
    \sa exclude
*/
QList<QObject *> *QmlPropertyAnimation::targets()
{
    Q_D(QmlPropertyAnimation);
    return &d->targets;
}

/*!
    \qmlproperty list<Item> PropertyAnimation::exclude
    This property holds the items not to be affected by this animation.
    \sa targets
*/
QList<QObject *> *QmlPropertyAnimation::exclude()
{
    Q_D(QmlPropertyAnimation);
    return &d->exclude;
}

void QmlPropertyAnimationPrivate::valueChanged(qreal r)
{
    if (!fromSourced) {
        if (!fromIsDefined) {
            from = property.read();
            convertVariant(from, interpolatorType ? interpolatorType : property.propertyType());
            //### check for invalid variant if using property type
        }
        fromSourced = true;
    }

    if (r == 1.) {
        property.write(to, QmlMetaProperty::BypassInterceptor | QmlMetaProperty::DontRemoveBinding);
    } else {
        if (interpolator)
            property.write(interpolator(from.constData(), to.constData(), r), QmlMetaProperty::BypassInterceptor | QmlMetaProperty::DontRemoveBinding);
    }
}

QAbstractAnimation *QmlPropertyAnimation::qtAnimation()
{
    Q_D(QmlPropertyAnimation);
    return d->va;
}

void QmlPropertyAnimation::prepare(QmlMetaProperty &p)
{
    Q_D(QmlPropertyAnimation);
    if (d->userProperty.isNull)
        d->property = p;
    else
        d->property = d->userProperty;

    int propType = d->property.propertyType();
    d->convertVariant(d->to, d->interpolatorType ? d->interpolatorType : propType);
    if (d->fromIsDefined)
        d->convertVariant(d->from, d->interpolatorType ? d->interpolatorType : propType);

    if (!d->interpolatorType) {
        //### check for invalid variants
        d->interpolator = QVariantAnimationPrivate::getInterpolator(propType);
    }

    d->fromSourced = false;
    d->value.QmlTimeLineValue::setValue(0.);
    d->va->setAnimValue(&d->value, QAbstractAnimation::KeepWhenStopped);
    d->va->setFromSourcedValue(&d->fromSourced);
}

void QmlPropertyAnimation::transition(QmlStateActions &actions,
                                     QmlMetaProperties &modified,
                                     TransitionDirection direction)
{
    Q_D(QmlPropertyAnimation);
    Q_UNUSED(direction);

    struct PropertyUpdater : public QmlTimeLineValue
    {
        QmlStateActions actions;
        int interpolatorType;       //for Number/ColorAnimation
        int prevInterpolatorType;   //for generic
        QVariantAnimation::Interpolator interpolator;
        bool reverse;
        bool *wasDeleted;
        PropertyUpdater() : wasDeleted(0) {}
        ~PropertyUpdater() { if (wasDeleted) *wasDeleted = true; }
        void setValue(qreal v)
        {
            bool deleted = false;
            wasDeleted = &deleted;
            if (reverse)    //QVariantAnimation sends us 1->0 when reversed, but we are expecting 0->1
                v = 1 - v;
            QmlTimeLineValue::setValue(v);
            for (int ii = 0; ii < actions.count(); ++ii) {
                Action &action = actions[ii];

                if (v == 1.)
                    action.property.write(action.toValue, QmlMetaProperty::BypassInterceptor | QmlMetaProperty::DontRemoveBinding);
                else {
                    if (action.fromValue.isNull()) {
                        action.fromValue = action.property.read();
                        if (interpolatorType)
                            QmlPropertyAnimationPrivate::convertVariant(action.fromValue, interpolatorType);
                    }
                    if (!interpolatorType) {
                        int propType = action.property.propertyType();
                        if (!prevInterpolatorType || prevInterpolatorType != propType) {
                            prevInterpolatorType = propType;
                            interpolator = QVariantAnimationPrivate::getInterpolator(prevInterpolatorType);
                        }
                    }
                    if (interpolator)
                        action.property.write(interpolator(action.fromValue.constData(), action.toValue.constData(), v), QmlMetaProperty::BypassInterceptor | QmlMetaProperty::DontRemoveBinding);
                }
                if (deleted)
                    return;
            }
            wasDeleted = 0;
        }
    };

    QStringList props = d->properties.isEmpty() ? QStringList() : d->properties.split(QLatin1Char(','));
    for (int ii = 0; ii < props.count(); ++ii)
        props[ii] = props.at(ii).trimmed();
    if (!d->propertyName.isEmpty() && !props.contains(d->propertyName))
        props.append(d->propertyName);

    bool useType = (props.isEmpty() && d->defaultToInterpolatorType) ? true : false;

    bool targetNeedsReset = false;
    if (d->userProperty.isValid() && props.isEmpty() && !target()) {
        props.append(d->userProperty.value.name());
        d->target = d->userProperty.value.object();
        targetNeedsReset = true;
    }

    PropertyUpdater *data = new PropertyUpdater;
    data->interpolatorType = d->interpolatorType;
    data->interpolator = d->interpolator;
    data->reverse = direction == Backward ? true : false;

    QSet<QObject *> objs;
    for (int ii = 0; ii < actions.count(); ++ii) {
        Action &action = actions[ii];

        QObject *obj = action.property.object();
        QString propertyName = action.property.name();
        QObject *sObj = action.specifiedObject;
        QString sPropertyName = action.specifiedProperty;
        bool same = (obj == sObj);

        if ((d->targets.isEmpty() || d->targets.contains(obj) || (!same && d->targets.contains(sObj))) &&
           (!d->exclude.contains(obj)) && (same || (!d->exclude.contains(sObj))) &&
           (props.contains(propertyName) || (!same && props.contains(sPropertyName))
               || (useType && action.property.propertyType() == d->interpolatorType)) &&
           (!target() || target() == obj || (!same && target() == sObj))) {
            objs.insert(obj);
            Action myAction = action;

            if (d->fromIsDefined) {
                myAction.fromValue = d->from;
            } else {
                myAction.fromValue = QVariant();
            }
            if (d->toIsDefined)
                myAction.toValue = d->to;

            d->convertVariant(myAction.fromValue, d->interpolatorType ? d->interpolatorType : myAction.property.propertyType());
            d->convertVariant(myAction.toValue, d->interpolatorType ? d->interpolatorType : myAction.property.propertyType());

            modified << action.property;

            data->actions << myAction;
            action.fromValue = myAction.toValue;
        }
    }

    if (d->toIsDefined && target() && !objs.contains(target())) {
        QObject *obj = target();
        for (int jj = 0; jj < props.count(); ++jj) {
            Action myAction;
            myAction.property = d->createProperty(obj, props.at(jj));
            if (!myAction.property.isValid())
                continue;

            if (d->fromIsDefined) {
                d->convertVariant(d->from, d->interpolatorType ? d->interpolatorType : myAction.property.propertyType());
                myAction.fromValue = d->from;
            }
            d->convertVariant(d->to, d->interpolatorType ? d->interpolatorType : myAction.property.propertyType());
            myAction.toValue = d->to;
            data->actions << myAction;
        }
    }

    if (data->actions.count()) {
        d->va->setAnimValue(data, QAbstractAnimation::DeleteWhenStopped);
    } else {
        delete data;
    }
    if (targetNeedsReset)
        d->target = 0;
}

QML_DEFINE_TYPE(Qt,4,6,(QT_VERSION&0x00ff00)>>8,PropertyAnimation,QmlPropertyAnimation)

QT_END_NAMESPACE
