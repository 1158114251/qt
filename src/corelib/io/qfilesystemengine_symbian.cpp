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
#include <QtCore/private/qcore_symbian_p.h>

#include <f32file.h>

QT_BEGIN_NAMESPACE

bool QFileSystemEngine::isCaseSensitive()
{
    return false;
}

//TODO: resolve this with QDir::cleanPath, without breaking the behaviour of that
//function which is documented only by autotest
//input: a dirty absolute path, e.g. c:/../../foo/./
//output: a clean absolute path, e.g. c:/foo/
static QString symbianCleanAbsolutePath(const QString& path)
{
    bool isDir = path.endsWith(QLatin1Char('/'));
    //using SkipEmptyParts flag to eliminate duplicated slashes
    QStringList components = path.split(QLatin1Char('/'), QString::SkipEmptyParts);
    int cdups = 0;
    for(int i=components.count() - 1; i>=0; --i) {
        if(components.at(i) == QLatin1String("..")) {
            components.removeAt(i);
            cdups++;
        }
        else if(components.at(i) == QLatin1String(".")) {
            components.removeAt(i);
        }
        else if(cdups && i > 0) {
            --cdups;
            components.removeAt(i);
        }
    }
    QString result = components.join(QLatin1String("/"));
    if ((isDir&& !result.endsWith(QLatin1Char('/')))
        || (result.length() == 2 && result.at(1).unicode() == ':'))
        result.append(QLatin1Char('/'));
    return result;
}

//static
QFileSystemEntry QFileSystemEngine::getLinkTarget(const QFileSystemEntry &link, QFileSystemMetaData &data)
{
    return link; // TODO implement
}

//static
QFileSystemEntry QFileSystemEngine::canonicalName(const QFileSystemEntry &entry)
{
    if (entry.isEmpty() || entry.isRoot())
        return entry;

    QFileSystemEntry result = absoluteName(entry);
    QFileSystemMetaData meta;
    if (!fillMetaData(result, meta, QFileSystemMetaData::ExistsAttribute) || !meta.exists()) {
        // file doesn't exist
        return QFileSystemEntry();
    } else {
        return result;
    }
}

//static
QFileSystemEntry QFileSystemEngine::absoluteName(const QFileSystemEntry &entry)
{
    QString orig = entry.filePath();
    const bool needsDrive = (!orig.isEmpty() && orig.at(0).unicode() == '/');
    const bool isDriveLetter = (orig.size() == 2 && orig.at(1).unicode() == ':');
    const bool isDriveRelative = (orig.size() > 2 && orig.at(1).unicode() == ':' && orig.at(2).unicode() != '/');
    const bool isDirty = (orig.contains(QLatin1String("/../")) || orig.contains(QLatin1String("/./")) ||
            orig.endsWith(QLatin1String("/..")) || orig.endsWith(QLatin1String("/.")));
    const bool isAbsolute = entry.isAbsolute();
    if (isAbsolute &&
        !(needsDrive || isDriveLetter || isDriveRelative || isDirty))
        return entry;

    QString result;
    if (needsDrive || isDriveLetter || isDriveRelative || !isAbsolute || orig.isEmpty()) {
        QFileSystemEntry cur(QFSFileEngine::currentPath());
        if(needsDrive)
            result = cur.filePath().left(2);
        else if(isDriveRelative && cur.filePath().at(0) != orig.at(0))
            result = orig.left(2); // for BC, see tst_QFileInfo::absolutePath(<not current drive>:my.dll)
        else
            result = cur.filePath();
        if(isDriveLetter) {
            result[0] = orig.at(0); //copy drive letter
            orig.clear();
        }
        if(isDriveRelative) {
            orig = orig.mid(2); //discard the drive specifier from orig
        }
    }
    if (!orig.isEmpty() && !(orig.length() == 1 && orig.at(0).unicode() == '.')) {
        if (!result.isEmpty() && !result.endsWith(QLatin1Char('/')))
            result.append(QLatin1Char('/'));
        result.append(orig);
    }

    return symbianCleanAbsolutePath(result);
}

//static
QString QFileSystemEngine::resolveUserName(uint userId)
{
    Q_UNUSED(userId)
    return QString(); // no users or groups on symbian
}

//static
QString QFileSystemEngine::resolveGroupName(uint groupId)
{
    Q_UNUSED(groupId)
    return QString(); // no users or groups on symbian
}

//static
QString QFileSystemEngine::bundleName(const QFileSystemEntry &entry)
{
    Q_UNUSED(entry);
    return QString();
}

void QFileSystemMetaData::fillFromTEntry(const TEntry& entry)
{
    entryFlags &= ~(QFileSystemMetaData::SymbianTEntryFlags);
    knownFlagsMask |= QFileSystemMetaData::SymbianTEntryFlags;
    //Symbian doesn't have unix type file permissions
    entryFlags |= QFileSystemMetaData::Permissions;
    if(entry.IsReadOnly()) {
        entryFlags &= ~(QFileSystemMetaData::WritePermissions);
    }
    //set the type
    if(entry.IsDir())
        entryFlags |= QFileSystemMetaData::DirectoryType;
    else
        entryFlags |= QFileSystemMetaData::FileType;

    //set the attributes
    entryFlags |= QFileSystemMetaData::ExistsAttribute;
    if(entry.IsHidden())
        entryFlags |= QFileSystemMetaData::HiddenAttribute;

#ifdef SYMBIAN_ENABLE_64_BIT_FILE_SERVER_API
    size_ = entry.FileSize();
#else
    size_ = (TUint)(entry.iSize);
#endif

    modificationTime_ = entry.iModified;
}

void QFileSystemMetaData::fillFromVolumeInfo(const TVolumeInfo& info)
{
    entryFlags &= ~(QFileSystemMetaData::SymbianTEntryFlags);
    knownFlagsMask |= QFileSystemMetaData::SymbianTEntryFlags;
    entryFlags |= QFileSystemMetaData::ExistsAttribute;
    entryFlags |= QFileSystemMetaData::Permissions;
    if(info.iDrive.iDriveAtt & KDriveAttRom) {
        entryFlags &= ~(QFileSystemMetaData::WritePermissions);
    }
    entryFlags |= QFileSystemMetaData::DirectoryType;
    size_ = info.iSize;
    modificationTime_ = qt_symbian_time_t_To_TTime(0);
}

//static
bool QFileSystemEngine::fillMetaData(const QFileSystemEntry &entry, QFileSystemMetaData &data, QFileSystemMetaData::MetaDataFlags what)
{
    if (what & QFileSystemMetaData::SymbianTEntryFlags) {
        RFs& fs(qt_s60GetRFs());
        TInt err;
        QFileSystemEntry absentry(absoluteName(entry));
        if (absentry.isRoot()) {
            //Root directories don't have an entry, and Entry() returns KErrBadName.
            //Therefore get information about the volume instead.
            TInt drive;
            err = RFs::CharToDrive(TChar(absentry.nativeFilePath().at(0).unicode()), drive);
            if (!err) {
                TVolumeInfo info;
                err = fs.Volume(info, drive);
                if (!err)
                    data.fillFromVolumeInfo(info);
            }
        } else {
            TEntry ent;
            err = fs.Entry(qt_QString2TPtrC(absentry.nativeFilePath()), ent);
            if (!err)
                data.fillFromTEntry(ent);
        }
        if (err) {
            data.size_ = 0;
            data.modificationTime_ = TTime(0);
        }
    }
    return data.hasFlags(what);
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
    return err == KErrNone; // TODO error reporting;
}

//static
bool QFileSystemEngine::setPermissions(const QFileSystemEntry &entry, QFile::Permissions permissions, QFileSystemMetaData *data)
{
    QString targetpath = absoluteName(entry).nativeFilePath();
    TUint setmask = 0;
    TUint clearmask = 0;
    RFs& fs(qt_s60GetRFs());
    if (permissions & (QFile::WriteOwner | QFile::WriteUser | QFile::WriteGroup | QFile::WriteOther))
        clearmask = KEntryAttReadOnly; //if anyone can write, it's not read-only
    else
        setmask = KEntryAttReadOnly;
    TInt err = fs.SetAtt(qt_QString2TPtrC(targetpath), setmask, clearmask);
    if (data && !err) {
        data->entryFlags &= ~QFileSystemMetaData::Permissions;
        data->entryFlags |= QFileSystemMetaData::MetaDataFlag(uint(permissions));
        data->knownFlagsMask |= QFileSystemMetaData::Permissions;
    }
    return err == KErrNone; // TODO error reporting
}

QT_END_NAMESPACE
