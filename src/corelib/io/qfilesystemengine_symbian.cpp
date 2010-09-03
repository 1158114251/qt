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
#include "qfsfileengine.h"
#include "qcore_symbian_p.h"

#include <f32file.h>

QT_BEGIN_NAMESPACE

bool QFileSystemEngine::isCaseSensitive()
{
    return false;
}

//static
QFileSystemEntry QFileSystemEngine::getLinkTarget(const QFileSystemEntry &link)
{
    return link; // TODO implement
}

//static
QFileSystemEntry QFileSystemEngine::canonicalName(const QFileSystemEntry &entry)
{
    if (entry.isEmpty() || entry.isRoot())
        return entry;

    QFileSystemEntry result = absoluteName(entry);
    RFs& fs(qt_s60GetRFs());
    TUint e;
    //Att is faster than Entry for determining if a file exists, due to smaller IPC
    TInt r = fs.Att(qt_QString2TPtrC(result.nativeFilePath()), e);

    if (r == KErrNone) {
        return result;
    } else { // file doesn't exist
        return QFileSystemEntry();
    }
}

//static
QFileSystemEntry QFileSystemEngine::absoluteName(const QFileSystemEntry &entry)
{
    if (entry.isAbsolute())
        return entry;

    QString orig = entry.filePath();
    QString result;
    if (orig.isEmpty() || !orig.startsWith('/')) {
        QFileSystemEntry cur(QFSFileEngine::currentPath());
        result = cur.filePath();
    }
    if (!orig.isEmpty() && !(orig.length() == 1 && orig[0] == '.')) {
        if (!result.isEmpty() && !result.endsWith('/'))
            result.append('/');
        result.append(orig);
    }

    if (result.length() == 1 && result[0] == '/')
        return QFileSystemEntry(result);
    const bool isDir = result.endsWith('/');

    result = QDir::cleanPath(result);
    if (isDir)
        result.append(QLatin1Char('/'));
    return QFileSystemEntry(result);
}

//static
QString QFileSystemEngine::bundleName(const QFileSystemEntry &entry)
{
    return QString(); // TODO implement;
}

//static
bool QFileSystemEngine::fillMetaData(const QFileSystemEntry &entry, QFileSystemMetaData &data, QFileSystemMetaData::MetaDataFlags what)
{
    return false; // TODO implement;
}

//static
bool QFileSystemEngine::createDirectory(const QFileSystemEntry &entry, bool createParents)
{
    QString abspath = absoluteName(entry).nativeFilePath();
    if (!abspath.endsWith(QLatin1Char('\\')))
        abspath.append(QLatin1Char('\\'));
    TInt r;
    if (createParents)
        r = qt_s60GetRFs().MkDirAll(qt_QString2TPtrC(abspath));
    else
        r = qt_s60GetRFs().MkDir(qt_QString2TPtrC(abspath));
    return (r == KErrNone || r == KErrAlreadyExists);
}

//static
bool QFileSystemEngine::removeDirectory(const QFileSystemEntry &entry, bool removeEmptyParents)
{
    QString abspath = absoluteName(entry).nativeFilePath();
    if (!abspath.endsWith(QLatin1Char('\\')))
        abspath.append(QLatin1Char('\\'));
    TPtrC dir(qt_QString2TPtrC(abspath));
    RFs& fs = qt_s60GetRFs();
    bool ok = false;
    //behaviour of FS file engine:
    //returns true if the directory could be removed
    //success/failure of removing parent directories does not matter
    while (KErrNone == fs.RmDir(dir)) {
        ok = true;
        if (!removeEmptyParents)
            break;
        //RFs::RmDir treats "c:\foo\bar" and "c:\foo\" the same, so it is sufficient to remove the last \ to the end
        dir.Set(dir.Left(dir.LocateReverse(TChar('\\'))));
    }
    return ok;
}

//static
bool QFileSystemEngine::createLink(const QFileSystemEntry &source, const QFileSystemEntry &target)
{
    Q_UNUSED(source)
    Q_UNUSED(target)
    return false;
}

//static
bool QFileSystemEngine::copyFile(const QFileSystemEntry &source, const QFileSystemEntry &target)
{
    //TODO: review - should this be a singleton?
    CFileMan *fm = 0;
    TRAPD(err, fm = CFileMan::NewL(qt_s60GetRFs()));
    if(err == KErrNone) {
        err = fm->Copy(qt_QString2TPtrC(source.nativeFilePath()), qt_QString2TPtrC(target.nativeFilePath()), 0);
        delete fm;
        return true;
    }
    //TODO: error reporting d->setSymbianError(err, QFile::CopyError);
    return false;
}

//static
bool QFileSystemEngine::renameFile(const QFileSystemEntry &source, const QFileSystemEntry &target)
{
    QString sourcepath = absoluteName(source).nativeFilePath();
    QString targetpath = absoluteName(target).nativeFilePath();
    RFs& fs(qt_s60GetRFs());
    TInt err = fs.Rename(qt_QString2TPtrC(sourcepath), qt_QString2TPtrC(targetpath));
    return err == KErrNone; // TODO error reporting;
}

//static
bool QFileSystemEngine::removeFile(const QFileSystemEntry &entry)
{
    QString targetpath = absoluteName(entry).nativeFilePath();
    RFs& fs(qt_s60GetRFs());
    TInt err = fs.Delete(qt_QString2TPtrC(targetpath));
    return false; // TODO error reporting;
}

//static
bool QFileSystemEngine::setPermissions(const QFileSystemEntry &entry, QFile::Permissions permissions, QFileSystemMetaData *data)
{
    QString targetpath = absoluteName(entry).nativeFilePath();
    TUint setmask = 0;
    TUint clearmask = 0;
    RFs& fs(qt_s60GetRFs());
    if (permissions & (QFile::ReadOwner | QFile::ReadUser | QFile::ReadGroup | QFile::ReadOther))
        clearmask = KEntryAttReadOnly;
    else
        setmask = KEntryAttReadOnly;
    TInt err = fs.SetAtt(qt_QString2TPtrC(targetpath), setmask, clearmask);
    return err != KErrNone; // TODO error reporting, metadata update;
}

QT_END_NAMESPACE
