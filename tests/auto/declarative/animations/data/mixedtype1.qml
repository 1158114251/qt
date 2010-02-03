import Qt 4.6

Rectangle {
    id: wrapper
    width: 240
    height: 320
    Rectangle {
        id: myRect
        objectName: "myRect"
        color: "red"
        width: 50; height: 50
        x: 100; y: 100
        MouseRegion {
            anchors.fill: parent
            onClicked: if (wrapper.state == "state1") wrapper.state = ""; else wrapper.state = "state1";
        }
    }
    states: State {
        name: "state1"
        PropertyChanges { target: myRect; x: 200; border.width: 10 }
    }
    transitions: Transition {
        PropertyAnimation { matchProperties: "x,border.width"; duration: 1000 } //x is real, border.width is int
    }
}
