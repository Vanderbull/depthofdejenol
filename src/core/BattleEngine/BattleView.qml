import QtQuick
import QtQuick.Controls

Window {
    id: battleRoot
    width: 960
    height: 600
    visible: true          // <--- This makes the native desktop window appear!
    title: "Heroes Battle Prototype"

    Rectangle {
        anchors.fill: parent
        color: "#1e1e1e"

        // Property to track selected unit index
        property int selectedUnitIndex: 0
        property int tileSize: 60

        // Grid representation
        Item {
            id: gridContainer
            width: battleEngine.gridWidth * tileSize
            height: battleEngine.gridHeight * tileSize
            anchors.centerIn: parent

            Grid {
                id: mapGrid
                columns: battleEngine.gridWidth
                rows: battleEngine.gridHeight

                Repeater {
                    model: battleEngine.gridWidth * battleEngine.gridHeight
                    delegate: Rectangle {
                        required property int index
                        width: tileSize; height: tileSize
                        color: (index % 2 === 0) ? "#2b2b2b" : "#333333"
                        border.color: "#444"
                        border.width: 1

                        property int posX: index % battleEngine.gridWidth
                        property int posY: Math.floor(index / battleEngine.gridWidth)

                        Text {
                            anchors.centerIn: parent
                            text: {
                                var unit = battleEngine.getUnitAt(posX, posY);
                                return unit.name ? unit.name[0] + "\n[" + unit.count + "]" : "";
                            }
                            color: {
                                var unit = battleEngine.getUnitAt(posX, posY);
                                return unit.isPlayer ? "#55ff55" : "#ff5555";
                            }
                            horizontalAlignment: Text.AlignHCenter
                            font.pixelSize: 12
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                battleEngine.moveUnit(battleRoot.selectedUnitIndex, posX, posY);
                            }
                        }
                    }
                }
            }
        }

        // Classic HoMM Style Control Panel at Bottom
        Rectangle {
            id: controlPanel
            height: 100
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            color: "#2e2b25"
            border.color: "#5c5341"
            border.width: 2

            Row {
                anchors.centerIn: parent
                spacing: 20

                Button {
                    text: "Wait"
                    onClicked: console.log("Unit delayed turn")
                }
                Button {
                    text: "Defend"
                    onClicked: console.log("Unit defending")
                }
                Button {
                    text: "Surrender"
                    onClicked: Qt.quit()
                }
            }
        }
    }
}
