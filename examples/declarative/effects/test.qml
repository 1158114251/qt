import Qt 4.6

Rectangle {
    color: "white"
    width: 800
    height: 600

    Image {
        source: "pic.jpg"

        effect: Blur {
            blurRadius: NumberAnimation { id: blur; from: 0; to: 10; duration: 200; repeat: true }
        }

        MouseRegion { anchors.fill: parent; onClicked: blur.running = !blur.running }

        Text { color: "white"; text: "Blur" }
    }

    Image {
        source: "pic.jpg"

        x: 200
        effect: Grayscale {}

        Text { color: "white"; text: "Grayscale" }
    }

    Image {
        source: "pic.jpg"

        x: 400
        effect: Colorize { color: "blue" }

        Text { color: "white"; text: "Colorize" }
    }

    Image {
        source: "pic.jpg"

        y: 300
        effect: Pixelize {
            pixelSize: NumberAnimation { id: pixelize; from: 0; to: 10; duration: 200; repeat: true }
        }

        MouseRegion { anchors.fill: parent; onClicked: pixelize.running = !pixelize.running }

        Text { color: "white"; text: "Pixelize" }
    }


    Image {
        source: "pic.jpg"

        x: 200
        y: 300
        effect: DropShadow {
            blurRadius: 3
            offset.x: 3
            offset.y: NumberAnimation { id: dropShadow; from: 0; to: 10; duration: 200; repeat: true; }
        }

        MouseRegion { anchors.fill: parent; onClicked: dropShadow.running = !dropShadow.running }

        Text { color: "white"; text: "DropShadow" }
    }


    Image {
        source: "pic.jpg"

        x: 400
        y: 300
        effect: Bloom {
            blurRadius: 3
            brightness: 128
            strength: NumberAnimation { id: bloom; from: 0; to: 1; duration: 200; repeat: true; }
        }

        MouseRegion { anchors.fill: parent; onClicked: bloom.running = !bloom.running }

        Text { color: "white"; text: "Bloom" }
    }

    Text {
        x: 100; y: 250
        text: "Clicking Blur, Pixelize or DropShadow will \ntoggle animation."
        color: "black"
    }

}
