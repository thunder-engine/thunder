import QtQuick 2.11
import QtGraphicalEffects 1.0
import QtQuick.Shapes 1.0

import "qrc:/QML/qml/."

Rectangle {
    id: rect

    antialiasing: false
    color: theme.backColor

    Theme {
        id: theme
    }

    Connections {
        target: schemeModel
        onSchemeUpdated: {
            nodes = schemeModel.nodes()
            links = schemeModel.links()
            canvas.requestPaint()
        }
        onNodeMoved: {
            nodes = schemeModel.nodes()
            links = schemeModel.links()
            canvas.requestPaint()
        }
    }

    property variant nodes: undefined
    property variant links: undefined

    property variant selection: []

    property int cell: 20
    property int nodeBorder: (stateMachine) ? cell / 2 : 0

    property int focusNode: -1
    property int focusPort: -1
    property int focusLink: -1

    property int selectNode: -1
    property int selectPort: -1
    property int selectLink: -1

    property bool createMode: false

    signal nodeSelected(int index)
    signal linkSelected(int index)

    FontMetrics {
        id: fontMetrics
        font.pointSize: 8
    }

    function nodeWidth(node) {
        return Math.max(fontMetrics.advanceWidth(node.name) + 80, 100)
    }

    function nodeHeight(node) {
        var iport = 0;
        var oport = 0;
        for(var i = 0; i < node.ports.length; i++) {
            if(node.ports[i].out) {
                oport++;
            } else {
                iport++;
            }
        }
        return (Math.max(iport, oport) + 1) * (cell * 2)
    }

    function moveNodes(id, x, y) {
        var dx = x - nodes[id].pos.x
        var dy = y - nodes[id].pos.y
        schemeModel.moveNode(id, nodes[id].pos.x + dx, nodes[id].pos.y + dy)

        for(var i = 0; i < selection.length; i++) {
            var index = selection[i]
            if(index === 0 || index === id) {
                continue
            }
            schemeModel.moveNode(index, nodes[index].pos.x + dx, nodes[index].pos.y + dy)
        }
    }

    function nodeSelect(id) {
        selectNode = id
        nodeSelected(selectNode)
        selection = [id]
    }

    function deleteSelected() {
        if(selection.length > -1) {
            schemeModel.deleteNodes(selection)
            nodeSelect(0)
        }
    }

    MouseArea {
        id: rootArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        property int oldX: 0
        property int oldY: 0

        onPositionChanged: {
            canvas.mouseX = mouse.x - canvas.translateX
            canvas.mouseY = mouse.y - canvas.translateY

            canvas.requestPaint()

            if(mouse.buttons & Qt.RightButton) {
                canvas.translateX += mouse.x - oldX
                canvas.translateY += mouse.y - oldY
                canvas.requestPaint()
            }
            oldX = mouse.x
            oldY = mouse.y

            if(rubberBand.visible) {
                rubberBand.width = mouse.x - rubberBand.x
                rubberBand.height = mouse.y - rubberBand.y
            }
        }

        onClicked: {
            if(selection.length === 0) {
                nodeSelect(0)
            }
            selectPort = -1

            canvas.requestPaint()
        }

        onPressed: {
            rubberBand.x = mouse.x
            rubberBand.y = mouse.y
            rubberBand.width = 0
            rubberBand.height = 0
            rubberBand.visible = true

            selection = []
        }

        onReleased: {
            rubberBand.visible = false
            if(nodes !== undefined) {
                var array = new Array
                for(var i = 0; i < nodes.length; i++) {
                    if(nodes[i].focus !== undefined) {
                        nodes[i].focus = undefined
                        array.push(i)
                    }
                }
                selection = array
            }
        }
    }

    DropArea {
        anchors.fill: parent
        onDropped: {
            if(drop.keys[0] === "text/component") {
                schemeModel.createNode(drop.getDataAsArrayBuffer(drop.keys[0]),
                                       drop.x - canvas.translateX, drop.y - canvas.translateY);
            }
        }
    }

    Canvas {
        id: canvas
        anchors.fill: parent
        contextType: "2d"
        antialiasing: true

        objectName: "Canvas"

        property int translateX: rect.width / 2
        property int translateY: rect.height / 2
        property string linkColor: "white"

        property int mouseX: 0
        property int mouseY: 0

        onPaint: {
            context.clearRect(0, 0, canvas.width, canvas.height)
            context.strokeStyle = theme.greyLight
            context.translate(translateX, translateY)
            // Grid
            var beforeX = Math.round(-translateX / cell)
            var afterX = Math.round((canvas.width - translateX) / cell)
            for(var x = beforeX; x < afterX; x++) {
                context.lineWidth = (x % 9 == 0) ? 2 : 1
                context.beginPath()
                context.moveTo(x * cell, -translateY)
                context.lineTo(x * cell, canvas.height - translateY)
                context.stroke()
            }

            var beforeY = Math.round(-translateY / cell)
            var afterY = Math.round((canvas.height - translateY) / cell)
            for(var y = beforeY; y < afterY; y++) {
                context.lineWidth = (y % 9 == 0) ? 2 : 1
                context.beginPath()
                context.moveTo(-translateX, y * cell)
                context.lineTo(canvas.width - translateX, y * cell)
                context.stroke()
            }
            // Creation link
            var border = nodeBorder * 2
            context.strokeStyle = linkColor
            context.lineWidth = 2
            if(selectNode > -1 && createMode) {
                var x0 = nodes[selectNode].pos.x
                var y0 = nodes[selectNode].pos.y
                var width = nodeWidth(nodes[selectNode])
                if(selectPort > -1) {
                    var port = nodes[selectNode].ports[selectPort]
                    if(port !== undefined) {
                        x0 += (port.out) ? width : 0
                        y0 += port.pos * (cell * 2) + (cell * 3)

                        var length = Math.abs(x0 - mouseX) * 0.5;

                        context.beginPath()
                        if(port.out) {
                            context.moveTo(x0, y0)
                            context.bezierCurveTo(x0 + length, y0,
                                                  mouseX - length, mouseY,
                                                  mouseX, mouseY)
                        } else {
                            context.moveTo(mouseX, mouseY)
                            context.bezierCurveTo(mouseX + length, mouseY,
                                                  x0 - length, y0,
                                                  x0, y0)
                        }
                        context.stroke()
                    }
                } else {
                    x0 += (width + border) / 2
                    y0 += (nodeHeight(nodes[selectNode]) + border) / 2

                    context.beginPath()
                    context.moveTo(x0, y0)
                    context.lineTo(mouseX, mouseY)
                    context.stroke()
                }
            }

            // Links
            if(links !== undefined && !stateMachine) {
                for(var i = 0; i < links.length; i++) {
                    context.strokeStyle = ((focusNode === links[i].sender && focusPort === links[i].oport) ||
                                           (focusNode === links[i].receiver && focusPort === links[i].iport)) ? "red" : "white"
                    context.fillStyle = context.strokeStyle

                    var oport = nodes[links[i].sender].ports[links[i].oport]
                    var iport = nodes[links[i].receiver].ports[links[i].iport]

                    if(oport !== undefined && iport !== undefined) {
                        var x1 = nodes[links[i].sender].pos.x + ((oport.out) ? nodeWidth(nodes[links[i].sender]) - radius : -radius)
                        var y1 = nodes[links[i].sender].pos.y + oport.pos * (cell * 2) + (cell * 3) - radius

                        var x2 = nodes[links[i].receiver].pos.x + ((iport.out) ? nodeWidth(nodes[links[i].receiver]) - radius : -radius)
                        var y2 = nodes[links[i].receiver].pos.y + iport.pos * (cell * 2) + (cell * 3) - radius

                        length = Math.abs(x2 - x1) * 0.5;

                        context.beginPath()
                        context.moveTo(x1, y1)
                        context.bezierCurveTo(x1 + length, y1,
                                              x2 - length, y2,
                                              x2, y2)
                        context.stroke()
                    }
                }
            }
            context.setTransform(1, 0, 0, 1, 0, 0)
        }
    }

    Repeater {
        model: (links !== undefined && stateMachine) ? links.length : 0
        Rectangle {
            id: linkObject
            x: nodes[sender].pos.x + (nodeWidth(nodes[sender]) + (nodeBorder * 2)) / 2 + canvas.translateX
            y: nodes[sender].pos.y + (nodeHeight(nodes[sender]) + (nodeBorder * 2)) / 2 + canvas.translateY

            width: 2
            height: Math.sqrt(dx * dx + dy * dy)
            rotation: -Math.atan2(dx, dy) * (180.0 / Math.PI)
            transformOrigin: Item.Top

            color: (focusNode === sender || focusNode === receiver ||
                    focusLink === index || selectLink === index) ? "red" : "white"

            transform: Translate {
                x: -offset * Math.cos(rotation * (Math.PI / 180.0))
                y: -offset * Math.sin(rotation * (Math.PI / 180.0))
            }

            property int sender: links[index].sender
            property int receiver: links[index].receiver
            property int offset: {
                for(var i = 0; i < links.length; i++) {
                    if(links[i].sender === receiver && links[i].receiver === sender) {
                        if(i > index) {
                            return 16
                        }
                        break
                    }
                }
                return 0
            }

            property int dx: nodes[receiver].pos.x + (nodeWidth(nodes[receiver]) + (nodeBorder * 2)) / 2 + canvas.translateX - x
            property int dy: nodes[receiver].pos.y + (nodeHeight(nodes[receiver]) + (nodeBorder * 2)) / 2 + canvas.translateY - y

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true

                onEntered: focusLink = index
                onExited: focusLink = -1
                onClicked: {
                    linkSelected(index)
                    selectLink = index
                }
            }

            Shape {
                width: 12
                height: 12
                anchors.centerIn: parent
                ShapePath {
                    strokeWidth: 0
                    fillColor: linkObject.color
                    startX: 0; startY: 0
                    PathLine { x: 6; y:12 }
                    PathLine { x:12; y: 0 }
                }
            }

        }
    }

    Repeater {
        id: nodeRepeater
        model: (nodes !== undefined) ? nodes.length : 0
        SchemePort {
            id: nodeObject
            x: nodes[index].pos.x + canvas.translateX
            y: nodes[index].pos.y + canvas.translateY

            width: nodeWidth(nodes[index]) + (nodeBorder * 2)
            height: nodeHeight(nodes[index]) + (nodeBorder * 2)

            node: index

            property bool isFocus: {
                var result = (rubberBand.visible && isCollide(nodeObject.x, nodeObject.y, nodeObject.width, nodeObject.height,
                                                              rubberBand.x, rubberBand.y, rubberBand.width, rubberBand.height))

                if(result === true) {
                    nodes[index].focus = true
                }
                return result
            }
            property bool isSelected: false

            Connections {
                target: rect
                onSelectionChanged: {
                    isSelected = (selection.indexOf(index) !== -1)
                }
            }

            function isCollide(x1, y1, width1, height1, x2, y2, width2, height2) {
                return !(x1 > x2 + width2 || x1 + width1 < x2 ||
                         y1 > y2 + height2 || y1 + height1 < y2)
            }

            Rectangle {
                x: nodeBorder
                y: nodeBorder
                width: parent.width - (nodeBorder * 2)
                height: parent.height - (nodeBorder * 2)
                radius: 4
                color: "#e0e0e0"

                border.color: theme.red
                border.width: (nodeObject.isFocus || nodeObject.isSelected) ? 2 : 0

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    drag.target: (index != 0) ? nodeObject : undefined
                    drag.threshold: 0

                    onPositionChanged: {
                        if(drag.active) {
                            moveNodes(node,
                                      Math.round((nodeObject.x - canvas.translateX) / cell) * cell,
                                      Math.round((nodeObject.y - canvas.translateY) / cell) * cell)
                        }
                        canvas.mouseX = nodeObject.x + mouse.x - canvas.translateX
                        canvas.mouseY = nodeObject.y + mouse.y - canvas.translateY
                        canvas.requestPaint()
                    }
                    onClicked: {
                        nodeSelect(nodeObject.node)
                    }
                }

                Text {
                    id: name
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    text: nodes[nodeObject.node].name
                    font.pointSize: 12
                }

                Repeater {
                    model: nodes[nodeObject.node].ports.length
                    SchemePort {
                        x: {
                            if(portObject.out) {
                                return nodeObject.width - radius
                            }
                            return -radius
                        }
                        y: portObject.pos * (cell * 2) + (cell * 3) - radius

                        node: nodeObject.node
                        port: index

                        property variant portObject: nodes[nodeObject.node].ports[index]

                        Text {
                            anchors.leftMargin: 8
                            anchors.rightMargin: 8
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: (!portObject.out) ? parent.right : undefined
                            anchors.right: (portObject.out) ? parent.left : undefined
                            text: portObject.name
                            font.pointSize: 12
                        }
                    }
                }

            }
        }
    }

    Rectangle {
        id: rubberBand
        visible: false
        color: theme.emitterColor;
        border.width: 1
        border.color: theme.hoverColor
    }

    Keys.onPressed: {
        if(event.key === Qt.Key_Delete) {
            deleteSelected()

            if(selectLink > -1) {
                schemeModel.deleteLink(selectLink)
                selectLink = -1
                linkSelected(selectLink)
                focusLink = -1
            }
        } else if(event.key === Qt.Key_V) {

        }
    }
    focus: true
}
