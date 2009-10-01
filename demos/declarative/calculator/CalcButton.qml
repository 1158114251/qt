import Qt 4.6

Rectangle {
    property alias operation: Label.text
    property bool toggable: false
    property bool toggled: false
    signal clicked

    id: Button; width: 50; height: 30
    border.color: Palette.mid; radius: 6
    gradient: Gradient {
        GradientStop { id: G1; position: 0.0; color: Palette.lighter(Palette.button) }
        GradientStop { id: G2; position: 1.0; color: Palette.button }
    }

    Text { id: Label; anchors.centerIn: parent; color: Palette.buttonText }

    MouseRegion {
        id: ClickRegion
        anchors.fill: parent
        onClicked: {
            doOp(operation);
            Button.clicked();
            if (!Button.toggable) return;
            Button.toggled ? Button.toggled = false : Button.toggled = true
        }
    }

    states: [
        State {
            name: "Pressed"; when: ClickRegion.pressed == true
            PropertyChanges { target: G1; color: Palette.dark }
            PropertyChanges { target: G2; color: Palette.button }
        },
        State {
            name: "Toggled"; when: Button.toggled == true
            PropertyChanges { target: G1; color: Palette.dark }
            PropertyChanges { target: G2; color: Palette.button }
        }
    ]
}
