import QtQuick 2.0

Rectangle {
    width: radius * 2
    height: radius * 2
    radius: 6
    color: theme.blueColor

    property int node: -1
    property int port: -1

    MouseArea {
        id: area
        anchors.fill: parent
        hoverEnabled: true

        onClicked: {
            if(mouse.modifiers & Qt.AltModifier) {
                schemeModel.deleteLinksByNode(node, port)
            } else if(selectNode > -1 && selectNode != node) {
                schemeModel.createLink(selectNode, selectPort, node, port)
                selectNode = -1
                selectPort = -1
                createMode = false
            } else {
                selectNode = node
                selectPort = port
                createMode = true
            }
        }

        onEntered: {
            parent.color = theme.hoverBlueColor
            focusNode = node
            focusPort = port

            canvas.requestPaint()
        }
        onExited: {
            parent.color = theme.blueColor
            focusNode = -1
            focusPort = -1

            canvas.requestPaint()
        }
    }
}
