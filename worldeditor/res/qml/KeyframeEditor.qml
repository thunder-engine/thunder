import QtQuick 2.0
import QtQuick.Controls 2.3

Rectangle {
    id: keyframeEditor

    color: "#f0808080"
    clip: true

    focus: true

    property int selectInd: -1
    property int selectRow: -1
    property int selectCol: -1

    property int posX: 0
    property int posY: 0

    property int timeStep: minStep
    property real timeScale: 0.01

    function toScreenSpaceX(pos) {
        return ((pos / 1000.0) / timeScale) * timeStep + minStep
    }

    Keys.onPressed: {
        if(event.key === Qt.Key_Delete) {
            removeKey(selectRow, selectCol, selectInd)
            selectInd = -1
            selectRow = -1
            selectCol = -1
        }
    }

    Connections {
        target: clipModel
        onLayoutChanged: {
            points.model = 0 // to update repeater
            points.model = clipModel.rowCount()
        }
    }

    Repeater {
        id: points

        property int pointSize: 6
        property int pointCenter: pointSize * 0.5

        anchors.fill: parent

        Rectangle {
            id: keys

            height: rowHeight
            width: parent.width
            y: expanded * height - posY

            color: "#40000000"

            property int row: index
            property variant curve: clipModel.trackData(row)
            property int componentsNumber: curve.length

            property int expanded: clipModel.expandHeight(row)

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                onDoubleClicked: {
                    addKey(row, -1, Math.max(Math.round((mouseX + posX) / timeStep), 0) * timeScale * 1000)
                }
                onClicked: {
                    if(mouse.button === Qt.LeftButton) {
                        selectInd = -1
                        selectRow = -1
                        selectCol = -1
                    }

                    if(mouse.button === Qt.RightButton) {
                        menu.x = mouseX
                        menu.y = mouseY
                        menu.open()
                    }
                }

                Menu {
                    id: menu
                    y: parent.height

                    MenuItem {
                        text: qsTr("Add Key")
                        onTriggered: addKey(row, Math.max(Math.round((menu.x + posX) / timeStep), 0) * timeScale * 1000)
                    }
                    MenuItem {
                        text: qsTr("Delete Key")
                        visible: (selectRow >= 0 && selectInd >= 0)
                        onTriggered: {
                            var k = selectInd
                            var r = selectRow
                            var c = selectCol

                            selectInd = -1
                            selectRow = -1
                            selectCol = -1

                            removeKey(r, c, k)
                        }
                    }
                }
            }

            Rectangle {
                 color: "#808080"
                 height: 1
                 anchors.left: parent.left
                 anchors.right: parent.right
                 anchors.top: parent.top
            }

            Repeater {
                model: keys.componentsNumber

                Repeater {
                    id: comp
                    model: keys.curve[col].length - 1

                    property int col: index
                    property int component: keys.curve[col][0]

                    Item {
                        Keyframe {
                            x: toScreenSpaceX(key[0]) - posX - points.pointCenter
                        }

                        Keyframe {
                            x: toScreenSpaceX(key[0]) - posX - points.pointCenter
                            y: (comp.col + 1) * rowHeight + 2
                            visible: clipModel.isExpanded(row)
                        }
                    }
                }
            }
        }
    }
}
