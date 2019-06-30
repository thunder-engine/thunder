import QtQuick 2.0
import QtQuick.Controls 2.3

Item {
    anchors.fill: parent
    GridView {
        id: gridView
        cellHeight: 180
        cellWidth: 200
        anchors.margins: 20
        anchors.fill: parent
        model: projectsModel
        delegate: Rectangle {
            x: 5
            width: 190
            height: 170
            color: theme.greyDark
            radius: theme.frameRadius

            Column {
                spacing: 5
                anchors.fill: parent
                Image {
                    width: 128
                    height: 128
                    source: icon
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                Rectangle {
                    width: parent.width
                    height: 1
                    color: theme.blue
                }
                Text {
                    text: name
                    anchors.horizontalCenter: parent.horizontalCenter
                    verticalAlignment: Text.AlignBottom
                    color: theme.textColor
                    font.bold: false
                    font.pixelSize: 14
                }
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onClicked: openProject(path)
                onEntered: parent.color = "#404040"
                onExited: parent.color = theme.greyDark
            }
        }
    }
}
