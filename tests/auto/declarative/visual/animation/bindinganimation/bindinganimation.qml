import Qt 4.6

Rectangle {
    color: "blue"
    width: 320
    height: 240
    id: page
    Rectangle {
        id: myRectangle
        width: 100
        height: 100
        color: "red"
        x: 10
    }
    states: [
        State {
            name: "hello"
            PropertyChanges {
                target: myRectangle
                x: 50 + 50
            }
            PropertyChanges {
                target: myMouseRegion
                onClicked: page.state = ''
            }
        }
    ]
    transitions: [
        Transition {
            NumberAnimation {
                matchProperties: "x"
            }
        }
    ]
    MouseRegion {
        id: myMouseRegion
        anchors.fill: parent
        onClicked: { page.state= 'hello' }
    }
}
