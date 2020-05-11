import QtQuick 2.0

Rectangle {
    color: (selectInd == index && selectRow == row && selectCol == col) ? theme.blueHover : "#a0606060"
    border.color: theme.textColor
    border.width: 1

    height: 7
    width: 7

    y: 2
    rotation: 45

    property variant key: undefined

    function commitKey() {
        var data = keys.curve
        data[component.col][index + 1] = key
        keys.curve = data
    }

    MouseArea {
        anchors.fill: parent

        drag.target: parent
        drag.axis: Drag.XAxis
        drag.minimumX: 0
        drag.maximumX: keyframeEditor.width
        drag.threshold: 0

        drag.onActiveChanged: {
            if(!drag.active) {
                clipModel.setTrackData(keys.row, keys.curve)
            }
        }

        onPressed: {
            selectInd = index
            selectRow = row
            selectCol = col

            selectKey(row, col, index)
        }

        onPositionChanged: {
            if(drag.active) {
                var x = Math.round(parent.x / timeStep) * timeStep - points.pointSize

                parent.key[0] = Math.max(Math.round((x + posX) / timeStep), 0) * timeScale * 1000
                parent.commitKey()
            }
        }
    }
}
