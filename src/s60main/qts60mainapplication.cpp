/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Symbian application wrapper of the Qt Toolkit.
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
** In addition, as a special exception, Nokia gives you certain
** additional rights.  These rights are described in the Nokia Qt LGPL
** Exception version 1.1, included in the file LGPL_EXCEPTION.txt in this
** package.
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

// INCLUDE FILES
#include <exception>
#include "qts60maindocument_p.h"
#include "qts60mainapplication_p.h"
#include <bautils.h>
#include <coemain.h>

// ============================ MEMBER FUNCTIONS ===============================


_LIT(KQtWrapperResourceFile, "\\resource\\apps\\s60main.rsc");

// -----------------------------------------------------------------------------
// CQtS60MainApplication::CreateDocumentL()
// Creates CApaDocument object
// -----------------------------------------------------------------------------
//
CApaDocument* CQtS60MainApplication::CreateDocumentL()
{
    // Create an QtS60Main document, and return a pointer to it
    return (static_cast<CApaDocument*>(CQtS60MainDocument::NewL(*this)));
}

// -----------------------------------------------------------------------------
// CQtS60MainApplication::AppDllUid()
// Returns application UID
// -----------------------------------------------------------------------------
//
TUid CQtS60MainApplication::AppDllUid() const
{
    // Return the UID for the QtS60Main application
    return ProcessUid();
}

// -----------------------------------------------------------------------------
// CQtS60MainApplication::ResourceFileName()
// Returns application resource filename
// -----------------------------------------------------------------------------
//
TFileName CQtS60MainApplication::ResourceFileName() const
{
    TFindFile finder(iCoeEnv->FsSession());
    TInt err = finder.FindByDir(KQtWrapperResourceFile, KNullDesC);
    if (err == KErrNone)
        return finder.File();
    return KNullDesC();
}


// End of File
