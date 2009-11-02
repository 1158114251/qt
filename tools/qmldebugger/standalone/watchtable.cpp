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
#include "watchtable.h"

#include <QtCore/qdebug.h>
#include <QtGui/qevent.h>
#include <QtGui/qaction.h>
#include <QtGui/qmenu.h>

#include <private/qmldebug_p.h>
#include <QtDeclarative/qmlmetatype.h>

QT_BEGIN_NAMESPACE


WatchTableModel::WatchTableModel(QmlEngineDebug *client, QObject *parent)
    : QAbstractTableModel(parent),
      m_client(client)
{
}

WatchTableModel::~WatchTableModel()
{    
    for (int i=0; i<m_columns.count(); i++)
        delete m_columns[i].watch;
}

void WatchTableModel::setEngineDebug(QmlEngineDebug *client)
{
    m_client = client;
}

void WatchTableModel::addWatch(QmlDebugWatch *watch, const QString &title)
{
    QString property;
    if (qobject_cast<QmlDebugPropertyWatch *>(watch))
        property = qobject_cast<QmlDebugPropertyWatch *>(watch)->name();

    connect(watch, SIGNAL(valueChanged(QByteArray,QVariant)),
            SLOT(watchedValueChanged(QByteArray,QVariant)));

    connect(watch, SIGNAL(stateChanged(State)), SLOT(watchStateChanged()));

    int col = columnCount(QModelIndex());
    beginInsertColumns(QModelIndex(), col, col);

    WatchedEntity e;
    e.title = title;
    e.hasFirstValue = false;
    e.property = property;
    e.watch = watch;
    m_columns.append(e);

    endInsertColumns();
}

void WatchTableModel::removeWatch(QmlDebugWatch *watch)
{
    int column = columnForWatch(watch);
    if (column == -1)
        return;

    WatchedEntity entity = m_columns.takeAt(column);

    for (QList<Value>::Iterator iter = m_values.begin(); iter != m_values.end();) {
        if (iter->column == column) {
            iter = m_values.erase(iter);
        } else {
            if(iter->column > column)
                --iter->column;
            ++iter;
        }
    }

    reset();
}

void WatchTableModel::updateWatch(QmlDebugWatch *watch, const QVariant &value)
{
    int column = columnForWatch(watch);
    if (column == -1)
        return;

    addValue(column, value);

    if (!m_columns[column].hasFirstValue) {
        m_columns[column].hasFirstValue = true;
        m_values[m_values.count() - 1].first = true;
    }
}

QmlDebugWatch *WatchTableModel::findWatch(int column) const
{
    if (column < m_columns.count())
        return m_columns.at(column).watch;
    return 0;
}

QmlDebugWatch *WatchTableModel::findWatch(int objectDebugId, const QString &property) const
{
    for (int i=0; i<m_columns.count(); i++) {
        if (m_columns[i].watch->objectDebugId() == objectDebugId
                && m_columns[i].property == property) {
            return m_columns[i].watch;
        }
    }
    return 0;
}

int WatchTableModel::rowCount(const QModelIndex &) const
{
    return m_values.count();
}

int WatchTableModel::columnCount(const QModelIndex &) const
{
    return m_columns.count();
}

QVariant WatchTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (section < m_columns.count() && role == Qt::DisplayRole)
            return m_columns.at(section).title;
    } else {
        if (role == Qt::DisplayRole)
            return section + 1;
    }
    return QVariant();
}

QVariant WatchTableModel::data(const QModelIndex &idx, int role) const
{
    if (m_values.at(idx.row()).column == idx.column()) {
        if (role == Qt::DisplayRole) {
            const QVariant &value = m_values.at(idx.row()).variant;
            QString str = value.toString();

            if (str.isEmpty() && QmlMetaType::isObject(value.userType())) {
                QObject *o = QmlMetaType::toQObject(value);
                if(o) {
                    QString objectName = o->objectName();
                    if(objectName.isEmpty())
                        objectName = QLatin1String("<unnamed>");
                    str = QLatin1String(o->metaObject()->className()) +
                          QLatin1String(": ") + objectName;
                }
            }

            if(str.isEmpty()) {
                QDebug d(&str);
                d << value;
            }
            return QVariant(str);
        } else if(role == Qt::BackgroundRole) {
            if(m_values.at(idx.row()).first)
                return QColor(Qt::green);
            else
                return QVariant();
        } else {
            return QVariant();
        }
    } else {
        return QVariant();
    }
}

void WatchTableModel::watchStateChanged()
{
    QmlDebugWatch *watch = qobject_cast<QmlDebugWatch*>(sender());

    if (watch && watch->state() == QmlDebugWatch::Inactive) {
        removeWatch(watch);
        watch->deleteLater();
    }
}

int WatchTableModel::columnForWatch(QmlDebugWatch *watch) const
{
    for (int i=0; i<m_columns.count(); i++) {
        if (m_columns.at(i).watch == watch)
            return i;
    }
    return -1;
}

void WatchTableModel::addValue(int column, const QVariant &value)
{
    int row = columnCount(QModelIndex());
    beginInsertRows(QModelIndex(), row, row);

    Value v;
    v.column = column;
    v.variant = value;
    v.first = false;
    m_values.append(v);

    endInsertRows();
}

void WatchTableModel::togglePropertyWatch(const QmlDebugObjectReference &object, const QmlDebugPropertyReference &property)
{
    if (!m_client || !property.hasNotifySignal())
        return;

    QmlDebugWatch *watch = findWatch(object.debugId(), property.name());
    if (watch) {
        // watch will be deleted in watchStateChanged()
        m_client->removeWatch(watch);
        return;
    }

    watch = m_client->addWatch(property, this);
    if (watch->state() == QmlDebugWatch::Dead) {
        delete watch;
        watch = 0;
    } else {
        QString desc = property.name()
                + QLatin1String(" on\n")
                + object.className()
                + QLatin1String(":\n")
                + (object.name().isEmpty() ? QLatin1String("<unnamed object>") : object.name());
        addWatch(watch, desc);
        emit watchCreated(watch);
    }
}

void WatchTableModel::watchedValueChanged(const QByteArray &propertyName, const QVariant &value)
{
    Q_UNUSED(propertyName);
    QmlDebugWatch *watch = qobject_cast<QmlDebugWatch*>(sender());
    if (watch)
        updateWatch(watch, value);
}

void WatchTableModel::expressionWatchRequested(const QmlDebugObjectReference &obj, const QString &expr)
{
    if (!m_client)
        return;
        
    QmlDebugWatch *watch = m_client->addWatch(obj, expr, this);

    if (watch->state() == QmlDebugWatch::Dead) {
        delete watch;
        watch = 0;
    } else {
        addWatch(watch, expr);
        emit watchCreated(watch);
    }
}

void WatchTableModel::removeWatchAt(int column)
{
    if (!m_client)
        return;
        
    QmlDebugWatch *watch = findWatch(column);
    if (watch) {
        m_client->removeWatch(watch);
        delete watch;
        watch = 0;
    }
}

void WatchTableModel::removeAllWatches()
{
    for (int i=0; i<m_columns.count(); i++) {
        if (m_client)
            m_client->removeWatch(m_columns[i].watch);
        else    
            delete m_columns[i].watch;
    }
    m_columns.clear();
    m_values.clear();
    reset();
}

//----------------------------------------------

WatchTableHeaderView::WatchTableHeaderView(WatchTableModel *model, QWidget *parent)
    : QHeaderView(Qt::Horizontal, parent),
      m_model(model)
{
    setClickable(true);
}

void WatchTableHeaderView::mousePressEvent(QMouseEvent *me)
{
    QHeaderView::mousePressEvent(me);

    if (me->button() == Qt::RightButton && me->type() == QEvent::MouseButtonPress) {
        int col = logicalIndexAt(me->pos());
        if (col >= 0) {
            QAction action(tr("Stop watching"), 0);
            QList<QAction *> actions;
            actions << &action;
            if (QMenu::exec(actions, me->globalPos()))
                m_model->removeWatchAt(col);
        }
    } 
}


//----------------------------------------------

WatchTableView::WatchTableView(WatchTableModel *model, QWidget *parent)
    : QTableView(parent),
      m_model(model)
{
    setAlternatingRowColors(true);
    connect(model, SIGNAL(watchCreated(QmlDebugWatch*)), SLOT(watchCreated(QmlDebugWatch*)));
    connect(this, SIGNAL(activated(QModelIndex)), SLOT(indexActivated(QModelIndex)));
}

void WatchTableView::indexActivated(const QModelIndex &index)
{
    QmlDebugWatch *watch = m_model->findWatch(index.column());
    if (watch)
        emit objectActivated(watch->objectDebugId());
}

void WatchTableView::watchCreated(QmlDebugWatch *watch)
{
    int column = m_model->columnForWatch(watch);
    resizeColumnToContents(column);
}

QT_END_NAMESPACE
