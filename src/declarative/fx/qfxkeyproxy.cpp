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

#include "qfxkeyproxy.h"
#include <QGraphicsScene>
#include <QKeyEvent>

QT_BEGIN_NAMESPACE

QML_DEFINE_TYPE(QFxKeyProxy,KeyProxy)

/*!
    \qmlclass KeyProxy
    \brief The KeyProxy item proxies key presses to a number of other items.
    \inherits Item

*/

/*!
    \internal
    \class QFxKeyProxy
    \brief The QFxKeyProxy class proxies key presses to a number of other items.
    \ingroup group_utility
*/

class QFxKeyProxyPrivate
{
public:
    QFxKeyProxyPrivate() : inPress(false), inRelease(false), inIM(false) {}
    QList<QFxItem *> targets;

    //loop detection
    bool inPress:1;
    bool inRelease:1;
    bool inIM:1;
};

QFxKeyProxy::QFxKeyProxy(QFxItem *parent)
: QFxItem(parent), d(new QFxKeyProxyPrivate)
{
    setOptions(AcceptsInputMethods);
}

QFxKeyProxy::~QFxKeyProxy()
{
    delete d; d = 0;
}

/*!
    \qmlproperty list<Item> KeyProxy::targets

    The proxy targets.
*/

/*!
    \property QFxKeyProxy::targets
    \brief the proxy targets.
*/

QList<QFxItem *> *QFxKeyProxy::targets() const
{
    return &d->targets;
}

void QFxKeyProxy::keyPressEvent(QKeyEvent *e)
{
    if (!d->inPress) {
        d->inPress = true;
        for (int ii = 0; ii < d->targets.count(); ++ii) {
            QFxItem *i = d->targets.at(ii); //### FocusRealm
            if (i && scene())
                scene()->sendEvent(i, e);
            if (e->isAccepted()) {
                d->inPress = false;
                return;
            }
        }
        d->inPress = false;
    }
}

void QFxKeyProxy::keyReleaseEvent(QKeyEvent *e)
{
    if (!d->inRelease) {
        d->inRelease = true;
        for (int ii = 0; ii < d->targets.count(); ++ii) {
            QFxItem *i = d->targets.at(ii); //### FocusRealm
            if (i && scene())
                scene()->sendEvent(i, e);
            if (e->isAccepted()) {
                d->inRelease = false;
                return;
            }
        }
        d->inRelease = false;
    }
}

void QFxKeyProxy::inputMethodEvent(QInputMethodEvent *e)
{
    if (!d->inIM) {
        d->inIM = true;
        for (int ii = 0; ii < d->targets.count(); ++ii) {
            QFxItem *i = d->targets.at(ii); //### FocusRealm
            if (i && scene())
                scene()->sendEvent(i, e);
            if (e->isAccepted()) {
                d->inIM = false;
                return;
            }
        }
        d->inIM = false;
    }
}

QT_END_NAMESPACE
