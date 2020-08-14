import QtQuick 2.10
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

    property int cell: 16
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

    signal nodesSelected(variant indices)
    signal linksSelected(variant indices)

    FontMetrics {
        id: fontMetrics
        font.pointSize: 12
    }

    function nodeWidth(node) {
        return Math.max(Math.round((fontMetrics.advanceWidth(node.name) + 80) / cell) * cell, 10 * cell)
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
        var data = nodes
        var dx = x - nodes[id].pos.x
        var dy = y - nodes[id].pos.y
        data[id].pos.x += dx
        data[id].pos.y += dy

        for(var i = 0; i < selection.length; i++) {
            var index = selection[i]
            if(index === 0 || index === id) {
                continue
            }
            data[index].pos.x += dx
            data[index].pos.y += dy
        }
        nodes = data
    }

    function nodeSelect(id) {
        selectNode = id
        selection = [id]
        nodesSelected(selection)
        selectPort = -1
        createMode = false
    }

    function deleteSelected() {
        if(selection.length > 0) {
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

            if(mouse.buttons & Qt.RightButton) {
                canvas.translateX += mouse.x - oldX
                canvas.translateY += mouse.y - oldY
            }

            canvas.requestPaint()

            oldX = mouse.x
            oldY = mouse.y

            if(rubberBand.visible) {
                if(mouse.x < rubberBand.sx) {
                    rubberBand.x = mouse.x
                    rubberBand.width = Math.abs(rubberBand.sx - mouse.x)
                } else {
                    rubberBand.x = rubberBand.sx
                    rubberBand.width = Math.abs(mouse.x - rubberBand.sx)
                }
                if(mouse.y < rubberBand.sy) {
                    rubberBand.y = mouse.y
                    rubberBand.height = Math.abs(rubberBand.sy - mouse.y)
                } else {
                    rubberBand.y = rubberBand.sy
                    rubberBand.height = Math.abs(mouse.y - rubberBand.sy)
                }
            }
        }

        onClicked: {
            if(selection.length === 0) {
                nodesSelected([])
            }
            selectPort = -1
            createMode = false

            canvas.requestPaint()
        }

        onPressed: {
            if(mouse.buttons & Qt.LeftButton) {
                rubberBand.width = 0
                rubberBand.height = 0
                rubberBand.sx = mouse.x
                rubberBand.sy = mouse.y
                rubberBand.visible = true
            }
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
                nodesSelected(selection)
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
        property string hoverLinkColor: "red"

        property int mouseX: 0
        property int mouseY: 0

        onPaint: {
            context.clearRect(0, 0, canvas.width, canvas.height)
            context.strokeStyle = theme.greyLight
            context.translate(translateX, translateY)
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
                    context.strokeStyle = hoverLinkColor
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
                    strokeColor: linkObject.color
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
            property bool isSelected: (selection.indexOf(index) !== -1)

            function isCollide(x1, y1, width1, height1, x2, y2, width2, height2) {
                return !(x1 > x2 + width2 || x1 + width1 < x2 ||
                         y1 > y2 + height2 || y1 + height1 < y2)
            }

            Rectangle {
                id: nodeBody
                x: nodeBorder
                y: nodeBorder
                width: parent.width - (nodeBorder * 2)
                height: parent.height - (nodeBorder * 2)
                radius: 4
                color: theme.greyDark

                border.color: theme.blue
                border.width: (nodeObject.isFocus || nodeObject.isSelected) ? 2 : 0

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    drag.target: (index != 0) ? nodeObject : undefined
                    drag.threshold: 0

                    drag.onActiveChanged: {
                        if(!drag.active) {
                            schemeModel.moveNode(selection, nodes)
                        } else {
                            if(selection.indexOf(nodeObject.node) === -1) {
                                var array = selection
                                array.push(nodeObject.node)

                                selection = array
                                nodesSelected(selection)
                            }
                        }
                    }

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
                        if((selection.length > 0) && ((mouse.modifiers & Qt.ControlModifier) || (mouse.modifiers & Qt.ShiftModifier))) {
                            var array = selection
                            array.push(nodeObject.node)

                            selection = array
                            nodesSelected(selection)
                        } else {
                            nodeSelect(nodeObject.node)
                        }
                    }
                }

                Text {
                    id: name
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    text: nodes[nodeObject.node].name
                    font.pointSize: fontMetrics.font.pointSize
                    color: theme.textColor
                }

                Rectangle {
                    anchors.top: name.bottom
                    anchors.topMargin: cell / 2
                    width: parent.width
                    height: 1
                    color: theme.blue
                    visible: !stateMachine
                }

                Repeater {
                    model: nodes[nodeObject.node].ports.length
                    SchemePort {
                        x: {
                            if(portObject.out) {
                                return nodeObject.width - width
                            }
                            return 0
                        }
                        y: portObject.pos * (cell * 2) + (cell * 2)

                        height: (cell * 2)
                        width: nodeObject.width / 2

                        node: nodeObject.node
                        port: index

                        label: portObject.name

                        portObject: nodes[nodeObject.node].ports[index]
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
        property int sx: 0
        property int sy: 0
    }


    Shortcut {
        sequence: StandardKey.Delete
        onActivated: {
            deleteSelected()

            if(selectLink > -1) {
                schemeModel.deleteLink(selectLink)
                selectLink = -1
                linkSelected(selectLink)
                focusLink = -1
            }
        }
    }

    Shortcut {
        sequence: StandardKey.Copy
        onActivated: {
            if(selection.length > -1) {
                schemeModel.copyNodes(selection)
            }
        }
    }

    Shortcut {
        sequence: StandardKey.Paste
        onActivated: {
            var result = schemeModel.pasteNodes(canvas.mouseX, canvas.mouseY)
            selection = []
            for(var i = 0; i < result.length; i++) {
                selection.push(result[i])
            }
        }
    }

    focus: true
}
