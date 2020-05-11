import QtQuick 2.0
import QtQuick.Controls 2.3

ScrollBar {
    id: bar
    hoverEnabled: true
    active: hovered || pressed

    property bool isResizable: false

    signal headPositionChanged(real value);
    signal tailPositionChanged(real value);

    Rectangle {
        id: head
        color: "red"

        visible: bar.active && isResizable

        width: 10
        height: 10

        x: parent.width * parent.position

        MouseArea {
            anchors.fill: parent

            drag.target: parent
            drag.axis: (bar.orientation === Qt.Horizontal) ? Drag.XAxis : Drag.YAxis
            drag.minimumX: 0
            drag.maximumX: tail.x
            drag.minimumY: 0
            drag.maximumY: tail.y
            drag.threshold: 0

            cursorShape: (bar.orientation === Qt.Horizontal) ? Qt.SizeHorCursor : Qt.SizeVerCursor

            onPositionChanged: {
                if(drag.active) {
                    if(bar.orientation === Qt.Horizontal) {
                        headPositionChanged((tail.x - head.x + width) / bar.width);
                    } else {
                        headPositionChanged((tail.y - head.y + height) / bar.height);
                    }
                }
            }
        }
    }

    Rectangle {
        id: tail
        color: "blue"

        visible: bar.active && isResizable

        width: 10
        height: 10

        x: (parent.position + parent.size) * parent.width - width

        MouseArea {
            anchors.fill: parent

            drag.target: parent
            drag.axis: (bar.orientation === Qt.Horizontal) ? Drag.XAxis : Drag.YAxis
            drag.minimumX: head.x
            drag.maximumX: bar.width - width
            drag.minimumY: head.y
            drag.maximumY: bar.height - height
            drag.threshold: 0

            cursorShape: (bar.orientation === Qt.Horizontal) ? Qt.SizeHorCursor : Qt.SizeVerCursor

            onPositionChanged: {
                if(drag.active) {
                    if(bar.orientation === Qt.Horizontal) {
                        tailPositionChanged((tail.x - head.x + width) / bar.width);
                    } else {
                        tailPositionChanged((tail.y - head.y + height) / bar.height);
                    }
                }
            }
        }
    }
}
