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

#include "private/qobject_p.h"
#include "qmlsystempalette.h"
#include <QApplication>

QT_BEGIN_NAMESPACE

class QmlSystemPalettePrivate : public QObjectPrivate
{
public:
    QPalette palette;
    QPalette::ColorGroup group;
};

QML_DEFINE_TYPE(Qt,4,6,(QT_VERSION&0x00ff00)>>8,SystemPalette,QmlSystemPalette)

/*!
    \qmlclass SystemPalette QmlSystemPalette
    \ingroup group_utility
    \brief The SystemPalette item gives access to the Qt palettes.
    \sa QPalette

    Example:
    \qml
    SystemPalette { id: MyPalette; colorGroup: Qt.Active }

    Rectangle {
        width: 640; height: 480
        color: MyPalette.window
        Text {
            anchors.fill: parent
            text: "Hello!"; color: MyPalette.windowText
        }
    }
    \endqml
*/
QmlSystemPalette::QmlSystemPalette(QObject *parent)
    : QObject(*(new QmlSystemPalettePrivate), parent)
{
    Q_D(QmlSystemPalette);
    d->palette = qApp->palette();
    d->group = QPalette::Active;
    qApp->installEventFilter(this);
}

QmlSystemPalette::~QmlSystemPalette()
{
}

/*!
    \qmlproperty color SystemPalette::window
    The window (general background) color of the current color group.

    \sa QPalette::ColorRole
*/
QColor QmlSystemPalette::window() const
{
    Q_D(const QmlSystemPalette);
    return d->palette.color(d->group, QPalette::Window);
}

/*!
    \qmlproperty color SystemPalette::windowText
    The window text (general foreground) color of the current color group.

    \sa QPalette::ColorRole
*/
QColor QmlSystemPalette::windowText() const
{
    Q_D(const QmlSystemPalette);
    return d->palette.color(d->group, QPalette::WindowText);
}

/*!
    \qmlproperty color SystemPalette::base
    The base color of the current color group.

    \sa QPalette::ColorRole
*/
QColor QmlSystemPalette::base() const
{
    Q_D(const QmlSystemPalette);
    return d->palette.color(d->group, QPalette::Base);
}

/*!
    \qmlproperty color SystemPalette::text
    The text color of the current color group.

    \sa QPalette::ColorRole
*/
QColor QmlSystemPalette::text() const
{
    Q_D(const QmlSystemPalette);
    return d->palette.color(d->group, QPalette::Text);
}

/*!
    \qmlproperty color SystemPalette::alternateBase
    The alternate base color of the current color group.

    \sa QPalette::ColorRole
*/
QColor QmlSystemPalette::alternateBase() const
{
    Q_D(const QmlSystemPalette);
    return d->palette.color(d->group, QPalette::AlternateBase);
}

/*!
    \qmlproperty color SystemPalette::button
    The button color of the current color group.

    \sa QPalette::ColorRole
*/
QColor QmlSystemPalette::button() const
{
    Q_D(const QmlSystemPalette);
    return d->palette.color(d->group, QPalette::Button);
}

/*!
    \qmlproperty color SystemPalette::buttonText
    The button text foreground color of the current color group.

    \sa QPalette::ColorRole
*/
QColor QmlSystemPalette::buttonText() const
{
    Q_D(const QmlSystemPalette);
    return d->palette.color(d->group, QPalette::ButtonText);
}

/*!
    \qmlproperty color SystemPalette::light
    The light color of the current color group.

    \sa QPalette::ColorRole
*/
QColor QmlSystemPalette::light() const
{
    Q_D(const QmlSystemPalette);
    return d->palette.color(d->group, QPalette::Light);
}

/*!
    \qmlproperty color SystemPalette::midlight
    The midlight color of the current color group.

    \sa QPalette::ColorRole
*/
QColor QmlSystemPalette::midlight() const
{
    Q_D(const QmlSystemPalette);
    return d->palette.color(d->group, QPalette::Midlight);
}

/*!
    \qmlproperty color SystemPalette::dark
    The dark color of the current color group.

    \sa QPalette::ColorRole
*/
QColor QmlSystemPalette::dark() const
{
    Q_D(const QmlSystemPalette);
    return d->palette.color(d->group, QPalette::Dark);
}

/*!
    \qmlproperty color SystemPalette::mid
    The mid color of the current color group.

    \sa QPalette::ColorRole
*/
QColor QmlSystemPalette::mid() const
{
    Q_D(const QmlSystemPalette);
    return d->palette.color(d->group, QPalette::Mid);
}

/*!
    \qmlproperty color SystemPalette::shadow
    The shadow color of the current color group.

    \sa QPalette::ColorRole
*/
QColor QmlSystemPalette::shadow() const
{
    Q_D(const QmlSystemPalette);
    return d->palette.color(d->group, QPalette::Shadow);
}

/*!
    \qmlproperty color SystemPalette::highlight
    The highlight color of the current color group.

    \sa QPalette::ColorRole
*/
QColor QmlSystemPalette::highlight() const
{
    Q_D(const QmlSystemPalette);
    return d->palette.color(d->group, QPalette::Highlight);
}

/*!
    \qmlproperty color SystemPalette::highlightedText
    The highlighted text color of the current color group.

    \sa QPalette::ColorRole
*/
QColor QmlSystemPalette::highlightedText() const
{
    Q_D(const QmlSystemPalette);
    return d->palette.color(d->group, QPalette::HighlightedText);
}

/*!
    \qmlproperty color SystemPalette::lighter
*/
QColor QmlSystemPalette::lighter(const QColor& color) const
{
    return color.lighter();
}

/*!
    \qmlproperty color SystemPalette::darker
*/
QColor QmlSystemPalette::darker(const QColor& color) const
{
    return color.darker();
}

/*!
    \qmlproperty QPalette::ColorGroup SystemPalette::colorGroup

    The color group of the palette. It can be Active, Inactive or Disabled.
    Active is the default.

    \sa QPalette::ColorGroup
*/
QPalette::ColorGroup QmlSystemPalette::colorGroup() const
{
    Q_D(const QmlSystemPalette);
    return d->group;
}

void QmlSystemPalette::setColorGroup(QPalette::ColorGroup colorGroup)
{
    Q_D(QmlSystemPalette);
    d->group = colorGroup;
    emit paletteChanged();
}

bool QmlSystemPalette::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == qApp) {
        if (event->type() == QEvent::ApplicationPaletteChange) {
            QApplication::postEvent(this, new QEvent(QEvent::ApplicationPaletteChange));
            return false;
        }
    }
    return QObject::eventFilter(watched, event);
}

bool QmlSystemPalette::event(QEvent *event)
{
    Q_D(QmlSystemPalette);
    if (event->type() == QEvent::ApplicationPaletteChange) {
        d->palette = qApp->palette();
        emit paletteChanged();
        return true;
    }
    return QObject::event(event);
}

QT_END_NAMESPACE
