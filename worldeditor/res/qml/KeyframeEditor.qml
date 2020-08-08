import QtQuick 2.0
import QtQuick.Controls 2.3

Rectangle {
    id: keyframeEditor

    color: "#80808080"
    clip: true

    focus: true

    property int selectInd: -1
    property int selectRow: -1
    property int selectCol: -1

    property int posX: 0
    property int posY: 0

    property int timeStep: minStep
    property real timeScale: 0.01

    property variant keys: undefined

    function toScreenSpaceX(pos) {
        return ((pos / 1000.0) / timeScale) * timeStep + minStep
    }

    function deleteKey() {
        removeKey(selectRow, selectCol, selectInd)
        selectInd = -1
        selectRow = -1
        selectCol = -1
    }

    Connections {
        target: clipModel
        onLayoutChanged: {
            keys = {}
            var rows = clipModel.rowCount()
            for(var r = 0; r < rows; r++) {
                var track = clipModel.trackData(r)
                for(var c = 0; c < track.length; c++) {
                    var curve = track[c]
                    for(var k = 1; k < curve.length; k++) {
                        var position = curve[k][0]

                        if(keys[position] === undefined) {
                            keys[position] = []
                        }
                        keys[position].push(c)
                    }
                }
            }

            points.model = 0 // to update repeater
            points.model = rows
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
                    insertKey(row, -1, Math.max(Math.round((mouseX + posX - minStep) / timeStep), 0) * timeScale * 1000)
                }
                onClicked: {
                    if(mouse.button === Qt.LeftButton) {
                        selectInd = -1
                        selectRow = -1
                        selectCol = -1
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
                id: components

                model: keys.componentsNumber

                property bool expanded: clipModel.isExpanded(row)

                Repeater {
                    id: component
                    model: keys.curve[col].length - 1

                    property int col: index
                    property int component: keys.curve[col][0]

                    Item {
                        Keyframe {
                            x: toScreenSpaceX(key[0]) - posX - points.pointCenter
                            key: keys.curve[component.col][index + 1]
                        }

                        Keyframe {
                            x: toScreenSpaceX(key[0]) - posX - points.pointCenter
                            y: (component.col + 1) * rowHeight + 2
                            visible: components.expanded
                            key: keys.curve[component.col][index + 1]
                        }
                    }
                }
            }

        }
    }
}
