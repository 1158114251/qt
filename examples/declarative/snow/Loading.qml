import Qt 4.6

Image {
    id: loading; source: "pics/loading.png"; transformOrigin: "Center"
    rotation: NumberAnimation {
        id: rotationAnimation; from: 0; to: 360; running: loading.visible == true; repeat: true; duration: 900
    }
}
