import Qt 4.6

// Here, we implement "Scale to Fit" behaviour, using the
// fillMode property.
//
Rectangle {
    // default size: whole image, unscaled
    width: face.width
    height: face.height
    color: "gray"
    clip: true

    Image {
        id: face
        source: "pics/face.png"
        fillMode: "PreserveAspectFit"
        anchors.fill: parent
    }
}
