import QtQuick 2.12
import QtQuick.Controls 2.3

import "qrc:/QML/qml/."

Rectangle {
    id: rect
    color: theme.grey

    Theme {
        id: theme
    }

    signal openProject(string path)
    signal newProject()
    signal importProject()

    Rectangle {
        id: leftPanel
        width: 200
        color: theme.greyDark
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        anchors.left: parent.left

        Item {
            id: rectangle
            height: 40
            anchors.rightMargin: 10
            anchors.leftMargin: 10
            anchors.topMargin: 10
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.top: parent.top

            Image {
                id: image
                x: 0
                y: 0
                width: 40
                height: 40
                source: "qrc:/Images/icons/thunderlight.svg"
            }

            Text {
                anchors.fill: parent
                anchors.leftMargin: 50
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignBottom
                text: "Thunder Engine"
                font.bold: false
                font.pixelSize: 18
                color: theme.textColor
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: Qt.openUrlExternally("http://thunderengine.org")
            }
        }

        ButtonGroup {
            buttons: column.children
        }

        Column {
            id: column
            anchors.top: parent.top
            anchors.topMargin: 100
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.rightMargin: 10
            anchors.leftMargin: 10

            RadioButton {
                checked: true
                text: qsTr("Projects")
                indicator: Rectangle {
                    width: 4
                    height: parent.height
                    visible: parent.checked
                    color: theme.blue
                }
                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 24
                    color: parent.checked ? theme.textColor : theme.greyLight
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: 10
                }
                onClicked: pageLoader.source = "Projects.qml"
            }

            RadioButton {
                text: qsTr("Blog")
                indicator: Rectangle {
                    width: 4
                    height: parent.height
                    visible: parent.checked
                    color: theme.blue
                }
                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 24
                    color: parent.checked ? theme.textColor : theme.greyLight
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: 10
                }
                onClicked: pageLoader.source = "Blog.qml"
            }
/*
            RadioButton {
                text: qsTr("Learn")
                indicator: Rectangle {
                    width: 4
                    height: parent.height
                    visible: parent.checked
                    color: theme.blue
                }
                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 24
                    color: parent.checked ? theme.textColor : theme.greyLight
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: 10
                }
                onClicked: pageLoader.source = "Learn.qml"
            }
*/
        }

        Button {
            y: 500
            height: 30
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.bottom: addProject.top
            anchors.rightMargin: 10
            anchors.leftMargin: 10

            text: qsTr("Import")
            anchors.bottomMargin: 10

            background: Rectangle {
                anchors.fill: parent
                radius: theme.frameRadius
                color: parent.hovered ? theme.blue : "#00000000"
                border.color: parent.hovered ? "#00000000" : theme.blue
            }
            contentItem: Text {
                text: parent.text
                font.bold: false
                font.pixelSize: 16
                color: theme.textColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            onClicked: importProject()
        }

        Button {
            id: addProject
            height: 30
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.rightMargin: 10
            anchors.leftMargin: 10
            hoverEnabled: true

            text: qsTr("New")
            anchors.bottomMargin: 50

            background: Rectangle {
                anchors.fill: parent
                radius: theme.frameRadius
                color: parent.hovered ? theme.blue : "#00000000"
                border.color: parent.hovered ? "#00000000" : theme.blue
            }
            contentItem: Text {
                text: parent.text
                font.bold: false
                font.pixelSize: 16
                color: theme.textColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            onClicked: newProject()
        }
    }

    Item {
        anchors.left: leftPanel.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 1400

        Loader {
            id: pageLoader
            anchors.fill: parent
            source: "Projects.qml"
        }
    }
}
