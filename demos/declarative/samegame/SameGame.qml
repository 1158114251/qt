import Qt 4.6
import "content"

Rectangle {
    id: Screen
    width: 490; height: 720

    Script { source: "content/samegame.js" }

    SystemPalette { id: activePalette; colorGroup: Qt.Active }

    Item {
        width: parent.width; anchors.top: parent.top; anchors.bottom: ToolBar.top

        Image {
            id: background
            anchors.fill: parent; source: "content/pics/background.png"
            fillMode: "PreserveAspectCrop"
        }

        Item {
            id: gameCanvas
            property int score: 0

            z: 20; anchors.centerIn: parent
            width: parent.width - (parent.width % tileSize);
            height: parent.height - (parent.height % tileSize);

            MouseRegion {
                id: gameMR
                anchors.fill: parent; onClicked: handleClick(mouse.x,mouse.y);
            }
        }
    }

    Dialog { id: dialog; anchors.centerIn: parent; z: 21 }
    Dialog { 
        id: scoreName; anchors.centerIn: parent; z: 22; 
        TextInput {
            id: Editor
            onAccepted: { 
                if(scoreName.opacity==1&&Editor.text!="")
                    sendHighScore(Editor.text);
                scoreName.forceClose(); 
            }
            anchors.verticalCenter: parent.verticalCenter
            x:160; width: 200; height:20; focus: true
        }
    }

    Rectangle {
        id: ToolBar
        color: activePalette.window
        height: 32; width: parent.width
        anchors.bottom: Screen.bottom

        Button {
            id: btnA; text: "New Game"; onClicked: {initBoard();}
            anchors.left: parent.left; anchors.leftMargin: 3
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            id: Score
            text: "Score: " + gameCanvas.score; font.bold: true
            anchors.right: parent.right; anchors.rightMargin: 3
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
