import Qt 4.6

Item {
    id: Container

    property var flickableArea

    Rectangle {
        radius: 5
        color: "black"
        opacity: 0.3
        border.color: "white"
        border.width: 2
        x: 0
        y: flickableArea.pageYPosition * Container.height
        width: parent.width
        height: flickableArea.pageHeight * Container.height
    }
    states: [
        State {
            name: "show"
            when: flickableArea.moving
            PropertyChanges {
                target: Container
                opacity: 1
            }
        }
    ]
    transitions: [
        Transition {
            from: "*"
            to: "*"
            NumberAnimation {
                target: Container
                properties: "opacity"
                duration: 400
            }
        }
    ]
}
