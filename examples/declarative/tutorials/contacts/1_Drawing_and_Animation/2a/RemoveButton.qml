//! [0]
Rect {
    id: removeButton
    width: 30
    height: 30
    color: "red"
    radius: 5
    children: [
        Image {
            id: trashIcon
            width: 22
            height: 22
            anchors.right: parent.right
            anchors.rightMargin: 4
            anchors.verticalCenter: parent.verticalCenter
            src: "../../shared/pics/trash.png"
        }
    ]
}
//! [0]
