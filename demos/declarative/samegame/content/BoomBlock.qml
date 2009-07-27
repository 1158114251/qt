import Qt 4.6

Item { id:block
    property bool dying: false
    property bool spawned: false
    property bool selected: false
    property int type: 0
    property int targetX: 0
    property int targetY: 0

    x: Follow { enabled: spawned; source: targetX; spring: 2; damping: 0.2 }
    y: Follow { source: targetY; spring: 2; damping: 0.2 }

    //TODO: Replace with an image with a fuzzy effect like KSame
    Rect { id: shine; radius:16; anchors.fill:parent; color: "yellow";
        opacity: 0
        opacity: SequentialAnimation{running: selected&&!dying; finishPlaying: true; repeat: true
            NumberAnimation{ from: 0; to: 1; }NumberAnimation{ from:1; to:0; }}
    }
    MouseRegion {
        id: gameMR; anchors.fill: parent
        onClicked: handleClick(Math.floor(parent.x/width), Math.floor(parent.y/height));
        onEntered: handleHover(Math.floor(parent.x/width), Math.floor(parent.y/height));
        onExited: handleHover(-1,-1);
    }

    Image { id: img
        source: {
            if(type == 0){
                "pics/redStone.png";
            } else if(type == 1) {
                "pics/blueStone.png";
            } else {
                "pics/greenStone.png";
            }
        }
        opacity: 0
        opacity: Behavior { NumberAnimation { properties:"opacity"; duration: 200 } }
        anchors.fill: parent
    }

    Particles { id: particles
        width:1; height:1; anchors.centeredIn: parent; opacity: 0
        lifeSpan: 700; lifeSpanDeviation: 600; count:0; streamIn: false
        angle: 0; angleDeviation: 360; velocity: 100; velocityDeviation:30
        source: {
            if(type == 0){
                "pics/redStar.png";
            } else if (type == 1) {
                "pics/blueStar.png";
            } else {
                "pics/greenStar.png";
            }
        }
    }

    states: [

        State{ name: "AliveState"; when: spawned == true && dying == false
            SetProperties { target: img; opacity: 1 }
        },
        State{ name: "DeathState"; when: dying == true
            SetProperties { target: particles; count: 50 }
            SetProperties { target: particles; opacity: 1 }
            SetProperties { target: particles; emitting: false } // i.e. emit only once
            SetProperties { target: img; opacity: 0 }
        }
    ]
}
