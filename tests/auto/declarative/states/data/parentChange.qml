import Qt 4.6

Rectangle {
    width: 400; height: 400
    Item {
        x: 10; y: 10
        Rectangle {
            id: myRect
            objectName: "MyRect"
            x: 5
            width: 100; height: 100
            color: "red"
        }
    }
    MouseRegion {
        id: Clickable
        anchors.fill: parent
    }

    Item {
        x: -100; y: -50
        Item {
            id: newParent
            x: 248; y: 360
        }
    }

    states: State {
        name: "reparented"
        when: Clickable.pressed
        ParentChange {
            target: myRect
            parent: newParent
        }
    }
}
