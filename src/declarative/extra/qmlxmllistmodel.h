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

#ifndef QMLXMLLISTMODEL_H
#define QMLXMLLISTMODEL_H

#include <QtDeclarative/qml.h>
#include <QtDeclarative/QListModelInterface>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)

class QmlContext;
class Q_DECLARATIVE_EXPORT XmlListModelRole : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString query READ query WRITE setQuery)

public:
    XmlListModelRole() {}
    ~XmlListModelRole() {}

    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }

    QString query() const { return m_query; }
    void setQuery(const QString &query) { m_query = query; }

private:
    QString m_name;
    QString m_query;
};
QML_DECLARE_TYPE(XmlListModelRole)

class QmlXmlListModelPrivate;
class Q_DECLARATIVE_EXPORT QmlXmlListModel : public QListModelInterface, public QmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QmlParserStatus)
    Q_ENUMS(Status)

    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(qreal progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QUrl source READ source WRITE setSource)
    Q_PROPERTY(QString query READ query WRITE setQuery)
    Q_PROPERTY(QString namespaceDeclarations READ namespaceDeclarations WRITE setNamespaceDeclarations)
    Q_PROPERTY(QmlList<XmlListModelRole *> *roles READ roleObjects)
    Q_CLASSINFO("DefaultProperty", "roles")
public:
    QmlXmlListModel(QObject *parent = 0);
    ~QmlXmlListModel();

    virtual QHash<int,QVariant> data(int index, const QList<int> &roles = (QList<int>())) const;
    virtual int count() const;
    virtual QList<int> roles() const;
    virtual QString toString(int role) const;

    QmlList<XmlListModelRole *> *roleObjects();

    QUrl source() const;
    void setSource(const QUrl&);

    QString query() const;
    void setQuery(const QString&);

    QString namespaceDeclarations() const;
    void setNamespaceDeclarations(const QString&);

    enum Status { Idle, Loading, Error };
    Status status() const;
    qreal progress() const;

    virtual void classComplete();

signals:
    void statusChanged(Status);
    void progressChanged(qreal progress);

public Q_SLOTS:
    void reload();

private Q_SLOTS:
    void requestFinished();
    void requestProgress(qint64,qint64);
    void queryCompleted(int,int);

private:
    Q_DECLARE_PRIVATE(QmlXmlListModel)
    Q_DISABLE_COPY(QmlXmlListModel)
};

QML_DECLARE_TYPE(QmlXmlListModel)

QT_END_NAMESPACE

QT_END_HEADER

#endif // QMLXMLLISTMODEL_H
