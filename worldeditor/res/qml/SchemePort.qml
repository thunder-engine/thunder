import QtQuick 2.0

Rectangle {
    color: (port !== -1) ? "transparent" : theme.blue

    width: radius * 2
    height: radius * 2
    radius: 4

    property int node: -1
    property int port: -1

    property variant portObject: undefined

    property string label: ""

    property bool isFocus: {
        var result = (createMode &&
                      ((nodeObject.x + ((port > -1) ? x : 0) + scheme.x) < canvas.mouseX &&
                       (nodeObject.x + ((port > -1) ? x : 0) + scheme.x + width) > canvas.mouseX &&
                       (nodeObject.y + ((port > -1) ? y : 0) + scheme.y) < canvas.mouseY &&
                       (nodeObject.y + ((port > -1) ? y : 0) + scheme.y + height) > canvas.mouseY) )
        if(result === true) {
            bullet.color = theme.blueHover

            focusNode = node
            focusPort = port
        } else {
            bullet.color = theme.blue
        }

        return result
    }

    MouseArea {
        id: area
        anchors.fill: parent
        hoverEnabled: true

        onPositionChanged: {
            canvas.mouseX = (nodeObject.x + ((port > -1) ? parent.x : 0)) + mouse.x + scheme.x
            canvas.mouseY = (nodeObject.y + ((port > -1) ? parent.y : 0)) + mouse.y + scheme.y
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

                var node = selectNode
                var port = selectPort
                clearSelection()
                showContextMenu(node, port, portObject ? portObject.out : true)
                return
            }
            createMode = false
            canvas.requestPaint()
        }

        onEntered: {
            bullet.color = theme.blueHover
            focusNode = node
            focusPort = port
            canvas.requestPaint()
        }
        onExited: {
            bullet.color = theme.blue
            focusNode = -1
            focusPort = -1
            canvas.requestPaint()
        }
    }

    Rectangle {
        id: bullet
        visible: port !== -1
        width: radius * 2
        height: radius * 2
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: {
            if(portObject === undefined) {
                return undefined
            }
            return (portObject.out) ? parent.right : parent.left
        }
        radius: 4
        color: theme.blue
    }

    Text {
        visible: port !== -1
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: (portObject !== undefined && !portObject.out) ? bullet.right : undefined
        anchors.right: (portObject !== undefined && portObject.out) ? bullet.left : undefined
        text: label
        font.pointSize: theme.textSize
        color: theme.textColor
    }
}
