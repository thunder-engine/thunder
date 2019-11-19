import QtQuick 2.0

Rectangle {
    width: radius * 2
    height: radius * 2
    radius: 4
    color: theme.blue

    property int node: -1
    property int port: -1

    property variant portObject: undefined

    property bool isFocus: {
        var result = (createMode && ((nodeObject.x + x - canvas.translateX) < canvas.mouseX && (nodeObject.x + x + width - canvas.translateX) > canvas.mouseX &&
                                     (nodeObject.y + y - canvas.translateY) < canvas.mouseY && (nodeObject.y + y + height - canvas.translateY) > canvas.mouseY) )
        if(result === true) {
            color = theme.blueHover

            focusNode = node
            focusPort = port
        } else {
            color = theme.blue
        }

        return result
    }

    MouseArea {
        id: area
        anchors.fill: parent
        hoverEnabled: true

        onPositionChanged: {
            canvas.mouseX = (nodeObject.x + parent.x) + mouse.x - canvas.translateX
            canvas.mouseY = (nodeObject.y + parent.y) + mouse.y - canvas.translateY
            canvas.requestPaint()
        }

        onClicked: {
            if(mouse.modifiers & Qt.AltModifier) {
                schemeModel.deleteLinksByNode(node, port)
            }
        }
        onPressed: {
            nodeSelect(node)
            selectPort = port
            createMode = true
            canvas.requestPaint()
        }
        onReleased: {
            if(createMode && selectNode != focusNode) {
                if(portObject !== undefined && portObject.out) {
                    schemeModel.createLink(selectNode, selectPort, focusNode, focusPort)
                } else {
                    schemeModel.createLink(focusNode, focusPort, selectNode, selectPort)
                }

                selectNode = -1
                selectPort = -1
            }
            createMode = false
            canvas.requestPaint()
        }

        onEntered: {
            parent.color = theme.blueHover
            focusNode = node
            focusPort = port
            canvas.requestPaint()
        }
        onExited: {
            parent.color = theme.blue
            focusNode = -1
            focusPort = -1
            canvas.requestPaint()
        }
    }
}
