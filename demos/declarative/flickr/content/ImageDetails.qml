Flipable {
    id: Container

    property var frontContainer: ContainerFront
    property var flickableArea: Flickable
    property var fullScreenArea: BigImage
    property string photoTitle: ""
    property string photoDescription: ""
    property string photoTags: ""
    property int photoWidth
    property int photoHeight
    property string photoType
    property string photoAuthor
    property string photoDate
    property string photoUrl
    property int rating: 2

    signal closed

    axis: Axis { startX: Container.width / 2; endX: Container.width / 2; endY: 1 }

    front: Item {
        id: ContainerFront; anchors.fill: Container

        Rect {
            anchors.fill: parent
            color: "black"; opacity: 0.4
            pen.color: "white"; pen.width: 2
        }

        MediaButton {
            id: BackButton; x: 630; y: 370; text: "Back"
            onClicked: { Container.closed.emit() }
        }

        MediaButton {
            id: MoreButton; x: 530; y: 370; text: "View..."
            onClicked: { Container.state='Back' }
        }

        Text { id: TitleText; style: "Raised"; styleColor: "black"; color: "white"; elide: "ElideRight"
               x: 220; y: 30; width: parent.width - 240; text: Container.photoTitle; font.size: 22 }

        LikeOMeter { x: 40; y: 250; rating: Container.rating }

        Flickable { id: Flickable; x: 220; width: 480; height: 230; y: 120; clip: true
                    viewportWidth: 480; viewportHeight: DescriptionText.height

            WebView { id: DescriptionText; width: parent.width
                      html: "<style TYPE=\"text/css\">body {color: white;} a:link {color: cyan; text-decoration: underline; }</style>" + Container.photoDescription }
        }

        Text { id: Size; color: "white"; width: 300; x: 40; y: 300
               text: "<b>Size:</b> " + Container.photoWidth + 'x' + Container.photoHeight }
        Text { id: Type; color: "white"; width: 300; x: 40; anchors.top: Size.bottom
               text: "<b>Type:</b> " + Container.photoType }

        Text { id: Author; color: "white"; width: 300; x: 220; y: 80
               text: "<b>Author:</b> " + Container.photoAuthor }
        Text { id: Date; color: "white"; width: 300; x: 220; anchors.top: Author.bottom
               text: "<b>Published:</b> " + Container.photoDate }
        Text { id: TagsLabel; color: "white"; x: 220; anchors.top: Date.bottom;
               text: Container.photoTags == "" ? "" : "<b>Tags:</b> " }
        Text { id: Tags; color: "white"; width: parent.width-x-20; anchors.left: TagsLabel.right; anchors.top: Date.bottom; elide: "ElideRight"
               text: Container.photoTags == "" ? "" : Container.photoTags }

        ScrollBar { id: ScrollBar; x: 720; y: Flickable.y; width: 7; height: Flickable.height; opacity: 0;
                    flickableArea: Flickable; clip: true }
    }

    back: Item {
        anchors.fill: Container

        Rect { anchors.fill: parent; color: "black"; opacity: 0.4; pen.color: "white"; pen.width: 2 }

        Loading { anchors.centeredIn: parent; visible: BigImage.status }
        Flickable {
            id: Flick; width: Container.width - 10; height: Container.height - 10
            x: 5; y: 5; clip: true; viewportWidth: (BigImage.width * BigImage.scale) + BigImage.x;
            viewportHeight: BigImage.height * BigImage.scale

            Image {
                id: BigImage; source: Container.photoUrl; scale: Slider.value
                x:Math.max(0, ((Flick.width/2)-(width * scale / 2)));
                y:Math.max(0, (Flick.height/2)-(height * scale / 2));
            }
        }

        MediaButton {
            id: BackButton2; x: 630; y: 370; text: "Back"; onClicked: { Container.state = '' }
        }

        Slider { id: Slider; x: 25; y: 374; imageWidth: Container.photoWidth; imageHeight: Container.photoHeight }
    }

    states: [
        State {
            name: "Back"
            SetProperty { target: Container; property: "rotation"; value: 180 }
        }
    ]

    transitions: [
        Transition {
            NumericAnimation { easing: "easeInOutQuad"; properties: "rotation"; duration: 500 }
        }
    ]
}
