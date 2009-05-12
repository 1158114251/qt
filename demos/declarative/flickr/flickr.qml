import "content"

Item {
    id: MainWindow; width: 800; height: 450

    property bool showPathView : false

    resources: [
        XmlListModel {
            id: FeedModel
            property string tags : TagsEdit.text
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
        },

        Component {
        id: PhotoDelegate
        Item {
            id: Wrapper; width: 85; height: 85
            scale: Wrapper.PathView.scale; z: Wrapper.PathView.z

            transform: [
                Rotation3D { id: Rotation; axis.startX: 30; axis.endX: 30; axis.endY: 60; angle: Wrapper.PathView.angle }
            ]

            Connection {
                sender: ImageDetails; signal: "closed()"
                script: {
                    if (Wrapper.state == 'Details') {
                        Wrapper.state = '';
                        ImageDetails.photoUrl = "";
                    }
                }
            }

            Script {
               function photoClicked() {
                   ImageDetails.photoTitle = title;
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
                    SetProperties { target: ViewModeButton; opacity: 0 }
                    SetProperties { target: TagsEdit; opacity: 0 }
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
        }
    ]

    Item {
        id: Background
    
        anchors.fill: parent

        Image { source: "content/pics/background.png"; opaque: true; anchors.fill: parent }

        GridView {
            id: PhotoGridView; model: FeedModel; delegate: PhotoDelegate; cacheBuffer: 100
            cellWidth: 105; cellHeight: 105; x:32; y: 80; width: 800; height: 330; z: 1
        }

        PathView {
            id: PhotoPathView; model: FeedModel; delegate: PhotoDelegate
            y: -380; width: 800; height: 330; pathItemCount: 10; z: 1
            path: Path {
                startX: -50; startY: 40;

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
                PathAttribute { name: "angle"; value: 45 }
            }

        }

        ImageDetails { id: ImageDetails; width: 750; x: 25; y: 500; height: 410 }

        MediaButton {
            id: ViewModeButton; x: 680; y: 410; text: "View Mode"
            onClicked: { if (MainWindow.showPathView == true) MainWindow.showPathView = false; else MainWindow.showPathView = true }
        }

        MediaButton {
            id: FetchButton
            text: "Update"
            anchors.right: ViewModeButton.left; anchors.rightMargin: 5
            anchors.top: ViewModeButton.top
            onClicked: { FeedModel.reload(); }
        }

        MediaLineEdit {
            id: TagsEdit;
            label: "Tags"
            anchors.right: FetchButton.left; anchors.rightMargin: 5
            anchors.top: ViewModeButton.top
        }

        states: [
            State {
                name: "PathView"
                when: MainWindow.showPathView == true
                SetProperties { target: PhotoPathView; y: 80 }
                SetProperties { target: PhotoGridView; y: -380 }
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
