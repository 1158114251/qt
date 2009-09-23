import Qt 4.6

// Here, we implement "Scale and Crop" behaviour.
//
Rectangle {
    // default size: whole image, unscaled
    width: Face.width
    height: Face.height
    color: "gray"
    clip: true

    Image {
        id: Face
        source: "pics/face.png"
        x: (parent.width-width*scale)/2
        y: (parent.height-height*scale)/2
        scale: Math.max(parent.width/width,parent.height/height)
    }
}
