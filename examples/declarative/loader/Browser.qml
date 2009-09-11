import Qt 4.6

Rectangle {
    id: Root
    width: parent.width
    height: parent.height
    color: Palette.base
    FolderListModel {
        id: folders
        nameFilters: [ "*.qml" ]
//        folder: "E:"
    }

    SystemPalette { id: Palette; colorGroup: Qt.Active }

    Component {
        id: FolderDelegate
        Rectangle {
            id: Wrapper
            function launch() {
                if (folders.isFolder(index)) {
                    folders.folder = filePath;
                } else {
                    qmlLauncher.launch(filePath);
                }
            }
            width: Root.width
            height: 32
            color: "transparent"
            Rectangle {
                id: Highlight; visible: false
                anchors.fill: parent
                gradient: Gradient {
                    GradientStop { id: t1; position: 0.0; color: Palette.highlight }
                    GradientStop { id: t2; position: 1.0; color: Palette.lighter(Palette.highlight) }
                }
            }
            Item {
                width: 32; height: 32
                Image { source: "images/fileopen.png"; anchors.centerIn: parent; visible: folders.isFolder(index)}
            }
            Text {
                id: NameText
                anchors.fill: parent; verticalAlignment: "AlignVCenter"
                text: fileName; anchors.leftMargin: 32
                font.pointSize: 10
                color: Palette.windowText
            }
            MouseRegion {
                id: Mouse
                anchors.fill: parent
                onClicked: { launch() }
            }
            states: [
                State {
                    name: "pressed"
                    when: Mouse.pressed
                    PropertyChanges { target: Highlight; visible: true }
                    PropertyChanges { target: NameText; color: Palette.highlightedText }
                }
            ]
        }
    }

    Script {
        function up(path) {
            var pos = path.toString().lastIndexOf("/");
            return path.toString().substring(0, pos);
        }
    }

    ListView {
        id: View
        anchors.top: TitleBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        model: folders
        delegate: FolderDelegate
        highlight: Rectangle { color: "#FFFBAF" }
        clip: true
        focus: true
        Keys.onPressed: {
            if (event.key == Qt.Key_Return || event.key == Qt.Key_Select) {
                View.currentItem.launch();
                event.accepted = true;
            }
        }
    }

    Rectangle {
        id: TitleBar
        width: parent.width
        height: 32
        color: Palette.button; border.color: Palette.mid

        Rectangle {
            id: UpButton
            width: 30
            height: TitleBar.height
            border.color: Palette.mid; color: "transparent"
            MouseRegion { anchors.fill: parent; onClicked: folders.folder = up(folders.folder) }
            Image { anchors.centerIn: parent; source: "images/up.png" }
        }

        Text {
            anchors.left: UpButton.right; anchors.right: parent.right; height: parent.height
            anchors.leftMargin: 4; anchors.rightMargin: 4
            text: folders.folder; color: Palette.buttonText
            elide: "ElideLeft"; horizontalAlignment: "AlignRight"; verticalAlignment: "AlignVCenter"
        }
    }
}
