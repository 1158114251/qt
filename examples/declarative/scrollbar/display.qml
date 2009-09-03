import Qt 4.6

Rectangle {
    width: 640
    height: 480
    // Create a flickable to view a large image.
    Flickable {
        id: View
        anchors.fill: parent
        Image {
            id: Picture
            source: "pics/niagara_falls.jpg"
        }
        viewportWidth: Picture.width
        viewportHeight: Picture.height
        // Only show the scrollbars when the view is moving.
        states: [
            State {
                name: "ShowBars"
                when: View.moving
                PropertyChanges { target: SBV; opacity: 1 }
                PropertyChanges { target: SBH; opacity: 1 }
            }
        ]
        transitions: [
            Transition {
                from: "*"
                to: "*"
                NumberAnimation {
                    properties: "opacity"
                    duration: 400
                }
            }
        ]
    }
    // Attach scrollbars to the right and bottom edges of the view.
    ScrollBar {
        id: SBV
        opacity: 0
        orientation: "Vertical"
        position: View.visibleArea.yPosition
        pageSize: View.visibleArea.heightRatio
        width: 12
        height: View.height-12
        anchors.right: View.right
    }
    ScrollBar {
        id: SBH
        opacity: 0
        orientation: "Horizontal"
        position: View.visibleArea.xPosition
        pageSize: View.visibleArea.widthRatio
        height: 12
        width: View.width-12
        anchors.bottom: View.bottom
    }
}
