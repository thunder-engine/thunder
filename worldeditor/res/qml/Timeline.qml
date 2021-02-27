import QtQuick 2.0
import QtQuick.Controls 2.3
import QtGraphicalEffects 1.0

import "qrc:/QML/qml/."

Rectangle {
    id: rect

    antialiasing: false
    color: theme.backColor

    property int posX: width / hbar.size * hbar.position
    property int posY: height / vbar.size * vbar.position

    property int rowHeight: 17

    property int selectInd: -1
    property int selectRow: -1

    property int minStep: 8
    property int maxStep: 40

    property int maxDuration: 0

    property real timeScale: 0.01
    property int timeStep: minStep

    signal insertKey(int row, int col, int position)
    signal removeKey(int row, int col, int index)
    signal selectKey(int row, int col, int index)

    Theme {
        id: theme
    }

    Connections {
        target: clipModel
        onLayoutChanged: {
            maxDuration = 0
            for(var i = 0; i < clipModel.rowCount(); i++) {
                maxDuration = Math.max(clipModel.duration(i), maxDuration)
            }
            curve.row = 0
        }
    }

    Keys.onPressed: {
        if(event.key === Qt.Key_Delete) {
            if(curve.visible) {
                curve.deleteKey()
            } else {
                keys.deleteKey()
            }
        }
    }

    MouseArea {
        anchors.fill: parent

        acceptedButtons: Qt.RightButton
        hoverEnabled: true

        property int oldX: 0
        property int oldY: 0

        onPressed: {
            if(mouse.buttons === Qt.RightButton) {
                oldX = mouse.x
                oldY = mouse.y
            }
        }

        onWheel: {
            if(wheel.angleDelta.y > 0) {
                timeStep += 1
                if(timeStep > maxStep) {
                    if(timeScale > 0.01) {
                        timeStep = minStep
                        timeScale /= 5
                    }
                }

                curve.valueStep += 1
                if(curve.valueStep > curve.maxValue) {
                    curve.valueStep = curve.minValue
                    curve.valueScale /= 5.0
                }
            } else {
                timeStep -= 1
                if(timeStep < minStep) {
                    timeStep = maxStep
                    timeScale *= 5.0
                }

                curve.valueStep -= 1
                if(curve.valueStep < curve.minValue) {
                    curve.valueStep = curve.maxValue
                    curve.valueScale *= 5.0;
                }
            }
        }

        onPositionChanged: {
            if(mouse.buttons === Qt.RightButton) {
                var deltaX = (oldX - mouse.x) * hbar.size
                hbar.position += deltaX / width
                hbar.position = Math.max(hbar.position, 0.0)

                var deltaY = (oldY - mouse.y) * vbar.size
                vbar.position -= deltaY / height

                oldX = mouse.x
                oldY = mouse.y
            }
        }
    }

    GridLines {
        id: grid
        anchors.fill: parent
        anchors.topMargin: 19

        translateX: -(parent.posX % (timeStep * 5)) + minStep
        translateY: curve.valueStep + (curve.posY % curve.valueStep)

        drawHorizontal: curve.visible
        subItemsX: 5
        subItemsY: 2
        cellX: timeStep
        cellY: curve.valueStep
    }

    Item {
        id: ruler
        x: 0
        width: parent.width
        height: parent.height

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton
            onPressed: {
                if(mouse.buttons === Qt.LeftButton) {
                    clipModel.position = Math.max(Math.round((mouseX + posX - minStep) / timeStep), 0) * timeScale
                }
            }

            onPositionChanged: {
                if(pressedButtons === Qt.LeftButton) {
                    clipModel.position = Math.max(Math.round((mouseX + posX - minStep) / timeStep), 0) * timeScale
                }
            }
        }

        Repeater {
            model: Math.ceil(ruler.width / (timeStep * 5))
            Item {
                anchors.bottom: ruler.bottom
                height: ruler.height - 12
                x: (index * 5) * timeStep - (posX % (timeStep * 5)) + minStep
                Label {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.top
                    color: theme.textColor
                    text: {
                        var value = ((index + Math.floor(posX / (timeStep * 5))) * 5) * timeScale
                        return value.toLocaleString(Qt.locale("en_EN"), 'f', 2)
                    }
                    font.pointSize: 8
                    renderType: Text.NativeRendering
                }
            }
        }
    }

    CurveEditor {
        id: curve
        objectName: "curve"

        anchors.fill: parent
        anchors.topMargin: 19
        visible: false

        posX: parent.posX
        posY: parent.posY

        timeStep: parent.timeStep
        timeScale: parent.timeScale

        valueStep: minValue
        valueScale: 1.0

        onRowChanged: {
            if(visible) {
                vbar.position = vbar.size * 0.5
            }
        }

        onVisibleChanged: {
            if(visible) {
                vbar.position = vbar.size * 0.5
            }
        }
    }

    KeyframeEditor {
        id: keys
        objectName: "keys"

        anchors.fill: parent
        anchors.topMargin: 19

        posX: parent.posX

        timeStep: parent.timeStep
        timeScale: parent.timeScale
    }

    Rectangle {
        id: position
        x: (clipModel.position / timeScale) * timeStep - parent.posX + minStep
        width: 2
        height: parent.height
        color: theme.red
    }

    ResizableScrollBar {
        id: hbar
        orientation: Qt.Horizontal
        size: parent.width / (curve.toScreenSpaceX(maxDuration) + maxStep)

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        onHeadPositionChanged: {
            hbar.position -= value - hbar.size
            hbar.size = value
        }

        onTailPositionChanged: {
            size = value
        }
    }

    ResizableScrollBar {
        id: vbar
        orientation: Qt.Vertical
        size: {
            var value = curve.toScreenSpaceY(Math.abs(curve.maximum - curve.minimum))
            return height / ((value === 0) ? 1 : value)
        }

        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: parent.height - 19

        onHeadPositionChanged: {
            vbar.position -= value - vbar.size
            vbar.size = value
        }

        onTailPositionChanged: {
            vbar.size = value
        }
    }
}
