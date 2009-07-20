Rect {
    id: ImagePanel
    width: 1024
    height: 768
    color: "black"

    property int maximumColumn: 4
    property int selectedItemRow: 0
    property int selectedItemColumn: 0

    Script { source: "create.js" }

    onSelectedItemColumnChanged: if (selectedItemColumn == maximumColumn) createNewBlock();

    property int imageWidth: 200
    property int imageHeight: 200

    property int selectedX: selectedItemColumn * imageWidth
    property int selectedY: selectedItemRow * imageHeight

    Item {
        anchors.centeredIn: parent
        HorizontalLayout {
            id: MyLayout
            property real targetX: -(selectedX + imageWidth / 2)

            property real targetDeform: 0
            property bool slowDeform: true

            property real deform
            deform: Follow { source: MyLayout.targetDeform; velocity: MyLayout.slowDeform?0.1:2 }
                
            onDeformChanged: if(deform == targetDeform) { slowDeform = true; targetDeform = 0; }

            ImageBatch { offset: 0; ref: ImagePanel }

            x: Follow { source: MyLayout.targetX; velocity: 1000 }
            y: Follow { source: -(selectedY + imageHeight / 2); velocity: 500 }
        }

        transform: Rotation3D { 
            axis { startX: 0; startY: 0; endX: 0; endY: 1 }
            angle: MyLayout.deform * 100
        }
    }

    Script {
        function leftPressed() {
            if (selectedItemColumn <= 0) return;
            selectedItemColumn -= 1;
            MyLayout.slowDeform = false;
            MyLayout.targetDeform = Math.max(Math.min(MyLayout.deform - 0.1, -0.1), -0.4);
        }
        function rightPressed() {
            selectedItemColumn += 1;
            MyLayout.slowDeform = false;
            MyLayout.targetDeform = Math.min(Math.max(MyLayout.deform + 0.1, 0.1), 0.4);
        }
    }
    KeyActions {
        focus: true
        leftArrow: "leftPressed()"
        rightArrow: "rightPressed()"
        upArrow: "if (selectedItemRow > 0) selectedItemRow = selectedItemRow - 1"
        downArrow: "if (selectedItemRow < 3) selectedItemRow = selectedItemRow + 1"
    }
}
