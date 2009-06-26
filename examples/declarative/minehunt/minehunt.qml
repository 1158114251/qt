Item {
    id: field
    width: 370
    height: 480

    property int clickx : 0
    property int clicky : 0

    resources: [
        Component {
            id: tile
            Flipable {
                id: flipable
                width: 40
                height: 40
                axis: Axis {
                    startX: 20
                    startY: 20
                    endX: 20
                    endY: 0
                }
                front: Image {
                    source: "pics/front.png"
                    width: 40
                    height: 40
                    Image {
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                        source: "pics/flag.png"
                        opacity: modelData.hasFlag
                        opacity: Behavior {
                            NumberAnimation {
                                property: "opacity"
                                duration: 250
                            }
                        }
                    }
                }
                back: Image {
                    source: "pics/back.png"
                    width: 40
                    height: 40
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData.hint
                        color: "white"
                        font.bold: true
                        opacity: modelData.hasMine == false && modelData.hint > 0
                    }
                    Image {
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                        source: "pics/bomb.png"
                        opacity: modelData.hasMine
                    }
                    Explosion {
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                        explode: modelData.hasMine==true && modelData.flipped==true
                    }
                }
                states: [
                    State {
                        name: "back"
                        when: modelData.flipped == true
                        SetProperty {
                            target: flipable
                            property: "rotation"
                            value: 180
                        }
                    }
                ]
                transitions: [
                    Transition {
                        SequentialAnimation {
                            PauseAnimation {
                                duration: {var ret = Math.abs(flipable.parent.x-field.clickx) + Math.abs(flipable.parent.y-field.clicky); if (ret > 0) {if(modelData.hasMine==true && modelData.flipped==true){ret*3;}else{ret;}} else {0}}
                            }
                            NumberAnimation {
                                easing: "easeInOutQuad"
                                properties: "rotation"
                            }
                        }
                    }
                ]
                MouseRegion {
                    anchors.fill: parent
                    onClicked: { field.clickx = flipable.parent.x; field.clicky = flipable.parent.y; row = Math.floor(index/9); col = index - (Math.floor(index/9) * 9); if(mouse.button==undefined || mouse.button==Qt.RightButton){flag(row,col);}else{flip(row,col);} }
                }
            }
        }
    ]
    Image {
        source: "pics/No-Ones-Laughing-3.jpg"
        tile: true
    }
    Description {
        text: "Use the 'minehunt' executable to run this demo!"
        width: 300
        opacity: tiles?0:1
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
    }
    Repeater {
        dataSource: tiles
        x: 1
        y: 1
        Component {
            ComponentInstance {
                component: tile
                x: (index - (Math.floor(index/9) * 9)) * 41
                y: Math.floor(index/9) * 41
            }
        }
    }
    Item {
        id: gamedata
        width: 370
        height: 100
        y: 380
        Text {
            color: "white"
            font.size: 18
            x: 20
            y: 20
        }
        Image {
            x: 100
            y: 20
            source: "pics/bomb-color.png"
        }
        Text {
            x: 100
            y: 60
            color: "white"
            text: numMines
        }
        Image {
            x: 140
            y: 20
            source: "pics/flag-color.png"
        }
        Text {
            x: 140
            y: 60
            color: "white"
            text: numFlags
        }
        Image {
            x: 280
            y: 10
            source: if(isPlaying==true){'pics/face-smile.png'}else{if(hasWon==true){'pics/face-smile-big.png'}else{'pics/face-sad.png'}}
            MouseRegion {
                anchors.fill: parent
                onClicked: { reset() }
            }
        }
    }
}
