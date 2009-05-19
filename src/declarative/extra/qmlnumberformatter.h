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

#ifndef QMLNUMBERFORMATTER_H
#define QMLNUMBERFORMATTER_H

#include <QtDeclarative/qml.h>
#include <QtDeclarative/qnumberformat.h>


QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)
class QmlNumberFormatterPrivate;
class Q_DECLARATIVE_EXPORT QmlNumberFormatter : public QObject, public QmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QmlParserStatus)

    Q_PROPERTY(QString text READ text NOTIFY textChanged)
    Q_PROPERTY(QString format READ format WRITE setFormat)
    Q_PROPERTY(qreal number READ number WRITE setNumber)
public:
    QmlNumberFormatter(QObject *parent=0);
    ~QmlNumberFormatter();

    QString text() const;

    qreal number() const;
    void setNumber(const qreal &);

    QString format() const;
    void setFormat(const QString &);

    virtual void classBegin();
    virtual void classComplete();

Q_SIGNALS:
    void textChanged();

private:
    Q_DISABLE_COPY(QmlNumberFormatter)
    Q_DECLARE_PRIVATE(QmlNumberFormatter)
};

QML_DECLARE_TYPE(QmlNumberFormatter);


QT_END_NAMESPACE

QT_END_HEADER
#endif
