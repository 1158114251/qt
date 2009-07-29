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
#include "qmlpalette.h"
#include <QApplication>

QT_BEGIN_NAMESPACE

class QmlPalettePrivate : public QObjectPrivate
{
public:
    QPalette palette;
    QPalette::ColorGroup group;
};

QML_DEFINE_TYPE(Qt,4,6,(QT_VERSION&0x00ff00)>>8,Palette,QmlPalette)

/*!
    \internal
    \class QmlPalette
    \ingroup group_utility
    \brief The QmlPalette class gives access to the Qt palettes.
*/
QmlPalette::QmlPalette(QObject *parent)
    : QObject(*(new QmlPalettePrivate), parent)
{
    Q_D(QmlPalette);
    d->palette = qApp->palette();
    d->group = QPalette::Active;
    qApp->installEventFilter(this);
}

QmlPalette::~QmlPalette()
{
}

QColor QmlPalette::window() const
{
    Q_D(const QmlPalette);
    return d->palette.color(d->group, QPalette::Window);
}

QColor QmlPalette::windowText() const
{
    Q_D(const QmlPalette);
    return d->palette.color(d->group, QPalette::WindowText);
}

QColor QmlPalette::base() const
{
    Q_D(const QmlPalette);
    return d->palette.color(d->group, QPalette::Base);
}

QColor QmlPalette::alternateBase() const
{
    Q_D(const QmlPalette);
    return d->palette.color(d->group, QPalette::AlternateBase);
}

QColor QmlPalette::button() const
{
    Q_D(const QmlPalette);
    return d->palette.color(d->group, QPalette::Button);
}

QColor QmlPalette::buttonText() const
{
    Q_D(const QmlPalette);
    return d->palette.color(d->group, QPalette::ButtonText);
}

QColor QmlPalette::light() const
{
    Q_D(const QmlPalette);
    return d->palette.color(d->group, QPalette::Light);
}

QColor QmlPalette::midlight() const
{
    Q_D(const QmlPalette);
    return d->palette.color(d->group, QPalette::Midlight);
}

QColor QmlPalette::dark() const
{
    Q_D(const QmlPalette);
    return d->palette.color(d->group, QPalette::Dark);
}

QColor QmlPalette::mid() const
{
    Q_D(const QmlPalette);
    return d->palette.color(d->group, QPalette::Mid);
}

QColor QmlPalette::shadow() const
{
    Q_D(const QmlPalette);
    return d->palette.color(d->group, QPalette::Shadow);
}

QColor QmlPalette::highlight() const
{
    Q_D(const QmlPalette);
    return d->palette.color(d->group, QPalette::Highlight);
}

QColor QmlPalette::highlightedText() const
{
    Q_D(const QmlPalette);
    return d->palette.color(d->group, QPalette::HighlightedText);
}

QColor QmlPalette::lighter(const QColor& color) const
{
    return color.lighter();
}

QColor QmlPalette::darker(const QColor& color) const
{
    return color.darker();
}

QmlPalette::ColorGroup QmlPalette::colorGroup() const
{
    Q_D(const QmlPalette);
    return (QmlPalette::ColorGroup)int(d->group);
}

void QmlPalette::setColorGroup(ColorGroup colorGroup)
{
    Q_D(QmlPalette);
    d->group = (QPalette::ColorGroup)int(colorGroup);
    emit paletteChanged();
}

QPalette QmlPalette::palette() const
{
    Q_D(const QmlPalette);
    return d->palette;
}

bool QmlPalette::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == qApp) {
        if (event->type() == QEvent::ApplicationPaletteChange) {
            QApplication::postEvent(this, new QEvent(QEvent::ApplicationPaletteChange));
            return false;
        }
    }
    return QObject::eventFilter(watched, event);
}

bool QmlPalette::event(QEvent *event)
{
    Q_D(QmlPalette);
    if (event->type() == QEvent::ApplicationPaletteChange) {
        d->palette = qApp->palette();
        emit paletteChanged();
        return true;
    }
    return QObject::event(event);
}

QT_END_NAMESPACE
