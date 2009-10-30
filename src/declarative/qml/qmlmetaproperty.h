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

#ifndef QMLMETAPROPERTY_H
#define QMLMETAPROPERTY_H

#include <QtCore/qmetaobject.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)

class QObject;
class QmlAbstractBinding;
class QmlExpression;
class QStringList;
class QVariant;
struct QMetaObject;
class QmlContext;
class QmlEngine;

class QmlMetaPropertyPrivate;
class Q_DECLARATIVE_EXPORT QmlMetaProperty
{
public:
    enum PropertyCategory {
        Unknown,
        InvalidProperty,
        Bindable,
        List,
        QmlList,    //XXX
        Object,
        Normal
    };
    QmlMetaProperty();
    QmlMetaProperty(QObject *);
    QmlMetaProperty(QObject *, const QString &);
    QmlMetaProperty(QObject *, QmlContext *);
    QmlMetaProperty(QObject *, const QString &, QmlContext *);
    QmlMetaProperty(const QmlMetaProperty &);
    QmlMetaProperty &operator=(const QmlMetaProperty &);
    QmlMetaProperty(QObject *, int, QmlContext * = 0);
    ~QmlMetaProperty();

    static QStringList properties(QObject *);
    QString name() const;

    QVariant read() const;
    bool write(const QVariant &) const;
    enum WriteFlag { BypassInterceptor = 0x01, DontRemoveBinding = 0x02 };
    Q_DECLARE_FLAGS(WriteFlags, WriteFlag)
    bool write(const QVariant &, QmlMetaProperty::WriteFlags) const;

    bool hasChangedNotifier() const;
    bool needsChangedNotifier() const;
    bool connectNotifier(QObject *dest, const char *slot) const;
    bool connectNotifier(QObject *dest, int method) const;

    quint32 save() const;
    void restore(quint32, QObject *, QmlContext * = 0);

    QMetaMethod method() const;

    enum Type { Invalid = 0x00, 
                Property = 0x01, 
                SignalProperty = 0x02,
                Default = 0x08,
                Attached = 0x10,
                ValueTypeProperty = 0x20 };

    Type type() const;
    bool isProperty() const;
    bool isDefault() const;
    bool isWritable() const;
    bool isDesignable() const;
    bool isValid() const;
    QObject *object() const;

    PropertyCategory propertyCategory() const;

    int propertyType() const;
    const char *propertyTypeName() const;

    bool operator==(const QmlMetaProperty &) const;

    QMetaProperty property() const;

    QmlAbstractBinding *binding() const;
    QmlAbstractBinding *setBinding(QmlAbstractBinding *,
                                   QmlMetaProperty::WriteFlags flags = QmlMetaProperty::DontRemoveBinding) const;

    QmlExpression *signalExpression() const;
    QmlExpression *setSignalExpression(QmlExpression *) const;

    static QmlMetaProperty createProperty(QObject *, const QString &);

    int coreIndex() const;
    int valueTypeCoreIndex() const;
private:
    friend class QmlEnginePrivate;
    QmlMetaPropertyPrivate *d;
};
typedef QList<QmlMetaProperty> QmlMetaProperties;
 Q_DECLARE_OPERATORS_FOR_FLAGS(QmlMetaProperty::WriteFlags)

QT_END_NAMESPACE

QT_END_HEADER

#endif // QMLMETAPROPERTY_H
