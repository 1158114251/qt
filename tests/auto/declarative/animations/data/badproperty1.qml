import Qt 4.6

Rectangle {
    id: Wrapper
    width: 240
    height: 320
    Rectangle {
        id: MyRect
        color: "red"
        width: 50; height: 50
        x: 100; y: 100
        MouseRegion {
            anchors.fill: parent
            onClicked: if (Wrapper.state == "state1") Wrapper.state = ""; else Wrapper.state = "state1";
        }
    }
    states: State {
        name: "state1"
        PropertyChanges { target: MyRect; border.color: "blue" }
    }
    transitions: Transition {
        ColorAnimation { target: MyRect; to: "red"; property: "border.colr"; duration: 1000 }
    }
}
