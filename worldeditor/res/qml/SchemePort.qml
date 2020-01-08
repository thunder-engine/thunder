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
        var result = (createMode && ((nodeObject.x + ((port > -1) ? x : 0) - canvas.translateX) < canvas.mouseX && (nodeObject.x + ((port > -1) ? x : 0) + width - canvas.translateX) > canvas.mouseX &&
                                     (nodeObject.y + ((port > -1) ? y : 0) - canvas.translateY) < canvas.mouseY && (nodeObject.y + ((port > -1) ? y : 0) + height - canvas.translateY) > canvas.mouseY) )
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
            canvas.mouseX = (nodeObject.x + ((port > -1) ? parent.x : 0)) + mouse.x - canvas.translateX
            canvas.mouseY = (nodeObject.y + ((port > -1) ? parent.y : 0)) + mouse.y - canvas.translateY
            canvas.requestPaint()
        }

        onClicked: {
            if(mouse.modifiers & Qt.AltModifier) {
                schemeModel.deleteLinksByPort(node, port)
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
                if(stateMachine || focusPort !== -1) {
                    if((portObject !== undefined && portObject.out) || stateMachine) {
                        schemeModel.createLink(selectNode, selectPort, focusNode, focusPort)
                    } else {
                        schemeModel.createLink(focusNode, focusPort, selectNode, selectPort)
                    }
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
