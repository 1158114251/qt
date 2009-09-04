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

#ifndef QS60MAINAPPUI_P_H
#define QS60MAINAPPUI_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// INCLUDES
#include <aknappui.h>

#include <qglobal.h>

QT_BEGIN_NAMESPACE

// FORWARD DECLARATIONS

// CLASS DECLARATION
/**
* QS60MainAppUi application UI class.
* Interacts with the user through the UI and request message processing
* from the handler class
*/
class QS60MainAppUi : public CAknAppUi
{
public: // Constructors and destructor

    /**
     * ConstructL.
     * 2nd phase constructor.
     */
    void ConstructL();

    /**
     * QS60MainAppUi.
     * C++ default constructor. This needs to be public due to
     * the way the framework constructs the AppUi
     */
    QS60MainAppUi();

    /**
     * ~QS60MainAppUi.
     * Virtual Destructor.
     */
    virtual ~QS60MainAppUi();

protected:
    void RestoreMenuL(CCoeControl* aMenuWindow,TInt aMenuId,TMenuType aMenuType);
    void DynInitMenuBarL(TInt aResourceId, CEikMenuBar *aMenuBar);
    void DynInitMenuPaneL(TInt aResourceId, CEikMenuPane *aMenuPane);

private:  // Functions from base classes

    /**
     * From CEikAppUi, HandleCommandL.
     * Takes care of command handling.
     * @param aCommand Command to be handled.
     */
    void HandleCommandL( TInt aCommand );

    /**
     * From CAknAppUi, HandleResourceChangeL
     * Handles resource change events such as layout switches in global level.
     * @param aType event type.
     */
    void HandleResourceChangeL(TInt aType);

    /**
     *  HandleStatusPaneSizeChange.
     *  Called by the framework when the application status pane
     *  size is changed.
     */
    void HandleStatusPaneSizeChange();

protected:
    void HandleWsEventL(const TWsEvent& aEvent, CCoeControl* aDestination);
};

QT_END_NAMESPACE

#endif // QS60MAINAPPUI_P_H

// End of File
