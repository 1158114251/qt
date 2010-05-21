/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
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

import Qt 4.7

Rectangle {
    id: container
    width: 200; height: 200
    Rectangle {
        id: myRect
        objectName: "MyRect"
        color: "green";
        anchors.left: parent.left
        anchors.right: rightGuideline.left
        anchors.top: topGuideline.top
        anchors.bottom: container.bottom
    }
    Item { objectName: "LeftGuideline"; id: leftGuideline; x: 10 }
    Item { id: rightGuideline; x: 150 }
    Item { id: topGuideline; y: 10 }
    Item { objectName: "BottomGuideline"; id: bottomGuideline; y: 150 }
    states: State {
        name: "reanchored"
        AnchorChanges {
            target: myRect;
            anchors.left: leftGuideline.left
            anchors.right: container.right
            anchors.top: container.top
            anchors.bottom: bottomGuideline.bottom
        }
    }
}
