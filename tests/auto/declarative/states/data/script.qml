import Qt 4.6
Rectangle {
    id: MyRectangle
    width: 100; height: 100
    color: "red"
    states: State {
        name: "blue"
        StateChangeScript { script: MyRectangle.color = "blue"; }
    }
}
