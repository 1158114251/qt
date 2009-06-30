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
#include <QDirModel>
#include <qdebug.h>
#include "qmlfolderlistmodel.h"

QT_BEGIN_NAMESPACE

class QmlFolderListModelPrivate : public QObjectPrivate
{
public:
    QmlFolderListModelPrivate() {
        folder = QDir::currentPath();
        nameFilters << "*";
    }

    QDirModel model;
    QString folder;
    QStringList nameFilters;
    QModelIndex folderIndex;
};

QmlFolderListModel::QmlFolderListModel(QObject *parent)
    : QListModelInterface(*(new QmlFolderListModelPrivate), parent)
{
    Q_D(QmlFolderListModel);
    d->model.setFilter(QDir::AllDirs | QDir::Files | QDir::Drives);
    connect(&d->model, SIGNAL(rowsInserted(const QModelIndex&,int,int))
            , this, SLOT(inserted(const QModelIndex&,int,int)));
}

QmlFolderListModel::~QmlFolderListModel()
{
}

QHash<int,QVariant> QmlFolderListModel::data(int index, const QList<int> &roles) const
{
    Q_D(const QmlFolderListModel);
    QHash<int,QVariant> folderData;
    QModelIndex modelIndex = d->model.index(index, 0, d->folderIndex);
    if (modelIndex.isValid()) {
        folderData[QDirModel::FileNameRole] = d->model.data(modelIndex, QDirModel::FileNameRole);
        folderData[QDirModel::FilePathRole] = d->model.data(modelIndex, QDirModel::FilePathRole);
    }

    return folderData;
}

int QmlFolderListModel::count() const
{
    Q_D(const QmlFolderListModel);
    if (!d->folderIndex.isValid())
        return 0;
    return d->model.rowCount(d->folderIndex);
}

QList<int> QmlFolderListModel::roles() const
{
    QList<int> r;
    r << QDirModel::FileNameRole;
    r << QDirModel::FilePathRole;
    return r;
}

QString QmlFolderListModel::toString(int role) const
{
    switch (role) {
    case QDirModel::FileNameRole:
        return QLatin1String("fileName");
    case QDirModel::FilePathRole:
        return QLatin1String("filePath");
    }

    return QString();
}

QString QmlFolderListModel::folder() const
{
    Q_D(const QmlFolderListModel);
    return d->folder;
}

void QmlFolderListModel::setFolder(const QString &folder)
{
    Q_D(QmlFolderListModel);
    if (folder == d->folder)
        return;
    QModelIndex index = d->model.index(folder);
    if (index.isValid() && d->model.isDir(index)) {
        d->folder = folder;
        QMetaObject::invokeMethod(this, "refresh", Qt::QueuedConnection);
        emit folderChanged();
    }
}

QStringList QmlFolderListModel::nameFilters() const
{
    Q_D(const QmlFolderListModel);
    return d->nameFilters;
}

void QmlFolderListModel::setNameFilters(const QStringList &filters)
{
    Q_D(QmlFolderListModel);
    d->nameFilters = filters;
    d->model.setNameFilters(d->nameFilters);
}

void QmlFolderListModel::classComplete()
{
    Q_D(QmlFolderListModel);
    if (!d->folderIndex.isValid())
        QMetaObject::invokeMethod(this, "refresh", Qt::QueuedConnection);
}

bool QmlFolderListModel::isFolder(int index) const
{
    Q_D(const QmlFolderListModel);
    return d->model.isDir(d->model.index(index, 0, d->folderIndex));
}

void QmlFolderListModel::refresh()
{
    Q_D(QmlFolderListModel);
    int prevCount = count();
    d->folderIndex = QModelIndex();
    if (prevCount)
        emit itemsRemoved(0, prevCount);
    d->folderIndex = d->model.index(d->folder);
    qDebug() << "count" << count();
    if (count())
        emit itemsInserted(0, count());
}

void QmlFolderListModel::inserted(const QModelIndex &index, int start, int end)
{
    Q_D(QmlFolderListModel);
    qDebug() << "inserted" << start << end;
    if (index == d->folderIndex)
        emit itemsInserted(start, end - start + 1);
}

QML_DEFINE_TYPE(QmlFolderListModel,FolderListModel)

QT_END_NAMESPACE

