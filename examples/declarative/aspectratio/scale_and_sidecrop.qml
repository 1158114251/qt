import Qt 4.6

// Here, we implement a variant of "Scale and Crop" behaviour, where we
// crop the sides if necessary to fully fit vertically, but not the reverse.
//
Rect {
    // default size: whole image, unscaled
    width: Image.width
    height: Image.height
    color: "gray"
    clip: true

    Image {
        id: Image
        source: "pics/face.png"
        x: (parent.width-width*scale)/2
        y: (parent.height-height*scale)/2
        scale: parent.height/height
    }
}
