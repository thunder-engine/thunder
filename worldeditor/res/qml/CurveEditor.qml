import QtQuick 2.0
import QtQuick.Controls 2.3

Rectangle {
    id: curveEditor

    color: "#80808080"
    clip: true

    focus: true

    property int selectInd: -1
    property int selectCol: -1

    property int row: 0
    property int col: clipModel.col
    property variant curve: clipModel.trackData(row)

    property int timeStep: minStep
    property real timeScale: 0.01

    property int minValue: 20
    property int maxValue: 100

    property int valueStep: minStep
    property real valueScale: 0.01

    property int posX: 0
    property int posY: 0

    property real minimum: 0
    property real maximum: 0

    onPosXChanged: canvas.requestPaint()
    onPosYChanged: canvas.requestPaint()

    onValueStepChanged: canvas.requestPaint()

    function toScreenSpaceX(pos) {
        return ((pos / 1000.0) / timeScale) * timeStep + minStep
    }

    function toScreenSpaceY(pos) {
        return (pos / valueScale) * valueStep
    }

    function toLocalSpaceX(pos) {
        return (pos / timeStep) * timeScale
    }

    function toLocalSpaceY(pos) {
        return (pos / valueStep) * valueScale
    }

    Connections {
        target: clipModel
        onLayoutChanged: {
            curve = undefined
            curve = clipModel.trackData(row)
            canvas.requestPaint()
        }

        onRowChanged: {
            var r = clipModel.row
            curve = clipModel.trackData(r)

            minimum = Number.MAX_VALUE
            maximum = Number.MIN_VALUE
            for(var i = 0; i < canvas.componentsNumber; i++) {
                var keysNumber = curve[i].length - 1
                for(var k = 0; k < keysNumber; k++) {
                    var key = curve[i][k + 1]
                    var py = -(key[canvas.offset])

                    minimum = Math.min(py, minimum)
                    maximum = Math.max(py, maximum)
                }
            }
            var size = Math.abs(maximum - minimum)
            var value = 0.01
            while(true) {
                var v = value * 10.0
                if((size / v) <= 0.5) {
                    break;
                }
                value = v
            }
            valueScale = value
            valueStep = maxValue

            canvas.requestPaint()

            row = r
        }
    }

    onCurveChanged: {
        if(curve !== undefined) {
            canvas.componentsNumber = curve.length
            canvas.requestPaint()
        }
    }

    Keys.onPressed: {
        if(event.key === Qt.Key_Delete) {
            removeKey(row, selectCol, selectInd)
            selectInd = -1
            selectCol = -1
        }
    }


    Canvas {
        id: canvas
        anchors.fill: parent
        contextType: "2d"

        antialiasing: false

        property int componentsNumber: 0

        property int offset: 2
        property int leftOffset: offset + 1
        property int rightOffset: leftOffset + 1

        property int dist: 50

        onPaint: {
            context.clearRect(0, 0, canvas.width, canvas.height);
            if(curveEditor.curve !== undefined) {
                context.translate(-posX, posY)

                for(var i = 0; i < componentsNumber; i++) {
                    if(col > -1 && i != col) {
                        continue
                    }

                    context.strokeStyle = theme.colors[i]
                    context.fillStyle = Qt.rgba(1,0.5,0)
                    context.beginPath()

                    var keysNumber = curve[i].length - 1

                    var key = curve[i][1]
                    context.moveTo(toScreenSpaceX(key[0]), -toScreenSpaceY(key[offset]))

                    for(var k = 0; k < keysNumber; k++) {
                        var key1 = curve[i][k + 1]
                        var px1 = toScreenSpaceX(key1[0])
                        var py1 = -toScreenSpaceY(key1[offset])

                        var tx0 = px1
                        var ty0 = py1

                        var d = 0
                        if((k - 1) >= 0) {
                            var key0 = curve[i][k]
                            var px0 = toScreenSpaceX(key0[0])
                            var py0 = -toScreenSpaceY(key0[offset])

                            d = (px1 - px0) * 0.5

                            tx0 = px0 + d
                            ty0 = -toScreenSpaceY(key0[rightOffset])
                        }
                        var tx1 = px1 - d
                        var ty1 = -toScreenSpaceY(key1[leftOffset])

                        context.bezierCurveTo(tx0,ty0, tx1,ty1, px1,py1)
                    }
                    context.stroke()

                    if(selectInd >= 0 && selectCol == i) {
                        context.strokeStyle = Qt.rgba(0.3, 0.3, 0.3)

                        key1 = curve[selectCol][selectInd + 1]
                        px1 = toScreenSpaceX(key1[0])
                        py1 = -toScreenSpaceY(key1[offset])
                        // Right tangent
                        d = 0
                        if((selectInd + 1) < (curve[selectCol].length - 1)) {
                            key0 = curve[selectCol][selectInd + 2]
                            d = (px1 - toScreenSpaceX(key0[0])) * 0.5

                            tx1 = -d
                            ty1 = -toScreenSpaceY(key1[rightOffset]) - py1

                            var l = (1.0 / Math.sqrt(tx1 * tx1 + ty1 * ty1))
                            tx1 = l * tx1 * dist
                            ty1 = l * ty1 * dist

                            context.beginPath()
                            context.moveTo(px1, py1)
                            context.lineTo(tx1 + px1, ty1 + py1)
                            context.stroke()
                            context.fillRect(tx1 + px1 - 2, ty1 + py1 - 2, 4, 4);
                        }

                        // Left tangent
                        d = 0
                        if((selectInd - 1) >= 0) {
                            key0 = curve[selectCol][selectInd]
                            d = (px1 - toScreenSpaceX(key0[0])) * 0.5

                            tx1 = -d
                            ty1 = -toScreenSpaceY(key1[leftOffset]) - py1

                            l = (1.0 / Math.sqrt(tx1 * tx1 + ty1 * ty1))
                            tx1 = l * tx1 * dist
                            ty1 = l * ty1 * dist

                            context.beginPath()
                            context.moveTo(px1, py1)
                            context.lineTo(tx1 + px1, ty1 + py1)
                            context.stroke()
                            context.fillRect(tx1 + px1 - 2, ty1 + py1 - 2, 4, 4);
                        }
                    }
                }
                context.setTransform(1, 0, 0, 1, 0, 0)
            }
        }
    }

    Repeater {
        model: Math.ceil(curveEditor.height / valueStep)
        Item {
            anchors.left: curveEditor.left
            anchors.right: curveEditor.right
            y: index * valueStep + (posY % valueStep)

            property int shift: posY / valueStep
            Label {
                anchors.bottom: parent.top
                anchors.left: parent.left
                color: theme.textColor
                text: {
                    var value = -(index - parent.shift) * valueScale
                    return value.toLocaleString(Qt.locale("en_EN"), 'f', 2)
                }
                font.pointSize: 8
                renderType: Text.NativeRendering
            }
        }
    }

    Repeater {
        model: (curve === undefined) ? 0 : ((curveEditor.col > -1) ? 1 : canvas.componentsNumber)
        Repeater {
            id: points
            model: curve[col].length - 1

            property int col: (curveEditor.col > -1) ? curveEditor.col : index
            property int component: curve[col][0]

            property int pointSize: 6
            property int pointCenter: pointSize * 0.5

            Item {
                id: item
                property variant key: curve[points.col][index + 1]
                property bool breaked: false

                x: toScreenSpaceX(key[0]) - posX - points.pointCenter
                y: -toScreenSpaceY(key[canvas.offset]) + posY - points.pointCenter

                function commitKey() {
                    var data = curve
                    data[points.col][index + 1] = item.key
                    curve = data
                }

                Rectangle {
                    color: (selectInd == index && selectCol == points.col) ? theme.blueHover : "#a0606060"
                    border.color: theme.textColor

                    height: points.pointSize
                    width: points.pointSize

                    rotation: 45

                    MouseArea {
                        anchors.fill: parent

                        acceptedButtons: Qt.LeftButton | Qt.RightButton

                        drag.target: item
                        drag.axis: Drag.XAxis | Drag.YAxis
                        drag.minimumX: 0
                        drag.maximumX: rect.width
                        drag.minimumY: 0
                        drag.maximumY: rect.height
                        drag.threshold: 0

                        drag.onActiveChanged: {
                            if(!drag.active) {
                                selectKey(row, selectCol, selectInd)
                                clipModel.setTrackData(curveEditor.row, curveEditor.curve)
                            }
                        }

                        onPressed: {
                            selectInd = index
                            selectCol = points.col

                            selectKey(row, selectCol, selectInd)

                            canvas.requestPaint()

                            xLabel.visible = true
                            xLabel.x = item.x

                            yLabel.visible = true
                            yLabel.y = item.y
                        }

                        onReleased: {
                            xLabel.visible = false
                            yLabel.visible = false
                        }

                        onPositionChanged: {
                            if(drag.active) {
                                xLabel.x = item.x
                                yLabel.y = item.y

                                var x = Math.round(item.x / timeStep) * timeStep - points.pointSize

                                item.key[0] = Math.max(Math.round((x + posX) / timeStep), 0) * timeScale * 1000
                                var value = (-((item.y - posY + 3) / valueStep) * valueScale) - item.key[canvas.offset]
                                item.key[canvas.offset] += value
                                item.key[canvas.leftOffset]  += value
                                item.key[canvas.rightOffset] += value

                                item.commitKey()
                            }
                        }
                    }
                }

                Item {
                    id: leftTangent
                    visible: (selectInd == index) && (selectCol == points.col) && (index > 0)
                    height: 16
                    width: 16

                    property real d: {
                        var result = 0
                        if(index > 0) {
                            var key0 = curve[points.col][index]
                            result = (toScreenSpaceX(item.key[0]) - toScreenSpaceX(key0[0])) * 0.5
                        }

                        var py1 = -toScreenSpaceY(item.key[canvas.offset])
                        var tx1 = -result
                        var ty1 = -toScreenSpaceY(item.key[canvas.leftOffset]) - py1

                        var l = (1.0 / Math.sqrt(tx1 * tx1 + ty1 * ty1))
                        x = l * tx1 * canvas.dist - width * 0.5
                        y = l * ty1 * canvas.dist - height * 0.5

                        return result
                    }

                    MouseArea {
                        anchors.fill: parent

                        drag.target: parent
                        drag.axis: Drag.XAxis | Drag.YAxis
                        drag.minimumX:-canvas.dist
                        drag.maximumX: 0
                        drag.minimumY:-canvas.dist
                        drag.maximumY: canvas.dist
                        drag.threshold: 0

                        drag.onActiveChanged: {
                            if(!drag.active) {
                                clipModel.setTrackData(row, curve)
                            }
                        }

                        onPositionChanged: {
                            if(drag.active) {
                                var value = toLocalSpaceY((parent.y / parent.x) * parent.d)
                                item.key[canvas.leftOffset] = item.key[canvas.offset] + value
                                if(!item.breaked) {
                                    item.key[canvas.rightOffset] = item.key[canvas.offset] - value
                                }
                                item.commitKey()
                            }
                        }
                    }
                }

                Item {
                    id: rightTangent
                    visible: (selectInd == index) && (selectCol == points.col) && ((index + 1) < (curve[points.col].length - 1))
                    height: 16
                    width: 16

                    property real d: {
                        var result = 0
                        if((index + 1) < (curve[points.col].length - 1)) {
                            var key0 = curve[points.col][index + 2]
                            result = (toScreenSpaceX(item.key[0]) - toScreenSpaceX(key0[0])) * 0.5
                        }

                        var py1 = -toScreenSpaceY(item.key[canvas.offset])
                        var tx1 = -result
                        var ty1 = -toScreenSpaceY(item.key[canvas.rightOffset]) - py1

                        var l = (1.0 / Math.sqrt(tx1 * tx1 + ty1 * ty1))
                        x = l * tx1 * canvas.dist - width * 0.5
                        y = l * ty1 * canvas.dist - height * 0.5

                        return result
                    }

                    MouseArea {
                        anchors.fill: parent

                        drag.target: parent
                        drag.axis: Drag.XAxis | Drag.YAxis
                        drag.minimumX: 0
                        drag.maximumX: canvas.dist
                        drag.minimumY:-canvas.dist
                        drag.maximumY: canvas.dist
                        drag.threshold: 0

                        drag.onActiveChanged: {
                            if(!drag.active) {
                                clipModel.setTrackData(row, curve)
                            }
                        }

                        onPositionChanged: {
                            if(drag.active) {
                                var value = toLocalSpaceY((parent.y / parent.x) * parent.d)
                                item.key[canvas.rightOffset] = item.key[canvas.offset] + value
                                if(!item.breaked) {
                                    item.key[canvas.leftOffset] = item.key[canvas.offset] - value
                                }
                                item.commitKey()
                            }
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        id: xLabel
        visible: false
        anchors.top: parent.top
        color: "#a0000000"
        radius: 3

        width: xlabel.width + 6
        height: xlabel.height

        Label {
            id: xlabel
            color: theme.textColor
            text: {
                if(selectInd >= 0 && selectCol >= 0 && curve !== undefined) {
                    var key = curve[selectCol][selectInd + 1]
                    if(key !== undefined) {
                        var value = key[0] / 1000.0
                        return value.toLocaleString(Qt.locale("en_EN"), 'f', 3)
                    }
                }
                return ""
            }
            anchors.centerIn: parent
        }
    }

    Rectangle {
        id: yLabel
        visible: false
        anchors.left: parent.left
        color: "#a0000000"
        radius: 3

        width: ylabel.width + 6
        height: ylabel.height

        Label {
            id: ylabel
            color: theme.textColor
            text: {
                if(selectInd >= 0 && selectCol >= 0 && curve !== undefined) {
                    var key = curve[selectCol][selectInd + 1]
                    if(key !== undefined) {
                        var value = key[canvas.offset]
                        return value.toLocaleString(Qt.locale("en_EN"), 'f', 3) * 1
                    }
                }
                return ""
            }
            anchors.centerIn: parent
        }
    }
}
