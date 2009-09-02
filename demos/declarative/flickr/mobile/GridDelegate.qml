 import Qt 4.6

 Component {
     id: PhotoDelegate
     Item {
         id: Wrapper; width: 79; height: 79

         Script {
             function photoClicked() {
                 ImageDetails.photoTitle = title;
                 ImageDetails.photoTags = tags;
                 ImageDetails.photoWidth = photoWidth;
                 ImageDetails.photoHeight = photoHeight;
                 ImageDetails.photoType = photoType;
                 ImageDetails.photoAuthor = photoAuthor;
                 ImageDetails.photoDate = photoDate;
                 ImageDetails.photoUrl = url;
                 ImageDetails.rating = 0;
                 ScaleMe.state = "Details";
             }
         }

         Item {
             anchors.centerIn: parent
             scale: 0.0
             scale: Behavior { NumberAnimation { easing: "easeInOutQuad"} }
             id: ScaleMe

             Rectangle { height: 79; width: 79; id: BlackRect;  anchors.centerIn: parent; color: "black"; smooth: true }
             Rectangle {
                 id: WhiteRect; width: 77; height: 77; anchors.centerIn: parent; color: "#dddddd"; smooth: true
                 Image { id: Thumb; source: imagePath; x: 1; y: 1; smooth: true}
                 Image { source: "images/gloss.png" }
             }

             Connection {
                 sender: ToolBar; signal: "button2Clicked()"
                 script: if (ScaleMe.state == 'Details' ) ScaleMe.state = 'Show';
             }

             states: [
                 State {
                     name: "Show"; when: Thumb.status == 1
                     PropertyChanges { target: ScaleMe; scale: 1 }
                 },
                 State {
                     name: "Details"; extend: "Show"
                     //PropertyChanges { target: ImageDetails; z: 2 }
                     ParentChange { target: Wrapper; parent: ImageDetails.frontContainer }
                     PropertyChanges { target: Wrapper; x: 20; y: 60; z: 1000 }
                     PropertyChanges { target: Background; state: "DetailedView" }
                 }
             ]
             transitions: [
                 Transition {
                     from: "Show"; to: "Details"
                     ParentAction { }
                     NumberAnimation { properties: "x,y"; duration: 500; easing: "easeInOutQuad" }
                 },
                 Transition {
                     from: "Details"; to: "Show"
                     SequentialAnimation {
                         ParentAction { }
                         NumberAnimation { properties: "x,y"; duration: 500; easing: "easeInOutQuad" }
                         PropertyAction { target: Wrapper; properties: "z" }
                     }
                 }
             ]
         }
         MouseRegion { anchors.fill: Wrapper; onClicked: { photoClicked() } }
     }
 }
