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

#ifndef QML_H
#define QML_H

#include <QtCore/qbytearray.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qurl.h>
#include <QtCore/qmetaobject.h>
#include <QtDeclarative/qfxglobal.h>
#include <QtDeclarative/qmlmetatype.h>
#include <QtDeclarative/qmlmetaproperty.h>
#include <QtDeclarative/qmlparserstatus.h>
#include <QtDeclarative/qmllist.h>

QT_BEGIN_HEADER

QT_MODULE(Declarative)

#define QML_DECLARE_TYPE(TYPE) \
    Q_DECLARE_METATYPE(TYPE *) \
    Q_DECLARE_METATYPE(QList<TYPE *> *) \
    Q_DECLARE_METATYPE(QmlList<TYPE *> *)

#define QML_DECLARE_TYPE_HASMETATYPE(TYPE) \
    Q_DECLARE_METATYPE(QList<TYPE *> *) \
    Q_DECLARE_METATYPE(QmlList<TYPE *> *)

#define QML_DECLARE_INTERFACE(INTERFACE) \
    QML_DECLARE_TYPE(INTERFACE)

#define QML_DECLARE_INTERFACE_HASMETATYPE(INTERFACE) \
    QML_DECLARE_TYPE_HASMETATYPE(INTERFACE)

QT_BEGIN_NAMESPACE

#define QML_DEFINE_INTERFACE(INTERFACE) \
    template<> QmlPrivate::InstanceType QmlPrivate::Define<INTERFACE *,0,0,0>::instance(qmlRegisterInterface<INTERFACE>(#INTERFACE)); 

#define QML_DEFINE_EXTENDED_TYPE(URI, VERSION_MAJ, VERSION_MIN_FROM, VERSION_MIN_TO, NAME, TYPE, EXTENSION) \
    template<> QmlPrivate::InstanceType QmlPrivate::Define<TYPE *,(VERSION_MAJ), (VERSION_MIN_FROM), (VERSION_MIN_TO)>::instance(qmlRegisterExtendedType<TYPE,EXTENSION>(#URI, VERSION_MAJ, VERSION_MIN_FROM, VERSION_MIN_TO, #NAME, #TYPE));

#define QML_DEFINE_TYPE(URI, VERSION_MAJ, VERSION_MIN_FROM, VERSION_MIN_TO, NAME, TYPE) \
    template<> QmlPrivate::InstanceType QmlPrivate::Define<TYPE *,(VERSION_MAJ), (VERSION_MIN_FROM), (VERSION_MIN_TO)>::instance(qmlRegisterType<TYPE>(#URI, VERSION_MAJ, VERSION_MIN_FROM, VERSION_MIN_TO, #NAME, #TYPE));

#define QML_DEFINE_EXTENDED_NOCREATE_TYPE(TYPE, EXTENSION) \
    template<> QmlPrivate::InstanceType QmlPrivate::Define<TYPE *,0,0,0>::instance(qmlRegisterExtendedType<TYPE,EXTENSION>(#TYPE));

#define QML_DEFINE_NOCREATE_TYPE(TYPE) \
    template<> QmlPrivate::InstanceType QmlPrivate::Define<TYPE *,0,0,0>::instance(qmlRegisterType<TYPE>(#TYPE));

class QmlContext;
class QmlEngine;
Q_DECLARATIVE_EXPORT void qmlExecuteDeferred(QObject *);
Q_DECLARATIVE_EXPORT QmlContext *qmlContext(const QObject *);
Q_DECLARATIVE_EXPORT QmlEngine *qmlEngine(const QObject *);
Q_DECLARATIVE_EXPORT QObject *qmlAttachedPropertiesObjectById(int, const QObject *, bool create = true);

template<typename T>
QObject *qmlAttachedPropertiesObject(const QObject *obj, bool create = true)
{
    // ### is this threadsafe?
    static int idx = -1;

    if (idx == -1)
        idx = QmlMetaType::attachedPropertiesFuncId(&T::staticMetaObject);

    if (idx == -1 || !obj)
        return 0;

    return qmlAttachedPropertiesObjectById(idx, obj, create);
}

QT_END_NAMESPACE

QML_DECLARE_TYPE(QObject)
Q_DECLARE_METATYPE(QVariant)

QT_END_HEADER

#endif // QML_H
