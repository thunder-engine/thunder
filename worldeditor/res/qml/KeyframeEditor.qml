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
            y: index * height - posY
            color: "#40000000"

            property int row: index
            property variant curve: clipModel.trackData(row)
            property int componentsNumber: curve.length

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

                    Rectangle {
                        id: item
                        color: (selectInd == index && selectRow == row && selectCol == col) ? theme.blueHover : "#a0606060"
                        border.color: theme.textColor

                        height: 7
                        width: 7

                        x: toScreenSpaceX(key[0]) - posX - points.pointCenter
                        y: 2
                        rotation: 45

                        property variant key: keys.curve[comp.col][index + 1]

                        function commitKey() {
                            var data = keys.curve
                            data[comp.col][index + 1] = item.key
                            keys.curve = data
                        }

                        MouseArea {
                            anchors.fill: parent

                            drag.target: parent
                            drag.axis: Drag.XAxis
                            drag.minimumX: 0
                            drag.maximumX: keyframeEditor.width
                            drag.threshold: 0

                            drag.onActiveChanged: {
                                if(!drag.active) {
                                    clipModel.setTrackData(keys.row, keys.curve)
                                }
                            }

                            onPressed: {
                                selectInd = index
                                selectRow = row
                                selectCol = col

                                selectKey(row, col, index)
                            }

                            onPositionChanged: {
                                if(drag.active) {
                                    var x = Math.round(item.x / timeStep) * timeStep - points.pointSize

                                    item.key[0] = Math.max(Math.round((x + posX) / timeStep), 0) * timeScale * 1000
                                    item.commitKey()
                                }
                            }
                        }
                    }
                }
            }

        }
    }
}
