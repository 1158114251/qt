import Qt 4.6

Rectangle {
    color: "white"
    width: 800
    height: 600

    Keys.onDigit9Pressed: console.log("Error - Root")

    FocusScope {
        id: MyScope
        focus: true

        Keys.onDigit9Pressed: console.log("Error - FocusScope")
        
        Rectangle {
            height: 120
            width: 420

            color: "transparent"
            border.width: 5
            border.color: MyScope.wantsFocus?"blue":"black"

            Rectangle {
                id: Item1
                x: 10; y: 10 
                width: 100; height: 100; color: "green"
                border.width: 5
                border.color: wantsFocus?"blue":"black"
                Keys.onDigit9Pressed: console.log("Top Left");
                KeyNavigation.right: Item2
                focus: true 

                Rectangle {
                    width: 50; height: 50; anchors.centerIn: parent
                    color: parent.focus?"red":"transparent"
                }
            }

            Rectangle {
                id: Item2
                x: 310; y: 10
                width: 100; height: 100; color: "green"
                border.width: 5
                border.color: wantsFocus?"blue":"black"
                KeyNavigation.left: Item1
                Keys.onDigit9Pressed: console.log("Top Right");

                Rectangle {
                    width: 50; height: 50; anchors.centerIn: parent
                    color: parent.focus?"red":"transparent"
                }
            }
        }
        KeyNavigation.down: Item3
    }

    Text { x:100; y:170; text: "Blue border indicates scoped focus\nBlack border indicates NOT scoped focus\nRed box indicates active focus\nUse arrow keys to navigate\nPress \"9\" to print currently focused item" }

    Rectangle {
        id: Item3
        x: 10; y: 300
        width: 100; height: 100; color: "green"
        border.width: 5
        border.color: wantsFocus?"blue":"black"

        Keys.onDigit9Pressed: console.log("Bottom Left");
        KeyNavigation.up: MyScope

        Rectangle {
            width: 50; height: 50; anchors.centerIn: parent
            color: parent.focus?"red":"transparent"
        }
    }

}
