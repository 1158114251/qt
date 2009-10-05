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

#ifndef QMLINTEGERCACHE_P_H
#define QMLINTEGERCACHE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <private/qmlrefcount_p.h>
#include <private/qscriptdeclarativeclass_p.h>
#include <QtCore/qhash.h>

QT_BEGIN_NAMESPACE

class QmlType;
class QmlEngine;
class QmlIntegerCache : public QmlRefCount
{
public:
    QmlIntegerCache(QmlEngine *);
    virtual ~QmlIntegerCache();

    inline int count() const;
    void add(const QString &, int);
    int value(const QString &);
    inline int value(const QScriptDeclarativeClass::Identifier &id) const;

    static QmlIntegerCache *createForEnums(QmlType *, QmlEngine *);
private:
    struct Data : public QScriptDeclarativeClass::PersistentIdentifier {
        Data(const QScriptDeclarativeClass::PersistentIdentifier &i, int v) 
        : QScriptDeclarativeClass::PersistentIdentifier(i), value(v) {}

        int value;
    };

    typedef QHash<QString, Data *> StringCache;
    typedef QHash<QScriptDeclarativeClass::Identifier, Data *> IdentifierCache;

    StringCache stringCache;
    IdentifierCache identifierCache;
    QmlEngine *engine;
};

int QmlIntegerCache::value(const QScriptDeclarativeClass::Identifier &id) const
{
    Data *d = identifierCache.value(id);
    return d?d->value:-1;
}

int QmlIntegerCache::count() const 
{
    return stringCache.count();
}

QT_END_NAMESPACE

#endif // QMLINTEGERCACHE_P_H

