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

#ifndef QMLBINDINGVME_P_H
#define QMLBINDINGVME_P_H

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

#include <QtCore/qglobal.h>
#include <private/qmlbasicscript_p.h>
#include <private/qscriptdeclarativeclass_p.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QObject;
class QmlContextPrivate;
class QmlBindingVME 
{
public:
    struct Config {
        Config() : target(0), targetSlot(-1), subscriptions(0), identifiers(0) {}
        ~Config() { delete [] subscriptions; delete [] identifiers; }
        QObject *target;
        int targetSlot;

        struct Subscription {
            QGuard<QObject> source;
            int notifyIndex;
        };
        Subscription *subscriptions;
        QScriptDeclarativeClass::PersistentIdentifier *identifiers;
    };

    static QByteArray compile(const QmlBasicScript::Expression &, QmlEnginePrivate *);
    static void run(const char *program, 
                    Config *config, QmlContextPrivate *context, 
                    QObject **scopes, QObject **outputs);
    static void dump(const QByteArray &);
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QMLBINDINGVME_P_H

