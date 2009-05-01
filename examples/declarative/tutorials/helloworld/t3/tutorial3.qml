Rect {
    id: Page
    width: 480
    height: 200
    color: "white"
    Text {
        id: HelloText
        text: "Hello world!"
        font.size: 24
        font.bold: true
        y: 30
        anchors.horizontalCenter: Page.horizontalCenter
        states: [
            State {
                name: "down"
                when: MouseRegion.pressed == true
                SetProperty {
                    target: HelloText
                    property: "y"
                    value: 160
                }
                SetProperty {
                    target: HelloText
                    property: "color"
                    value: "red"
                }
            }
        ]
        transitions: [
            Transition {
                fromState: "*"
                toState: "down"
                reversible: true
                ParallelAnimation {
                    NumericAnimation {
                        properties: "y"
                        duration: 500
                        easing: "easeOutBounce"
                    }
                    ColorAnimation { duration: 500 }
                }
            }
        ]
    }
    MouseRegion { id: MouseRegion; anchors.fill: HelloText }
    GridLayout {
        id: ColorPicker
        x: 0
        anchors.bottom: Page.bottom
        width: 120; height: 50
        rows: 2; columns: 3
        Cell { color: "#ff0000" }
        Cell { color: "#00ff00" }
        Cell { color: "#0000ff" }
        Cell { color: "#ffff00" }
        Cell { color: "#00ffff" }
        Cell { color: "#ff00ff" }
    }
}
