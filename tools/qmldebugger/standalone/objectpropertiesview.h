/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt QML Debugger of the Qt Toolkit.
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
#ifndef PROPERTIESTABLEMODEL_H
#define PROPERTIESTABLEMODEL_H

#include <QtDeclarative/qmldebug.h>

#include <QtGui/qwidget.h>

QT_BEGIN_NAMESPACE

class QTreeWidget;
class QTreeWidgetItem;
class QmlDebugConnection;
class PropertiesViewItem;

class ObjectPropertiesView : public QWidget
{
    Q_OBJECT
public:
    ObjectPropertiesView(QmlEngineDebug *client = 0, QWidget *parent = 0);

    void setEngineDebug(QmlEngineDebug *client);
    void clear();
    
signals:
    void activated(const QmlDebugObjectReference &, const QmlDebugPropertyReference &);

public slots:
    void reload(const QmlDebugObjectReference &);
    void watchCreated(QmlDebugWatch *);

private slots:
    void queryFinished();
    void watchStateChanged();
    void valueChanged(const QByteArray &name, const QVariant &value);
    void itemActivated(QTreeWidgetItem *i);

private:
    void setObject(const QmlDebugObjectReference &object);
    void setWatched(const QString &property, bool watched);
    void setPropertyValue(PropertiesViewItem *item, const QVariant &value, bool makeGray);

    QmlEngineDebug *m_client;
    QmlDebugObjectQuery *m_query;
    QmlDebugWatch *m_watch;

    QTreeWidget *m_tree;
    QmlDebugObjectReference m_object;
};


QT_END_NAMESPACE

#endif
