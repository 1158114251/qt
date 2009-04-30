PathView {
    id: Container; pathItemCount: 6

    path: Path {
        startX: -50; startY: 40;

        PathAttribute { name: "scale"; value: 0.2 }

        PathCubic {
            x: 400; y: 220
            control1X: 140; control1Y: 40
            control2X: 210; control2Y: 220
        }

        PathAttribute { name: "scale"; value: 1.2  }
        PathAttribute { name: "z"; value: 1 }

        PathCubic {
            x: 850; y: 40
            control2X: 660; control2Y: 40
            control1X: 590; control1Y: 220
        }

        PathAttribute { name: "scale"; value: 0.2 }
    }

    delegate: Component {
        id: PhoneDelegate

        Item {
            id: Wrapper; width: 320; height: 200
            scale: Wrapper.PathView.scale; z: Wrapper.PathView.z

            Connection {
                sender: PhoneInfoContainer; signal: "closed()"
                script: { if (Wrapper.state == 'Details') Wrapper.state = '' }
            }

            Script {
               function phoneClicked() {
                  if (MainWindow.minimized == true) {
                     MainWindow.minimized = false;
                  } else {
                     PhoneInfoContainer.phoneTitle = title;
                     PhoneInfoContainer.flickableArea.yPosition = 0;
                     PhoneInfoContainer.phoneDescription = description;
                     PhoneInfoContainer.phoneSpecifications = specifications;
                     PhoneInfoContainer.phoneUrl = url;
                     PhoneInfoContainer.rating = rating;
                     Wrapper.state = "Details";
                  }
               }
            }

            Rect {
                id: Dvd; anchors.fill: parent; color: "white"

                Item {
                    x: (parent.width-width)/2
                    y: (parent.height-height)/2
                    width: Thumb.width*Thumb.scale
                    height: Thumb.height*Thumb.scale

                    Image {
                        id: Thumb; source: thumb
                        scale: 0.95*Math.min(Dvd.height/height,Dvd.width/width)
                    }
                }

                Image { source: "pics/shadow-right.png"; x: Dvd.width; height: Dvd.height }
                Image { source: "pics/shadow-bottom.png"; y: Dvd.height; width: Dvd.width }

                Image {
                    id: Corner
                    source: "pics/shadow-corner.png"
                    x: Dvd.width; y: Dvd.height
                }
            }

            MouseRegion { anchors.fill: Wrapper; onClicked: { phoneClicked() } }

            states: [
                State {
                    name: "Details"
                    ParentChange { target: Wrapper; parent: PhoneInfoContainer.frontContainer }
                    SetProperties { target: Wrapper; x: 50; y: 60; scale: 1 }
                    SetProperties { target: PhoneInfoContainer; y: 20 }
                    SetProperties { target: Container; y: "-480" }
                    SetProperties { target: CloseButton; opacity: 0 }
                    SetProperties { target: CategoryText; y: "-50" }
                },

                State {
                    name: "Stacked"
                    when: MainWindow.minimized == true
                    ParentChange { target: Wrapper; parent: Stack }
                    SetProperties {target: Wrapper; x: 0; y: 0; scale: 0.2 }
                    SetProperties { target: CloseButton; opacity: 0 }
                    SetProperties { target: CategoryText; y: "-50" }
                }
            ]

            transitions: [
                Transition {
                    fromState: ""; toState: "Details,Stacked"
                    ParentChangeAction { }
                    NumericAnimation { properties: "x,y,scale,opacity"; duration: 500; easing: "easeInOutQuad" }
                },

                Transition {
                    fromState: "Details,Stacked"
                    toState: ""
                    ParentChangeAction { }
                    NumericAnimation { properties: "x,y,scale,opacity"; duration: 500; easing: "easeInOutQuad" }
                }
            ]

        }
    }
}
