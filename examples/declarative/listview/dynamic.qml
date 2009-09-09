import Qt 4.6

Item {
    width: 300
    height: 500

    ListModel {
        id: FruitModel
        ListElement {
            name: "Apple"
            cost: 2.45
            attributes: [
                ListElement { description: "Core" },
                ListElement { description: "Deciduous" }
            ]
        }
        ListElement {
            name: "Banana"
            cost: 1.95
            attributes: [
                ListElement { description: "Tropical" },
                ListElement { description: "Seedless" }
            ]
        }
        ListElement {
            name: "Cumquat"
            cost: 3.25
            types: [ "Small", "Smaller" ]
            attributes: [
                ListElement { description: "Citrus" }
            ]
        }
        ListElement {
            name: "Durian"
            cost: 9.95
            attributes: [
                ListElement { description: "Tropical" },
                ListElement { description: "Smelly" }
            ]
        }
        ListElement {
            name: "Elderberry"
            cost: 0.05
            attributes: [
                ListElement { description: "Berry" }
            ]
        }
        ListElement {
            name: "Fig"
            cost: 0.25
            attributes: [
                ListElement { description: "Flower" }
            ]
        }
    }

    Component {
        id: FruitDelegate
        Item {
            width: parent.width; height: 55
            Text { id: Label; font.pixelSize: 24; text: name }
            Text { font.pixelSize: 24; text: '$'+Number(cost).toFixed(2); anchors.right: ItemButtons.left }
            Row { 
                anchors.top: Label.bottom
                spacing: 5
                Repeater {
                    model: attributes
                    Component {
                        Text { text: description }
                    }
                }
            }
            Row {
                id: ItemButtons
                anchors.right: parent.right
                width: childrenRect.width
                Image { source: "content/pics/add.png"
                    MouseRegion { anchors.fill: parent; onClicked: FruitModel.set(index,"cost",Number(cost)+0.25) }
                }
                Image { source: "content/pics/del.png"
                    MouseRegion { anchors.fill: parent; onClicked: FruitModel.set(index,"cost",Number(cost)-0.25) }
                }
                Image { source: "content/pics/trash.png"
                    MouseRegion { anchors.fill: parent; onClicked: FruitModel.remove(index) }
                }
                Column {
                    width: childrenRect.width
                    Image { source: "content/pics/moreUp.png"
                        MouseRegion { anchors.fill: parent; onClicked: FruitModel.move(index,index-1,1) }
                    }
                    Image { source: "content/pics/moreDown.png"
                        MouseRegion { anchors.fill: parent; onClicked: FruitModel.move(index,index+1,1) }
                    }
                }
            }
        }
    }

    ListView {
        model: FruitModel
        delegate: FruitDelegate
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: Buttons.top
    }

    Row {
        width: childrenRect.width
        height: childrenRect.height
        anchors.bottom: parent.bottom
        id: Buttons
        Image { source: "content/pics/add.png"
            MouseRegion { anchors.fill: parent;
                onClicked: {
                    FruitModel.append({
                        "name":"Pizza",
                        "cost":5.95,
                        "attributes":[{"description": "Supreme"},{"description": "Margarita"}]
                    })
                }
            }
        }
        Image { source: "content/pics/add.png"
            MouseRegion { anchors.fill: parent; onClicked: FruitModel.insert(0,{"name":"Pizza", "cost":5.95}) }
        }
        Image { source: "content/pics/trash.png"
            MouseRegion { anchors.fill: parent; onClicked: FruitModel.clear() }
        }
    }
}
