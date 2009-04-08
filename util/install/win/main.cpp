/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the utils of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
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
#include <qapplication.h>
#include <qmessagebox.h>
#include "setupwizardimpl.h"
#include "resource.h"
#include "globalinformation.h"
#include "environment.h"

#if defined Q_OS_WIN32
#include "archive.h"
#endif

GlobalInformation globalInformation;
SetupWizardImpl *wizard = 0;

int main( int argc, char** argv )
{
    QApplication app( argc, argv );
    int res( -1 );

    for( int i = 0; i < app.argc(); i++ ) {
	if( QString( app.argv()[i] ) == "-reconfig" ) {
	    globalInformation.setReconfig( true );

	    QString qmakespec = QEnvironment::getEnv( "QMAKESPEC" );
	    for (int mks = 0; mks <= GlobalInformation::MACX; ++mks) {
		if (globalInformation.text(GlobalInformation::Mkspec) == qmakespec) {
		    globalInformation.setSysId((GlobalInformation::SysId)mks);
		    break;
		}
	    }

	    if ( ++i < app.argc() ) {
		globalInformation.setQtVersionStr( app.argv()[i] );
	    }
	    break;
#if defined(Q_OS_WIN32)
	} else if ( QString( app.argv()[i] ) == "-add-archive" ) {
	    // -add-archive is an internal option to add the
	    // binary resource QT_ARQ
	    if ( ++i < app.argc() ) {
		if ( addArchive( app.argv()[i] ) )
		    return 0;
	    }
	    return res;
#endif
	}
    }

    wizard = new SetupWizardImpl( 0, 0, false, Qt::WStyle_NormalBorder | Qt::WStyle_Customize | Qt::WStyle_MinMax | Qt::WStyle_SysMenu | Qt::WStyle_Title );
    wizard->show();

    app.setMainWidget( wizard );
    res = app.exec();

    wizard->stopProcesses();

    //### memory leak

    return res;
}
