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

#include "qmlpixmapcache_p.h"

#include "qfxperf_p_p.h"

#include <qmlengine.h>

#include <QImageReader>
#include <QHash>
#include <QNetworkReply>
#include <QPixmapCache>
#include <QFile>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

class QSharedNetworkReply;
typedef QHash<QString, QSharedNetworkReply *> QmlGraphicsSharedNetworkReplyHash;
static QmlGraphicsSharedNetworkReplyHash qfxActiveNetworkReplies;

class QSharedNetworkReply
{
public:
    QSharedNetworkReply(QNetworkReply *r) : reply(r), refCount(1) {}
    ~QSharedNetworkReply()
    {
        reply->deleteLater();
    }
    QNetworkReply *reply;
    QPixmap pixmap; // ensure reference to pixmap to QPixmapCache does not discard

    int refCount;
    void addRef()
    {
        ++refCount;
    }
    void release()
    {
        Q_ASSERT(refCount > 0);
        --refCount;
        if (refCount == 0) {
            QString key = reply->url().toString();
            qfxActiveNetworkReplies.remove(key);
            delete this;
        }
    }
};

static bool readImage(QIODevice *dev, QPixmap *pixmap)
    {
        QImageReader imgio(dev);

//#define QT_TEST_SCALED_SIZE
#ifdef QT_TEST_SCALED_SIZE
        /*
        Some mechanism is needed for loading images at a limited size, especially
        for remote images. Loading only thumbnails of remote progressive JPEG
        images can be efficient. (Qt jpeg handler does not do so currently)
        */

        QSize limit(60,60);
        QSize sz = imgio.size();
        if (sz.width() > limit.width() || sz.height() > limit.height()) {
            sz.scale(limit,Qt::KeepAspectRatio);
            imgio.setScaledSize(sz);
        }
#endif

        QImage img;
        if (imgio.read(&img)) {
#ifdef QT_TEST_SCALED_SIZE
            if (!sz.isValid())
                img = img.scaled(limit,Qt::KeepAspectRatio);
#endif
            *pixmap = QPixmap::fromImage(img);
            return true;
        } else {
            qWarning() << imgio.errorString();
            return false;
        }
    }

/*!
    \internal
    \class QmlPixmapCache
    \brief Enacapsultes a pixmap for QmlGraphics items.

    This class is NOT reentrant.
 */

static QString toLocalFileOrQrc(const QUrl& url)
{
    QString r = url.toLocalFile();
    if (r.isEmpty() && url.scheme() == QLatin1String("qrc"))
        r = QLatin1Char(':') + url.path();
    return r;
}

/*!
    Finds the cached pixmap corresponding to \a url.
    A previous call to get() must have requested the URL,
    and the QNetworkReply must have finished before calling
    this function.

    Returns true if the image was loaded without error.
*/
bool QmlPixmapCache::find(const QUrl& url, QPixmap *pixmap)
{
#ifdef Q_ENABLE_PERFORMANCE_LOG
    QmlPerfTimer<QmlPerf::PixmapLoad> perf;
#endif

    QString key = url.toString();
    bool ok = true;
    if (!QPixmapCache::find(key,pixmap)) {
#ifndef QT_NO_LOCALFILE_OPTIMIZED_QML
        QString lf = toLocalFileOrQrc(url);
        if (!lf.isEmpty()) {
            QFile f(lf);
            if (f.open(QIODevice::ReadOnly)) {
                if (!readImage(&f, pixmap)) {
                    qWarning() << "Format error loading" << url;
                    *pixmap = QPixmap();
                    ok = false;
                }
            } else {
                *pixmap = QPixmap();
                ok = false;
            }
        } else
#endif
        {
            QmlGraphicsSharedNetworkReplyHash::Iterator iter = qfxActiveNetworkReplies.find(key);
            if (iter == qfxActiveNetworkReplies.end()) {
                // API usage error
                qWarning() << "QmlPixmapCache: URL not loaded" << url;
                ok = false;
            } else {
                if ((*iter)->reply->error()) {
                    qWarning() << "Network error loading" << url << (*iter)->reply->errorString();
                    *pixmap = QPixmap();
                    ok = false;
                } else if (!readImage((*iter)->reply, pixmap)) {
                    qWarning() << "Format error loading" << url;
                    *pixmap = QPixmap();
                    ok = false;
                } else {
                    if ((*iter)->refCount > 1)
                        (*iter)->pixmap = *pixmap;
                }
                (*iter)->release();
            }
        }
        QPixmapCache::insert(key, *pixmap);
    } else {
        ok = !pixmap->isNull();
#ifndef QT_NO_LOCALFILE_OPTIMIZED_QML
        if (url.scheme()!=QLatin1String("file"))
#endif
        // We may be the second finder. Still need to check for active replies.
        {
            QmlGraphicsSharedNetworkReplyHash::Iterator iter = qfxActiveNetworkReplies.find(key);
            if (iter != qfxActiveNetworkReplies.end())
                (*iter)->release();
        }
    }
    return ok;
}

/*!
    Starts a network request to load \a url.

    Returns a QNetworkReply if the image is not immediately available, otherwise
    returns 0.  Caller should connect to QNetworkReply::finished() to then call
    find() when the image is available.

    The returned QNetworkReply will be deleted when all get() calls are
    matched by a corresponding find() call.

    If the \a ok parameter is passed and \a url is a local file,
    its value will be set to false if the pixmap could not be loaded;
    otherwise the pixmap was loaded and *ok will be true.
*/
QNetworkReply *QmlPixmapCache::get(QmlEngine *engine, const QUrl& url, QPixmap *pixmap, bool *ok)
{
#ifndef QT_NO_LOCALFILE_OPTIMIZED_QML
    QString lf = toLocalFileOrQrc(url);
    if (!lf.isEmpty()) {
        QString key = url.toString();
        if (!QPixmapCache::find(key,pixmap)) {
            bool loaded = true;
            QFile f(lf);
            if (f.open(QIODevice::ReadOnly)) {
                if (!readImage(&f, pixmap)) {
                    qWarning() << "Format error loading" << url;
                    *pixmap = QPixmap();
                    loaded = false;
                }
            } else {
                qWarning() << "Cannot open" << url;
                *pixmap = QPixmap();
                loaded = false;
            }
            if (loaded)
                QPixmapCache::insert(key, *pixmap);
            if (ok) *ok = loaded;
        }
        return 0;
    }
#endif

    QString key = url.toString();
    if (QPixmapCache::find(key,pixmap)) {
        return 0;
    }

    QmlGraphicsSharedNetworkReplyHash::Iterator iter = qfxActiveNetworkReplies.find(key);
    if (iter == qfxActiveNetworkReplies.end()) {
        QNetworkRequest req(url);
        QSharedNetworkReply *item = new QSharedNetworkReply(engine->networkAccessManager()->get(req));
        iter = qfxActiveNetworkReplies.insert(key, item);
    } else {
        (*iter)->addRef();
    }

    return (*iter)->reply;
}

/*!
    Cancels a previous call to get().

    May also cancel loading (eg. if no other pending request).

    Any connections from the QNetworkReply returned by get() to \a obj will be
    disconnected.
*/
void QmlPixmapCache::cancelGet(const QUrl& url, QObject* obj)
{
    QString key = url.toString();
    QmlGraphicsSharedNetworkReplyHash::Iterator iter = qfxActiveNetworkReplies.find(key);
    if (iter == qfxActiveNetworkReplies.end())
        return;
    if (obj)
        QObject::disconnect((*iter)->reply, 0, obj, 0);
    (*iter)->release();
}

/*!
    This function is mainly for test verification. It returns the number of
    requests that are still unfinished.
*/
int QmlPixmapCache::pendingRequests()
{
    return qfxActiveNetworkReplies.count();
}

QT_END_NAMESPACE
