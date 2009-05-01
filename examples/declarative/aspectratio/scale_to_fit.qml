// The Image primitive does not have any special handling for maintaining
// aspect ratio. This example shows that you can provide whatever specific
// behaviour you like.
//
// Here, we implement "Scale to Fit" behaviour.
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
        scale: Math.min(parent.width/width,parent.height/height)
    }
}
