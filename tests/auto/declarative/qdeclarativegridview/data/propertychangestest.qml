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
    width: 360; height: 120; color: "white"
    Component {
        id: delegate
        Item {
            id: wrapper
            width: 180; height: 40; 
            Column {
                x: 5; y: 5
                Text { text: '<b>Name:</b> ' + name }
                Text { text: '<b>Number:</b> ' + number }
            }
        }
    }
    Component {
        id: highlightRed
        Rectangle {
            color: "red"
            radius: 10
            opacity: 0.5
        }
    }
    GridView {
        cellWidth:180
        cellHeight:40
        objectName: "gridView"
        anchors.fill: parent
        model: listModel
        delegate: delegate
        highlight: highlightRed
        focus: true
        keyNavigationWraps: true
        cacheBuffer: 10
        flow: GridView.LeftToRight
    }

    data:[  
        ListModel {
            id: listModel
            ListElement {
                name: "Bill Smith"
                number: "555 3264"
            }
            ListElement {
                name: "John Brown"
                number: "555 8426"
            }
            ListElement {
               name: "Sam Wise"
                number: "555 0473"
            }
        },
        ListModel {
            objectName: "alternateModel"
            ListElement {
                name: "Jack"
                number: "555 8426"
            }
            ListElement {
                name: "Mary"
                number: "555 3264"
            }
        }
    ]
}
 
 
