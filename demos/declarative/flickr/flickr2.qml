import "content"

Item {
    id: MainWindow; width: 800; height: 450

    property bool showPathView : false

        VisualModel {
            id: MyVisualModel
            model:
                XmlListModel {
                    id: FeedModel
                    property string tags : ""
                    source: "http://api.flickr.com/services/feeds/photos_public.gne?"+(tags ? "tags="+tags+"&" : "")+"format=rss2"
                    query: "doc($src)/rss/channel/item"
                    namespaceDeclarations: "declare namespace media=\"http://search.yahoo.com/mrss/\";"

                    Role { name: "title"; query: "title/string()" }
                    Role { name: "imagePath"; query: "media:thumbnail/@url/string()" }
                    Role { name: "url"; query: "media:content/@url/string()" }
                    Role { name: "description"; query: "description/string()"; isCData: true }
                    Role { name: "tags"; query: "media:category/string()" }
                    Role { name: "photoWidth"; query: "media:content/@width/string()" }
                    Role { name: "photoHeight"; query: "media:content/@height/string()" }
                    Role { name: "photoType"; query: "media:content/@type/string()" }
                    Role { name: "photoAuthor"; query: "author/string()" }
                    Role { name: "photoDate"; query: "pubDate/string()" }
                }

            delegate: Package {
                Item {
                    id: Wrapper; width: 85; height: 85
                    scale: Wrapper.PathView.scale; z: Wrapper.PathView.z

                    transform: [
                        Rotation3D { id: Rotation; axis.startX: 30; axis.endX: 30; axis.endY: 60; angle: Wrapper.PathView.angle }
                    ]

                    Connection {
                        sender: ImageDetails; signal: "closed()"
                        script: { if (Wrapper.state == 'Details') Wrapper.state = '' }
                    }

                    Script {
                        function photoClicked() {
                            ImageDetails.photoTitle = title;
                            ImageDetails.flickableArea.yPosition = 0;
                            ImageDetails.fullScreenArea.source = "";
                            ImageDetails.photoDescription = description;
                            ImageDetails.photoTags = tags;
                            ImageDetails.photoWidth = photoWidth;
                            ImageDetails.photoHeight = photoHeight;
                            ImageDetails.photoType = photoType;
                            ImageDetails.photoAuthor = photoAuthor;
                            ImageDetails.photoDate = photoDate;
                            ImageDetails.photoUrl = url;
                            ImageDetails.rating = 0;
                            Wrapper.state = "Details";
                        }
                    }

                    Rect {
                        id: WhiteRect; anchors.fill: parent; color: "white"; radius: 5

                        Loading { x: 26; y: 26; visible: Thumb.status }
                        Image { id: Thumb; source: imagePath; x: 5; y: 5 }

                        Item {
                            id: Shadows
                            Image { source: "content/pics/shadow-right.png"; x: WhiteRect.width; height: WhiteRect.height }
                            Image { source: "content/pics/shadow-bottom.png"; y: WhiteRect.height; width: WhiteRect.width }
                            Image { id: Corner; source: "content/pics/shadow-corner.png"; x: WhiteRect.width; y: WhiteRect.height }
                        }
                    }

                    MouseRegion { anchors.fill: Wrapper; onClicked: { photoClicked() } }

                    states: [
                        State {
                            name: "Details"
                            SetProperties { target: ImageDetails; z: 2 }
                            ParentChange { target: Wrapper; parent: ImageDetails.frontContainer }
                            SetProperties { target: Wrapper; x: 45; y: 35; scale: 1; z: 1000 }
                            SetProperties { target: Rotation; angle: 0 }
                            SetProperties { target: Shadows; opacity: 0 }
                            SetProperties { target: ImageDetails; y: 20 }
                            SetProperties { target: PhotoGridView; y: "-480" }
                            SetProperties { target: PhotoPathView; y: "-480" }
                            SetProperties { target: CloseButton; opacity: 0 }
                            SetProperties { target: FetchButton; opacity: 0 }
                            SetProperties { target: CategoryText; y: "-50" }
                        }
                    ]

                    transitions: [
                        Transition {
                            fromState: "*"; toState: "Details"
                            ParentChangeAction { }
                            NumericAnimation { properties: "x,y,scale,opacity,angle"; duration: 500; easing: "easeInOutQuad" }
                        },
                        Transition {
                            fromState: "Details"; toState: "*"
                            SequentialAnimation {
                                ParentChangeAction { }
                                NumericAnimation { properties: "x,y,scale,opacity,angle"; duration: 500; easing: "easeInOutQuad" }
                                SetPropertyAction { filter: Wrapper; properties: "z" }
                            }
                        }
                    ]
                }

                Item {
                    Package.name: "rightBox"
                    id: RightBox; x: 200; width: 85; height: 85
                }

                Item {
                    Package.name: "leftBox"
                    id: LeftBox; width: 85; height: 85
                }

                Item {
                id: MyItem
                states: [
                    State {
                        name: "left"
                        when: MainWindow.showPathView == true
                        SetProperty {
                            target: Wrapper
                            property: "moveToParent"
                            value: LeftBox
                        }
                    },
                    State {
                        name: "right"
                        when: MainWindow.showPathView == false
                            SetProperty {
                            target: Wrapper
                            property: "moveToParent"
                            value: RightBox
                        }
                    }
                ]
                transitions: [
                    Transition {
                        fromState: "left"
                        toState: "right"
                        SequentialAnimation {
                            SetPropertyAction {
                                target: Wrapper
                                property: "moveToParent"
                            }
                            ParallelAnimation {
                                NumericAnimation {
                                    target: Wrapper
                                    properties: "x,y"
                                    to: 0
                                    easing: "easeOutQuad"
                                    duration: 350
                                }
                            }
                        }
                    },
                    Transition {
                        fromState: "right"
                        toState: "left"
                        SequentialAnimation {
                            PauseAnimation {
                                duration: Math.floor(index/7)*100
                            }
                            SetPropertyAction {
                                target: Wrapper
                                property: "moveToParent"
                            }
                            ParallelAnimation {
                                NumericAnimation {
                                    target: Wrapper
                                    properties: "x,y"
                                    to: 0
                                    easing: "easeOutQuad"
                                    duration: 250
                                }
                            }
                        }
                    }
                ]
                state: "right"
            }

            }
        }


    Item {
        id: Background

        Image { source: "content/pics/background.png"; opaque: true }

        GridView {
            id: PhotoGridView; model: MyVisualModel.parts.leftBox
            cellWidth: 105; cellHeight: 105; x:32; y: 80; width: 800; height: 330; z: 1
            cacheBuffer: 100
        }

        PathView {
            id: PhotoPathView; model: MyVisualModel.parts.rightBox
            y: 80; width: 800; height: 330; z: 1
            pathItemCount: 10; snapPosition: 0.001
            path: Path {
                startX: -150; startY: 40;

                PathPercent { value: 0 }
                PathLine { x: -50; y: 40 }

                PathPercent { value: 0.001 }
                PathAttribute { name: "scale"; value: 1 }
                PathAttribute { name: "angle"; value: -45 }

                PathCubic {
                    x: 400; y: 220
                    control1X: 140; control1Y: 40
                    control2X: 210; control2Y: 220
                }

                PathAttribute { name: "scale"; value: 1.2  }
                PathAttribute { name: "z"; value: 1 }
                PathAttribute { name: "angle"; value: 0 }

                PathCubic {
                    x: 850; y: 40
                    control2X: 660; control2Y: 40
                    control1X: 590; control1Y: 220
                }

                PathAttribute { name: "scale"; value: 1 }
                PathAttribute { name: "angle"; value: -45 }

                PathPercent { value: 0.999 }
                PathLine { x: 950; y: 40 }
                PathPercent { value: 1.0 }
            }

        }

        ImageDetails { id: ImageDetails; width: 750; x: 25; y: 500; height: 410 }

        MediaButton {
            id: CloseButton; x: 680; y: 410; text: "View Mode"
            onClicked: { if (MainWindow.showPathView == true) MainWindow.showPathView = false; else MainWindow.showPathView = true }
        }

        MediaButton {
            id: FetchButton
            text: "Update"
            anchors.right: CloseButton.left; anchors.rightMargin: 5
            anchors.top: CloseButton.top
            onClicked: { FeedModel.reload(); }
        }

        states: [
            State {
                name: "PathView"
            }
        ]

        transitions: [
            Transition {
                fromState: "*"; toState: "*"
                NumericAnimation { properties: "y"; duration: 650; easing: "easeOutBounce(amplitude:0.1)" }
            }
        ]
    }

    Text {
        id: CategoryText;  anchors.horizontalCenter: parent.horizontalCenter; y: 15;
        text: "Flickr - " +
            (FeedModel.tags=="" ? "Uploads from everyone" : "Recent Uploads tagged " + FeedModel.tags)
        font.size: 16; font.bold: true; color: "white"; style: "Raised"; styleColor: "black"
    }
}
