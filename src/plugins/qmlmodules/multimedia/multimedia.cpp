/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the plugins of the Qt Toolkit.
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

#include <QtDeclarative/qmlmoduleplugin.h>
#include <QtDeclarative/qml.h>
#include <QtMultimedia/private/qsoundeffect_p.h>
#include <QtMultimedia/private/qmlaudio_p.h>
#include <QtMultimedia/private/qmlgraphicsvideo_p.h>

QT_BEGIN_NAMESPACE

class QMultimediaQmlModule : public QmlModulePlugin
{
    Q_OBJECT
public:
    QStringList keys() const
    {
        return QStringList() << QLatin1String("qt.multimedia");
    }

    void defineModule(const QString& uri)
    {
        Q_UNUSED(uri)
        Q_ASSERT(uri == QLatin1String("qt.multimedia"));

        qmlRegisterType<QSoundEffect>("qt.multimedia", 4, 7, "SoundEffect", "SoundEffect");
        qmlRegisterType<QmlAudio>("qt.multimedia", 4, 7, "Audio", "Audio");
        qmlRegisterType<QmlGraphicsVideo>("qt.multimedia", 4, 7, "Video", "Video");
    }
};

QT_END_NAMESPACE

#include "multimedia.moc"

Q_EXPORT_PLUGIN2(qmultimediaqmlmodule, QT_PREPEND_NAMESPACE(QMultimediaQmlModule));

