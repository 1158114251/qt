import Qt 4.6

// Here, we extend the "face_fit" example with animation to show how truly
// diverse and usage-specific behaviours are made possible by NOT putting a
// hard-coded aspect ratio feature into the Image primitive.
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
        scale: SpringFollow {
            source: Math.max(Math.min(Face.parent.width/Face.width*1.333,Face.parent.height/Face.height),
                        Math.min(Face.parent.width/Face.width,Face.parent.height/Face.height*1.333))
            spring: 1
            damping: 0.05
        }
    }
}
