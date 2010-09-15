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

#include "qplatformdefs.h"
#include "qfilesystemengine_p.h"
#include "qplatformdefs.h"
#include "qfsfileengine.h"

#include <stdlib.h> // for realpath()
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#if defined(Q_OS_SYMBIAN)
# include <sys/syslimits.h>
# include <f32file.h>
# include <pathinfo.h>
# include <QtCore/private/qcore_symbian_p.h>
#endif

#if defined(Q_OS_MAC)
# include <QtCore/private/qcore_mac_p.h>
#endif

QT_BEGIN_NAMESPACE

#if defined(Q_OS_SYMBIAN)
static bool _q_isSymbianHidden(const QFileSystemEntry &entry, bool isDir)
{
    RFs rfs = qt_s60GetRFs();

    QFileSystemEntry absoluteEntry = QFileSystemEngine::absoluteName(entry);
    QString absolutePath = absoluteEntry.filePath();

    if (isDir && !absolutePath.endsWith(QLatin1Char('/')))
        absolutePath.append(QLatin1Char('/'));

    TPtrC ptr(qt_QString2TPtrC(absolutePath));
    TUint attributes;
    TInt err = rfs.Att(ptr, attributes);
    return (err == KErrNone && (attributes & KEntryAttHidden));
}
#endif

#if !defined(QWS) && defined(Q_OS_MAC)
static inline bool _q_isMacHidden(const char *nativePath)
{
    OSErr err;

    FSRef fsRef;
    err = FSPathMakeRefWithOptions(reinterpret_cast<const UInt8 *>(nativePath),
            kFSPathMakeRefDoNotFollowLeafSymlink, &fsRef, 0);
    if (err != noErr)
        return false;

    FSCatalogInfo catInfo;
    err = FSGetCatalogInfo(&fsRef, kFSCatInfoFinderInfo, &catInfo, NULL, NULL, NULL);
    if (err != noErr)
        return false;

    FileInfo * const fileInfo = reinterpret_cast<FileInfo*>(&catInfo.finderInfo);
    return (fileInfo->finderFlags & kIsInvisible);
}
#else
static inline bool _q_isMacHidden(const char *nativePath)
{
    // no-op
    return false;
}
#endif

bool QFileSystemEngine::isCaseSensitive()
{
#if defined(Q_OS_SYMBIAN)
    return false;
#else
    return true;
#endif
}

//static
QFileSystemEntry QFileSystemEngine::getLinkTarget(const QFileSystemEntry &link, QFileSystemMetaData &data)
{
#if defined(__GLIBC__) && !defined(PATH_MAX)
#define PATH_CHUNK_SIZE 256
    char *s = 0;
    int len = -1;
    int size = PATH_CHUNK_SIZE;

    while (1) {
        s = (char *) ::realloc(s, size);
        Q_CHECK_PTR(s);
        len = ::readlink(link.nativeFilePath().constData(), s, size);
        if (len < 0) {
            ::free(s);
            break;
        }
        if (len < size) {
            break;
        }
        size *= 2;
    }
#else
    char s[PATH_MAX+1];
    int len = readlink(link.nativeFilePath().constData(), s, PATH_MAX);
#endif
    if (len > 0) {
        QString ret;
        if (!data.hasFlags(QFileSystemMetaData::DirectoryType))
            fillMetaData(link, data, QFileSystemMetaData::DirectoryType);
        if (data.isDirectory() && s[0] != '/') {
            QDir parent(link.filePath());
            parent.cdUp();
            ret = parent.path();
            if (!ret.isEmpty() && !ret.endsWith(QLatin1Char('/')))
                ret += QLatin1Char('/');
        }
        s[len] = '\0';
        ret += QFile::decodeName(QByteArray(s));
#if defined(__GLIBC__) && !defined(PATH_MAX)
        ::free(s);
#endif

        if (!ret.startsWith(QLatin1Char('/'))) {
            if (link.filePath().startsWith(QLatin1Char('/'))) {
                ret.prepend(link.filePath().left(link.filePath().lastIndexOf(QLatin1Char('/')))
                            + QLatin1Char('/'));
            } else {
                ret.prepend(QDir::currentPath() + QLatin1Char('/'));
            }
        }
        ret = QDir::cleanPath(ret);
        if (ret.size() > 1 && ret.endsWith(QLatin1Char('/')))
            ret.chop(1);
        return QFileSystemEntry(ret);
    }
#if !defined(QWS) && defined(Q_OS_MAC)
    {
        FSRef fref;
        if (FSPathMakeRef((const UInt8 *)QFile::encodeName(QDir::cleanPath(link.filePath())).data(), &fref, 0) == noErr) {
            // TODO get the meta data info from the QFileSystemMetaData object
            Boolean isAlias, isFolder;
            if (FSResolveAliasFile(&fref, true, &isFolder, &isAlias) == noErr && isAlias) {
                AliasHandle alias;
                if (FSNewAlias(0, &fref, &alias) == noErr && alias) {
                    QCFString cfstr;
                    if (FSCopyAliasInfo(alias, 0, 0, &cfstr, 0, 0) == noErr)
                        return QFileSystemEntry(QCFString::toQString(cfstr));
                }
            }
        }
    }
#endif
    return QFileSystemEntry();
}

//static
QFileSystemEntry QFileSystemEngine::canonicalName(const QFileSystemEntry &entry, QFileSystemMetaData &data)
{
    if (entry.isEmpty() || entry.isRoot())
        return entry;

#ifdef __UCLIBC__
    return QFileSystemEntry::slowCanonicalName(entry);
#else
    char *ret = 0;
# if defined(Q_OS_MAC)
    // Mac OS X 10.5.x doesn't support the realpath(X,0) extension we use here.
    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_6) {
        ret = realpath(entry.nativeFilePath().constData(), (char*)0);
    } else {
        // on 10.5 we can use FSRef to resolve the file path.
        QString path = QDir::cleanPath(entry.filePath());
        FSRef fsref;
        if (FSPathMakeRef((const UInt8 *)path.toUtf8().data(), &fsref, 0) == noErr) {
            CFURLRef urlref = CFURLCreateFromFSRef(NULL, &fsref);
            CFStringRef canonicalPath = CFURLCopyFileSystemPath(urlref, kCFURLPOSIXPathStyle);
            QString ret = QCFString::toQString(canonicalPath);
            CFRelease(canonicalPath);
            CFRelease(urlref);
            return QFileSystemEntry(ret);
        }
    }
# else
    ret = realpath(entry.nativeFilePath().constData(), (char*)0);
# endif
    if (ret) {
        data.knownFlagsMask |= QFileSystemMetaData::ExistsAttribute;
        data.entryFlags |= QFileSystemMetaData::ExistsAttribute;
        QString canonicalPath = QDir::cleanPath(QString::fromLocal8Bit(ret));
        free(ret);
        return QFileSystemEntry(canonicalPath);
    } else if (errno == ENOENT) { // file doesn't exist
        data.knownFlagsMask |= QFileSystemMetaData::ExistsAttribute;
        data.entryFlags &= ~(QFileSystemMetaData::ExistsAttribute);
        return QFileSystemEntry();
    }
    return entry;
#endif
}

//static
QFileSystemEntry QFileSystemEngine::absoluteName(const QFileSystemEntry &entry)
{
    if (entry.isAbsolute())
        return entry;

    QByteArray orig = entry.nativeFilePath();
    QByteArray result;
    if (orig.isEmpty() || !orig.startsWith('/')) {
        QFileSystemEntry cur(QFSFileEngine::currentPath());
        result = cur.nativeFilePath();
    }
    if (!orig.isEmpty() && !(orig.length() == 1 && orig[0] == '.')) {
        if (!result.isEmpty() && !result.endsWith('/'))
            result.append('/');
        result.append(orig);
    }

    if (result.length() == 1 && result[0] == '/')
        return QFileSystemEntry(result, QFileSystemEntry::FromNativePath());
    const bool isDir = result.endsWith('/');

    /* as long as QDir::cleanPath() operates on a QString we have to convert to a string here.
     * ideally we never convert to a string since that loses information. Please fix after
     * we get a QByteArray version of QDir::cleanPath()
     */
    QFileSystemEntry resultingEntry(result, QFileSystemEntry::FromNativePath());
    QString stringVersion = QDir::cleanPath(resultingEntry.filePath());
    if (isDir)
        stringVersion.append(QLatin1Char('/'));
    return QFileSystemEntry(stringVersion);
}

//static
QString QFileSystemEngine::resolveUserName(uint userId)
{
    return QString(); // TODO
}

//static
QString QFileSystemEngine::resolveGroupName(uint groupId)
{
    return QString(); // TODO
}

//static
QString QFileSystemEngine::bundleName(const QFileSystemEntry &entry)
{
#if !defined(QWS) && defined(Q_OS_MAC)
    QCFType<CFURLRef> url = CFURLCreateWithFileSystemPath(0, QCFString(entry.filePath()),
            kCFURLPOSIXPathStyle, true);
    if (QCFType<CFDictionaryRef> dict = CFBundleCopyInfoDictionaryForURL(url)) {
        if (CFTypeRef name = (CFTypeRef)CFDictionaryGetValue(dict, kCFBundleNameKey)) {
            if (CFGetTypeID(name) == CFStringGetTypeID())
                return QCFString::toQString((CFStringRef)name);
        }
    }
#endif
    return QString();
}

//static
bool QFileSystemEngine::fillMetaData(const QFileSystemEntry &entry, QFileSystemMetaData &data,
        QFileSystemMetaData::MetaDataFlags what)
{
#if !defined(QWS) && defined(Q_OS_MAC)
    if (what & QFileSystemMetaData::BundleType) {
        if (!data.hasFlags(QFileSystemMetaData::DirectoryType))
            what |= QFileSystemMetaData::DirectoryType;
    }
#endif

#if defined(Q_OS_SYMBIAN)
    if (what & QFileSystemMetaData::HiddenAttribute) {
        if (!data.hasFlags(QFileSystemMetaData::LinkType | QFileSystemMetaData::DirectoryType))
            what |= QFileSystemMetaData::LinkType | QFileSystemMetaData::DirectoryType;
    }
#endif

#if !defined(QWS) && defined(Q_OS_MAC) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
    if (what & QFileSystemMetaData::HiddenAttribute) {
        // Mac OS >= 10.5: st_flags & UF_HIDDEN
        what |= QFileSystemMetaData::PosixStatFlags;
    }
#endif

    if (what & QFileSystemMetaData::PosixStatFlags)
        what |= QFileSystemMetaData::PosixStatFlags;

    if (what & QFileSystemMetaData::ExistsAttribute) {
        //  FIXME:  Would other queries being performed provide this bit?
        what |= QFileSystemMetaData::PosixStatFlags;
    }

    data.entryFlags &= ~what;

    const char * nativeFilePath;
    int nativeFilePathLength;
    {
        const QByteArray &path = entry.nativeFilePath();
        nativeFilePath = path.constData();
        nativeFilePathLength = path.size();
    }

    bool entryExists = true; // innocent until proven otherwise

    QT_STATBUF statBuffer;
    bool statBufferValid = false;
    if (what & QFileSystemMetaData::LinkType) {
        if (QT_LSTAT(nativeFilePath, &statBuffer) == 0) {
            if (S_ISLNK(statBuffer.st_mode)) {
                data.entryFlags |= QFileSystemMetaData::LinkType;
            } else {
                statBufferValid = true;
                data.entryFlags &= ~QFileSystemMetaData::PosixStatFlags;
            }
        } else {
            entryExists = false;
        }

        data.knownFlagsMask |= QFileSystemMetaData::LinkType;
    }

    if (statBufferValid || (what & QFileSystemMetaData::PosixStatFlags)) {
        if (entryExists && !statBufferValid)
            statBufferValid = (QT_STAT(nativeFilePath, &statBuffer) == 0);

        if (statBufferValid)
            data.fillFromStatBuf(statBuffer);
        else {
            entryExists = false;
            data.creationTime_ = 0;
            data.modificationTime_ = 0;
            data.accessTime_ = 0;
            data.size_ = 0;
            data.userId_ = (uint) -2;
            data.groupId_ = (uint) -2;
        }

        // reset the mask
        data.knownFlagsMask |= QFileSystemMetaData::PosixStatFlags
            | QFileSystemMetaData::ExistsAttribute;
    }

#if !defined(QWS) && defined(Q_OS_MAC)
    if (what & QFileSystemMetaData::AliasType)
    {
        if (entryExists) {
            FSRef fref;
            if (FSPathMakeRef((const UInt8 *)nativeFilePath, &fref, NULL) == noErr) {
                Boolean isAlias, isFolder;
                if (FSIsAliasFile(&fref, &isAlias, &isFolder) == noErr) {
                    if (isAlias)
                        data.entryFlags |= QFileSystemMetaData::AliasType;
                }
            }
        }
        data.knownFlagsMask |= QFileSystemMetaData::AliasType;
    }
#endif

    if (what & QFileSystemMetaData::UserPermissions) {
        // calculate user permissions

        if (entryExists) {
            if (what & QFileSystemMetaData::UserReadPermission) {
                if (QT_ACCESS(nativeFilePath, R_OK) == 0)
                    data.entryFlags |= QFileSystemMetaData::UserReadPermission;
            }
            if (what & QFileSystemMetaData::UserWritePermission) {
                if (QT_ACCESS(nativeFilePath, W_OK) == 0)
                    data.entryFlags |= QFileSystemMetaData::UserWritePermission;
            }
            if (what & QFileSystemMetaData::UserExecutePermission) {
                if (QT_ACCESS(nativeFilePath, X_OK) == 0)
                    data.entryFlags |= QFileSystemMetaData::UserExecutePermission;
            }
        }
        data.knownFlagsMask |= (what & QFileSystemMetaData::UserPermissions);
    }

    if (what & QFileSystemMetaData::HiddenAttribute
            && !data.isHidden()) {
#if defined(Q_OS_SYMBIAN)
        // In Symbian, all symlinks have hidden attribute for some reason;
        // lets make them visible for better compatibility with other platforms.
        // If somebody actually wants a hidden link, then they are out of luck.
        if (entryExists && !data.isLink() && _q_isSymbianHidden(entry, data.isDirectory()))
            data.entryFlags |= QFileSystemMetaData::HiddenAttribute;
#else
        QString fileName = entry.fileName();
        if ((fileName.size() > 0 && fileName.at(0) == QLatin1Char('.'))
                || (entryExists && _q_isMacHidden(nativeFilePath)))
            data.entryFlags |= QFileSystemMetaData::HiddenAttribute;
#endif
        data.knownFlagsMask |= QFileSystemMetaData::HiddenAttribute;
    }

#if !defined(QWS) && defined(Q_OS_MAC)
    if (what & QFileSystemMetaData::BundleType) {
        if (entryExists && data.isDirectory()) {
            QCFType<CFStringRef> path = CFStringCreateWithBytes(0,
                    (const UInt8*)nativeFilePath, nativeFilePathLength,
                    kCFStringEncodingUTF8, false);
            QCFType<CFURLRef> url = CFURLCreateWithFileSystemPath(0, path,
                    kCFURLPOSIXPathStyle, true);

            UInt32 type, creator;
            if (CFBundleGetPackageInfoInDirectory(url, &type, &creator))
                data.entryFlags |= QFileSystemMetaData::BundleType;
        }

        data.knownFlagsMask |= QFileSystemMetaData::BundleType;
    }
#endif

    return data.hasFlags(what);
}

//static
bool QFileSystemEngine::createDirectory(const QFileSystemEntry &entry, bool createParents)
{
    QString dirName = entry.filePath();
    if (createParents) {
        dirName = QDir::cleanPath(dirName);
#if defined(Q_OS_SYMBIAN)
        dirName = QDir::toNativeSeparators(dirName);
#endif
        for (int oldslash = -1, slash=0; slash != -1; oldslash = slash) {
            slash = dirName.indexOf(QDir::separator(), oldslash+1);
            if (slash == -1) {
                if (oldslash == dirName.length())
                    break;
                slash = dirName.length();
            }
            if (slash) {
                QByteArray chunk = QFile::encodeName(dirName.left(slash));
                QT_STATBUF st;
                if (QT_STAT(chunk, &st) != -1) {
                    if ((st.st_mode & S_IFMT) != S_IFDIR)
                        return false;
                } else if (QT_MKDIR(chunk, 0777) != 0) {
                    return false;
                }
            }
        }
        return true;
    }
#if defined(Q_OS_DARWIN)  // Mac X doesn't support trailing /'s
    if (dirName.endsWith(QLatin1Char('/')))
        dirName.chop(1);
#endif
    return (QT_MKDIR(QFile::encodeName(dirName), 0777) == 0);
}

//static
bool QFileSystemEngine::removeDirectory(const QFileSystemEntry &entry, bool removeEmptyParents)
{
    if (removeEmptyParents) {
        QString dirName = QDir::cleanPath(entry.filePath());
#if defined(Q_OS_SYMBIAN)
        dirName = QDir::toNativeSeparators(dirName);
#endif
        for (int oldslash = 0, slash=dirName.length(); slash > 0; oldslash = slash) {
            QByteArray chunk = QFile::encodeName(dirName.left(slash));
            QT_STATBUF st;
            if (QT_STAT(chunk, &st) != -1) {
                if ((st.st_mode & S_IFMT) != S_IFDIR)
                    return false;
                if (::rmdir(chunk) != 0)
                    return oldslash != 0;
            } else {
                return false;
            }
            slash = dirName.lastIndexOf(QDir::separator(), oldslash-1);
        }
        return true;
    }
    return rmdir(QFile::encodeName(entry.filePath())) == 0;
}

//static
bool QFileSystemEngine::createLink(const QFileSystemEntry &source, const QFileSystemEntry &target)
{
    return (::symlink(source.nativeFilePath().constData(), target.nativeFilePath().constData()) == 0);
}

//static
bool QFileSystemEngine::copyFile(const QFileSystemEntry &source, const QFileSystemEntry &target)
{
    return false; // TODO implement;
}

//static
bool QFileSystemEngine::renameFile(const QFileSystemEntry &source, const QFileSystemEntry &target)
{
    return (::rename(source.nativeFilePath().constData(), target.nativeFilePath().constData()) == 0);
}

//static
bool QFileSystemEngine::removeFile(const QFileSystemEntry &entry)
{
    return (unlink(entry.nativeFilePath().constData()) == 0);
}

//static
bool QFileSystemEngine::setPermissions(const QFileSystemEntry &entry, QFile::Permissions permissions, QFileSystemMetaData *data)
{
    return false; // TODO implement;
}

QT_END_NAMESPACE
