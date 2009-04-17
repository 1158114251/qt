/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the test suite of the Qt Toolkit.
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
#ifndef TESTCOMPILER_H
#define TESTCOMPILER_H


#ifdef QT3_SUPPORT

#include <qobject.h>
#include <qstringlist.h>

QT_FORWARD_DECLARE_CLASS(Q3Process)

#define COMPILE_ERROR "Compile error"
#define COMPILE_SUCCESS "Compile successfull"
#define COMPILE_NOT_AVAIL "Binary not available for testing"
#define SELF_TEST "self-test"

enum BuildType { Exe, Dll, Lib, Plain };

class TestCompiler : public QObject
{
Q_OBJECT

public:
    TestCompiler();
    virtual ~TestCompiler();

    void setBaseCommands( QString makeCmd, QString qmakeCmd, bool qwsMode );

    // builds a complete project, e.g. qmake, make clean, make and exists.
    bool buildProject( const QString &project, BuildType buildType, const QString &targetName, const QString &destPath, const QString &version );

    // executes a make clean in the specified workPath
    bool makeClean( const QString &workPath );
    // executes a make dist clean in the specified workPath
    bool makeDistClean( const QString &workPath );
    // executes a qmake on proName in the specified workDir, output goes to buildDir or workDir if it's null
    bool qmake( const QString &workDir, const QString &proName, const QString &buildDir = QString() );
    // executes a make in the specified workPath, with an optional target (eg. install)
    bool make( const QString &workPath, const QString &target = QString() );
    // executes a make clean and then deletes the makefile in workpath + deletes the executable
    // in destPath.
    bool cleanAll( const QString &workPath, const QString &destPath, const QString &exeName, const QString &exeExt );
    // checks if the executable exists in destDir
    bool exists( const QString &destDir, const QString &exeName, BuildType buildType, const QString &version );
    // removes the makefile
    bool removeMakefile( const QString &workPath );

private:
    QString     make_cmd;
    QString     qmake_cmd;

    Q3Process	*childProc;
    QStringList env_list;

    bool	child_show;
    bool        qws_mode;
    bool	exit_ok;

private:
    bool runChild( bool showOutput, QStringList argList, QStringList *envList );
    void addMakeResult( const QString &result );
    QStringList make_result;

private slots:
    void childReady();
    void childHasData();
};

#endif // QT3_SUPPORT
#endif // TESTCOMPILER_H
