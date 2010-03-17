/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
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

#include "qelapsedtimer.h"
#include "qdatetime.h"

QT_BEGIN_NAMESPACE

bool QElapsedTimer::isMonotonic()
{
    return false;
}

void QElapsedTimer::start()
{
    QTime t = QTime::currentTime();
    t1 = t.mds;
    t2 = 0;
}

qint64 QElapsedTimer::restart()
{
    QTime t = QTime::currentTime();
    qint64 old = t1;
    t1 = t.mds;
    return t1 - old;
}

qint64 QElapsedTimer::elapsed() const
{
    QTime t = QTime::currentTime();
    return t.mds - t1;
}

qint64 QElapsedTimer::msecsTo(const QElapsedTimer &other) const
{
    qint64 diff = other.t1 - t1;
    if (diff < 0)             // passed midnight
        diff += 86400 * 1000;
    return diff;
}

qint64 QElapsedTimer::secsTo(const QElapsedTimer &other) const
{
    return msecsTo(other) / 1000;
}

bool operator<(const QElapsedTimer &v1, const QElapsedTimer &v2)
{
    return v1.t1 < v2.t1;
}

QT_END_NAMESPACE
