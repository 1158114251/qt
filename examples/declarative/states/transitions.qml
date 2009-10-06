import Qt 4.6

Rectangle {
    id: page
    width: 300; height: 300; color: "white"
    // A target region.  Clicking in here sets the state to '' - the default state
    Rectangle {
        x: 0; y: 0; width: 50; height: 50
        color: "transparent"; border.color: "black"
        MouseRegion { anchors.fill: parent; onClicked: { page.state='' } }
    }
    // Another target region.  Clicking in here sets the state to 'Position1'
    Rectangle {
        x: 150; y: 50; width: 50; height: 50
        color: "transparent"; border.color: "black"
        MouseRegion { anchors.fill: parent; onClicked: { page.state='Position1' } }
    }
    // Another target region.  Clicking in here sets the state to 'Position2'
    Rectangle {
        x: 0; y: 200; width: 50; height: 50
        color: "transparent"; border.color: "black"
        MouseRegion { anchors.fill: parent; onClicked: { page.state='Position2' } }
    }
    // Rect which will be moved when my state changes
    Rectangle { id: myrect; width: 50; height: 50; color: "red" }

    states: [
        // In state 'Position1', change the 'myrect' item x, y to 150, 50.
        State {
            name: "Position1"
            PropertyChanges {
                target: myrect
                x: 150
                y: 50
            }
        },
        // In state 'Position2', change y to 100.  We do not specify 'x' here -
        // it will therefore be restored to its default value of 0, if it
        // had been changed.
        State {
            name: "Position2"
            PropertyChanges {
                target: myrect
                y: 200
            }
        }
    ]

    // transitions define how the properties change.
    transitions: [
        // When transitioning to 'Position1' move x,y over a duration of 1 second,
        // with easeOutBounce easing function.
        Transition {
            from: "*"; to: "Position1"
            NumberAnimation { properties: "x,y"; easing: "easeOutBounce"; duration: 1000 }
        },
        // When transitioning to 'Position2' move x,y over a duration of 2 seconds,
        // with easeInOutQuad easing function.
        Transition {
            from: "*"; to: "Position2"
            NumberAnimation { properties: "x,y"; easing: "easeInOutQuad"; duration: 2000 }
        },
        // For any other state changes move x,y linearly over duration of 200ms.
        Transition {
            NumberAnimation { properties: "x,y"; duration: 200 }
        }
    ]
}
