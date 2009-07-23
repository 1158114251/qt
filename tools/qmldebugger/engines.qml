import Qt 4.6

Item {
    height: 100
    id: Root
    signal engineClicked(int id)
    signal refreshEngines()

    HorizontalLayout {
        anchors.fill: parent
        Repeater {
            dataSource: engines
            Item {
                width: 100; height: 100; 
                Image { 
                    id: Image; 
                    source: "engine.png" 
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                Text { 
                    anchors.top: Image.bottom; 
                    text: modelData.name + "(" + modelData.engineId + ")" 
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                MouseRegion {
                    anchors.fill: parent
                    onClicked: Root.engineClicked(modelData.engineId);
                }
            }
        }
    }


    Image { 
        y: 15
        source: "refresh.png"; 
        width: 75; 
        height: 63; 
        smooth: true 
        anchors.right: parent.right
        MouseRegion {
            anchors.fill: parent
            onClicked: Root.refreshEngines()
        }
    }
}
