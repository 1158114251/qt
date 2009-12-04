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

#include "qmlfontloader_p.h"

#include <qmlcontext.h>
#include <qmlengine.h>

#include <QUrl>
#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QFontDatabase>

#include <private/qobject_p.h>

QT_BEGIN_NAMESPACE

class QmlFontLoaderPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QmlFontLoader)

public:
    QmlFontLoaderPrivate() : reply(0), status(QmlFontLoader::Null) {}

    void addFontToDatabase(const QByteArray &);

    QUrl url;
    QString name;
    QNetworkReply *reply;
    QmlFontLoader::Status status;
};

QML_DEFINE_TYPE(Qt,4,6,FontLoader,QmlFontLoader)

/*!
    \qmlclass FontLoader QmlFontLoader
    \ingroup group_utility
    \brief This item allows using fonts by name or url.

    Example:
    \qml
    FontLoader { id: fixedFont; name: "Courier" }
    FontLoader { id: webFont; source: "http://www.mysite.com/myfont.ttf" }

    Text { text: "Fixed-size font"; font.family: fixedFont.name }
    Text { text: "Fancy font"; font.family: webFont.name }
    \endqml
*/
QmlFontLoader::QmlFontLoader(QObject *parent)
    : QObject(*(new QmlFontLoaderPrivate), parent)
{
}

QmlFontLoader::~QmlFontLoader()
{
}

static QString toLocalFileOrQrc(const QUrl& url)
{
    QString r = url.toLocalFile();
    if (r.isEmpty() && url.scheme() == QLatin1String("qrc"))
        r = QLatin1Char(':') + url.path();
    return r;
}


/*!
    \qmlproperty url FontLoader::source
    The url of the font to load.
*/
QUrl QmlFontLoader::source() const
{
    Q_D(const QmlFontLoader);
    return d->url;
}

void QmlFontLoader::setSource(const QUrl &url)
{
    Q_D(QmlFontLoader);
    if (url == d->url)
        return;
    d->url = qmlContext(this)->resolvedUrl(url);

    d->status = Loading;
    emit statusChanged();
#ifndef QT_NO_LOCALFILE_OPTIMIZED_QML
    QString lf = toLocalFileOrQrc(d->url);
    if (!lf.isEmpty()) {
        QFile file(lf);
        file.open(QIODevice::ReadOnly);
        QByteArray ba = file.readAll();
        d->addFontToDatabase(ba);
    } else
#endif
    {
        QNetworkRequest req(d->url);
        d->reply = qmlEngine(this)->networkAccessManager()->get(req);
        QObject::connect(d->reply, SIGNAL(finished()), this, SLOT(replyFinished()));
    }
}

/*!
    \qmlproperty string FontLoader::name

    This property holds the name of the font family.
    It is set automatically when a font is loaded using the \c url property.

    Use this to set the \c font.family property of a \c Text item.

    Example:
    \qml
    FontLoader { id: webFont; source: "http://www.mysite.com/myfont.ttf" }
    Text { text: "Fancy font"; font.family: webFont.name }
    \endqml
*/
QString QmlFontLoader::name() const
{
    Q_D(const QmlFontLoader);
    return d->name;
}

void QmlFontLoader::setName(const QString &name)
{
    Q_D(QmlFontLoader);
    if (d->name == name )
        return;
    d->name = name;
    emit nameChanged();
    d->status = Ready;
    emit statusChanged();
}

/*!
    \qmlproperty enum FontLoader::status

    This property holds the status of font loading.  It can be one of:
    \list
    \o Null - no font has been set
    \o Ready - the font has been loaded
    \o Loading - the font is currently being loaded
    \o Error - an error occurred while loading the font
    \endlist
*/
QmlFontLoader::Status QmlFontLoader::status() const
{
    Q_D(const QmlFontLoader);
    return d->status;
}

void QmlFontLoader::replyFinished()
{
    Q_D(QmlFontLoader);
    if (d->reply) {
        if (!d->reply->error()) {
            QByteArray ba = d->reply->readAll();
            d->addFontToDatabase(ba);
        } else {
            d->status = Error;
            emit statusChanged();
        }
        d->reply->deleteLater();
        d->reply = 0;
    }
}

void QmlFontLoaderPrivate::addFontToDatabase(const QByteArray &ba)
{
    Q_Q(QmlFontLoader);

    int id = QFontDatabase::addApplicationFontFromData(ba);
    if (id != -1) {
        name = QFontDatabase::applicationFontFamilies(id).at(0);
        emit q->nameChanged();
        status = QmlFontLoader::Ready;
    } else {
        status = QmlFontLoader::Error;
        qWarning() << "Cannot load font:" << url;
    }
    emit q->statusChanged();
}

QT_END_NAMESPACE
