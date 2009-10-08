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

#include "private/qobject_p.h"
#include "qmlpackage.h"

QT_BEGIN_NAMESPACE

class QmlPackagePrivate : public QObjectPrivate
{
public:
    QmlPackagePrivate() {}

    QmlConcreteList<QObject *> dataList;
};

class QmlPackageAttached : public QObject
{
Q_OBJECT
public:
    QmlPackageAttached(QObject *parent);
    virtual ~QmlPackageAttached();

    Q_PROPERTY(QString name READ name WRITE setName)
    QString name() const;
    void setName(const QString &n);

    static QHash<QObject *, QmlPackageAttached *> attached;
private:
    QString _name;
};

QHash<QObject *, QmlPackageAttached *> QmlPackageAttached::attached;

QmlPackageAttached::QmlPackageAttached(QObject *parent)
: QObject(parent)
{
    attached.insert(parent, this);
}

QmlPackageAttached::~QmlPackageAttached()
{
    attached.remove(parent());
}

QString QmlPackageAttached::name() const 
{ 
    return _name; 
}

void QmlPackageAttached::setName(const QString &n) 
{ 
    _name = n; 
}

QmlPackage::QmlPackage(QObject *parent)
    : QObject(*(new QmlPackagePrivate), parent)
{
}

QmlPackage::~QmlPackage()
{
    Q_D(QmlPackage);
    for (int ii = 0; ii < d->dataList.count(); ++ii) {
        QObject *obj = d->dataList.at(ii);
        delete obj;
    }
}

QmlList<QObject *> *QmlPackage::data()
{
    Q_D(QmlPackage);
    return &d->dataList;
}

bool QmlPackage::hasPart(const QString &name)
{
    Q_D(QmlPackage);
    for (int ii = 0; ii < d->dataList.count(); ++ii) {
        QObject *obj = d->dataList.at(ii);
        QmlPackageAttached *a = QmlPackageAttached::attached.value(obj);
        if (a && a->name() == name)
            return true;
    }
    return false;
}

QObject *QmlPackage::part(const QString &name)
{
    Q_D(QmlPackage);
    if (name.isEmpty() && !d->dataList.isEmpty())
        return d->dataList.at(0);

    for (int ii = 0; ii < d->dataList.count(); ++ii) {
        QObject *obj = d->dataList.at(ii);
        QmlPackageAttached *a = QmlPackageAttached::attached.value(obj);
        if (a && a->name() == name)
            return obj;
    }

    if (name == QLatin1String("default") && !d->dataList.isEmpty())
        return d->dataList.at(0);

    return 0;
}

QmlPackageAttached *QmlPackage::qmlAttachedProperties(QObject *o)
{
    return new QmlPackageAttached(o);
}

QML_DEFINE_TYPE(Qt,4,6,(QT_VERSION&0x00ff00)>>8,Package,QmlPackage)

QT_END_NAMESPACE

#include "qmlpackage.moc"
