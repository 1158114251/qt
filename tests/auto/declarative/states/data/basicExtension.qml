import Qt 4.6
Rectangle {
    id: MyRectangle
    width: 100; height: 100
    color: "red"
    states: [
        State {
            name: "blue"
            PropertyChanges { target: MyRectangle; color: "blue" }
        },
        State {
            name: "bordered"
            extend: "blue"
            PropertyChanges { target: MyRectangle; border.width: 2 }
        }]
}