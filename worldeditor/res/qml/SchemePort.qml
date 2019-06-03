import QtQuick 2.0

Rectangle {
    width: radius * 2
    height: radius * 2
    radius: 6
    color: theme.blue

    property int node: -1
    property int port: -1

    MouseArea {
        id: area
        anchors.fill: parent
        hoverEnabled: true

        onClicked: {
            if(mouse.modifiers & Qt.AltModifier) {
                schemeModel.deleteLinksByNode(node, port)
            } else if(createMode && selectNode != node) {
                schemeModel.createLink(selectNode, selectPort, node, port)
                selectNode = -1
                selectPort = -1
                createMode = false
            } else {
                nodeSelect(node)
                selectPort = port
                createMode = true
            }
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
