/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
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

#include "qfilesystemengine_p.h"

bool QFileSystemEngine::isCaseSensitive()
{
    return true;
}

//static
QFileSystemEntry QFileSystemEngine::getLinkTarget(const QFileSystemEntry &link)
{
    return link; // TODO implement
}

//static
QFileSystemEntry QFileSystemEngine::canonicalName(const QFileSystemEntry &entry)
{
    return entry; // TODO implement;
}

//static
QFileSystemEntry QFileSystemEngine::absoluteName(const QFileSystemEntry &entry)
{
    return entry; // TODO implement;
}

//static
QString QFileSystemEngine::bundleName(const QFileSystemEntry &entry)
{
    return QString(); // TODO implement;
}

//static
bool QFileSystemEngine::fillMetaData(const QFileSystemEntry &entry, QFileSystemMetaData &data, QFileSystemMetaData::FileFlags what)
{
    return false; // TODO implement;
}

//static
bool QFileSystemEngine::createDirectory(const QFileSystemEntry &entry, bool createParents)
{
    return false; // TODO implement;
}

//static
bool QFileSystemEngine::removeDirectory(const QFileSystemEntry &entry, bool removeEmptyParents)
{
    return false; // TODO implement;
}

//static
bool QFileSystemEngine::createLink(const QFileSystemEntry &source, const QFileSystemEntry &target)
{
    return false; // TODO implement;
}

//static
bool QFileSystemEngine::copyFile(const QFileSystemEntry &source, const QFileSystemEntry &target)
{
    return false; // TODO implement;
}

//static
bool QFileSystemEngine::renameFile(const QFileSystemEntry &source, const QFileSystemEntry &target)
{
    return false; // TODO implement;
}

//static
bool QFileSystemEngine::removeFile(const QFileSystemEntry &entry)
{
    return false; // TODO implement;
}

//static
bool QFileSystemEngine::setPermissions(const QFileSystemEntry &entry, QFile::Permissions permissions, QFileSystemMetaData *data)
{
    return false; // TODO implement;
}
