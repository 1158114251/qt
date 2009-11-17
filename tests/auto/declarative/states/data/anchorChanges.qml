import Qt 4.6

Rectangle {
    id: Container
    width: 200; height: 200
    Rectangle {
        id: myRect
        objectName: "MyRect"
        width: 50; height: 50
        color: "green";
        anchors.left: parent.left
        anchors.leftMargin: 5
    }
    states: State {
        name: "right"
        AnchorChanges {
            id: AncCh
            target: myRect;
            reset: "left"
            right: Container.right
        }
    }
}
