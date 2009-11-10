import Qt 4.6

Rectangle {
    color: "lightSteelBlue"
    width: 300; height: 600

    ListModel {
        id: List
        ListElement { dayColor: "steelblue" }
        ListElement { dayColor: "blue" }
        ListElement { dayColor: "yellow" }
        ListElement { dayColor: "purple" }
        ListElement { dayColor: "red" }
        ListElement { dayColor: "green" }
        ListElement { dayColor: "orange" }
    }

    Flickable {
        id: Flick
        anchors.fill: parent; viewportHeight: column.height

        Column {
            id: column
            Repeater {
                model: List
                Rectangle { width: 300; height: 200; color: dayColor }
            }
        }
    }
}
