import QtQuick 2.0
import QtQuick.Controls 2.3
import QtGraphicalEffects 1.0

import "qrc:/QML/qml/."

Rectangle {
    id: rect

    antialiasing: false
    color: theme.backColor

    property int posX: (width / hbar.size * hbar.position)

    property int rowHeight: 17

    property int selectInd: -1
    property int selectRow: -1

    property int minStep: 8
    property int maxStep: 40

    property int maxPos: 0

    property real timeScale: 0.01
    property int timeStep: minStep

    signal addKey   (int row, int col, int position)
    signal removeKey(int row, int col, int index)
    signal selectKey(int row, int col, int index)

    Theme {
        id: theme
    }

    Connections {
        target: clipModel
        onLayoutChanged: {
            maxPos = 0
            for(var i = 0; i < clipModel.rowCount(); i++) {
                maxPos = clipModel.maxPosition(i)
            }
            curve.row = 0
        }
    }

    MouseArea {
        anchors.fill: parent

        acceptedButtons: Qt.LeftButton | Qt.MidButton
        hoverEnabled: true

        property int oldX: 0
        property int oldY: 0

        onClicked: {
            selectInd = -1
            selectRow = -1
            clipModel.position = Math.max(Math.round((mouseX + posX) / timeStep), 0) * timeScale
        }

        onPressed: {
            if(mouse.buttons === Qt.MiddleButton) {
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
                hbar.position += (mouseX / curve.toScreenSpaceX(maxPos)) * (1 / timeStep)

                curve.valueStep += 1
                if(curve.valueStep > curve.maxValue) {
                    curve.valueStep = curve.minValue
                    curve.valueScale /= 5.0;
                }
                vbar.position += (mouseY / curve.toScreenSpaceY(Math.abs(curve.maximum - curve.minimum))) * (1 / curve.valueStep)

            } else {
                timeStep -= 1
                if(timeStep < minStep) {
                    timeStep = maxStep
                    timeScale *= 5.0
                }
                hbar.position -= (mouseX / curve.toScreenSpaceX(maxPos)) * (1 / timeStep)

                curve.valueStep -= 1
                if(curve.valueStep < curve.minValue) {
                    curve.valueStep = curve.maxValue
                    curve.valueScale *= 5.0;
                }
                vbar.position -= (mouseY / curve.toScreenSpaceY(Math.abs(curve.maximum - curve.minimum))) * (1 / curve.valueStep)

            }
            hbar.position = Math.max(hbar.position, 0.0)
        }

        onPositionChanged: {
            if(selectInd == -1 && pressedButtons === Qt.LeftButton) {
                clipModel.position = Math.max(Math.round((mouseX - posX) / timeStep), 0) * timeScale
            }

            if(mouse.buttons === Qt.MiddleButton) {
                hbar.position += curve.toLocalSpaceX(oldX - mouse.x) * 0.5
                hbar.position = Math.max(hbar.position, 0.0)
                vbar.position += curve.toLocalSpaceY(oldY - mouse.y) / curve.valueScale
                oldX = mouse.x
                oldY = mouse.y
            }

        }
    }

    Item {
        id: ruler
        x: 0
        width: parent.width
        height: parent.height

        Repeater {
            model: Math.ceil(ruler.width / (timeStep * 5))
            Rectangle {
                anchors.bottom: ruler.bottom
                height: ruler.height - 12
                width: 1
                color: theme.textColor
                x: (index * 5) * timeStep - (posX % (timeStep * 5)) + minStep
                Label {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.top
                    color: theme.textColor
                    text: {
                        var value = ((index + Math.floor(posX / (timeStep * 5))) * 5) * timeScale
                        return value.toLocaleString(Qt.locale("en_EN"), 'f', 2).replace('.', ':')
                    }
                    font.pointSize: 8

                    renderType: Text.NativeRendering
                }
            }
        }

        Repeater {
            model: ruler.width / timeStep
            Rectangle {
                anchors.bottom: ruler.bottom
                height: ruler.height - 15
                width: 1
                x: index * timeStep - (posX % timeStep) + minStep
                color: theme.textColor
            }
        }
    }

    CurveEditor {
        id: curve
        objectName: "curve"

        anchors.fill: parent
        anchors.topMargin: 19
        visible: false

        posX: (width / hbar.size * hbar.position)
        posY: (toScreenSpaceY(maximum + Math.abs(maximum - minimum) * 0.5)) - (parent.height / vbar.size * vbar.position)

        timeStep: parent.timeStep
        timeScale: parent.timeScale

        valueStep: minValue
        valueScale: 1.0

        onRowChanged: {
            if(visible) {
                vbar.position = 0.5 - vbar.size * 0.5
            }
        }

        onVisibleChanged: {
            if(visible) {
                vbar.size = (parent.height - 19) / (toScreenSpaceY(Math.abs(maximum - minimum)))
                vbar.position = 0.5 - vbar.size * 0.5
            }
        }
    }

    KeyframeEditor {
        id: keys
        objectName: "keys"

        anchors.fill: parent
        anchors.topMargin: 19

        posX: (width / hbar.size * hbar.position)
        //posY: (height / vbar.size * vbar.position)

        timeStep: parent.timeStep
        timeScale: parent.timeScale

        //visible: false
    }

    Rectangle {
        id: position
        x: (clipModel.position / timeScale) * timeStep - posX + minStep - 1
        width: 2
        height: parent.height
        color: theme.red
    }

    ScrollBar {
        id: hbar
        hoverEnabled: true
        active: hovered || pressed
        orientation: Qt.Horizontal
        size: parent.width / (curve.toScreenSpaceX(maxPos) + maxStep * 2)

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }

    ScrollBar {
        id: vbar
        hoverEnabled: true
        active: hovered || pressed
        orientation: Qt.Vertical
        size: (parent.height - 19) / (curve.toScreenSpaceY(Math.abs(curve.maximum - curve.minimum)))

        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }
}
