import Qt 4.6

Rectangle {
    property var myinput: input

    width: 800; height: 600; color: "blue"

    Item { 
        id: firstItem;
        KeyNavigation.right: input
    }

    Textinput { id: input; focus: true
        KeyNavigation.left: firstItem
        KeyNavigation.right: lastItem
        KeyNavigation.up: firstItem
        KeyNavigation.down: lastItem
    }
    Item {
        id: lastItem 
        KeyNavigation.left: input
    }
}
