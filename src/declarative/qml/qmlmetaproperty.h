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

#ifndef QMLMETAPROPERTY_H
#define QMLMETAPROPERTY_H

#include <QtDeclarative/qfxglobal.h>
#include <QMetaProperty>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)

class QObject;
class QmlBindableValue;
class QStringList;
class QVariant;
struct QMetaObject;
class QmlContext;

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
    QmlMetaProperty(QObject *, int, PropertyCategory = Unknown, QmlContext * = 0);
    ~QmlMetaProperty();

    static QStringList properties(QObject *);
    QString name() const;

    QVariant read() const;
    void write(const QVariant &) const;
    void emitSignal();

    bool hasChangedNotifier() const;
    bool connectNotifier(QObject *dest, const char *slot) const;
    bool connectNotifier(QObject *dest, int method) const;

    quint32 save() const;
    void restore(quint32, QObject *);

    QMetaMethod method() const;

    enum Type { Invalid = 0x00, 
                Property = 0x01, 
                SignalProperty = 0x02,
                Signal = 0x04,
                Default = 0x08,
                Attached = 0x10 };

    Type type() const;
    bool isProperty() const;
    bool isDefault() const;
    bool isWritable() const;
    bool isDesignable() const;
    bool isValid() const;
    QObject *object() const;

    PropertyCategory propertyCategory() const;
    static PropertyCategory propertyCategory(const QMetaProperty &);

    int propertyType() const;
    const char *propertyTypeName() const;

    bool operator==(const QmlMetaProperty &) const;

    const QMetaProperty &property() const;

    QmlBindableValue *binding();
    static int findSignal(const QObject *, const char *);

    int coreIndex() const;
private:
    void initDefault(QObject *obj);
    void initProperty(QObject *obj, const QString &name);
    friend class QmlEnginePrivate;
    QmlMetaPropertyPrivate *d;
};
typedef QList<QmlMetaProperty> QmlMetaProperties;

QT_END_NAMESPACE

QT_END_HEADER

#endif // QMLMETAPROPERTY_H
