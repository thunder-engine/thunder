import QtQuick 2.0
import QtQuick.Controls 2.3
import QtGraphicalEffects 1.0

import "qrc:/QML/qml/."

Rectangle {
    id: rectangle

    color: "#757575"

    Theme {
        id: theme
    }

    signal emitterSelected(string emitter)
    signal emitterCreate()
    signal emitterDelete(string emitter)

    signal functionSelected(string emitter, string name)
    signal functionCreate(string emitter, string name)
    signal functionDelete(string emitter, string name)

    ListView {
        id: emittersList
        anchors.fill: parent
        anchors.margins: 3
        spacing: 3
        model: effectModel
        orientation: ListView.Horizontal
        delegate: emitterDelegate

        footer: Component {
            Item {
                width: 64
                anchors.top: parent.top
                anchors.bottom: parent.bottom

                Rectangle {
                    id: addEmitter
                    anchors.fill: parent
                    anchors.leftMargin: 3
                    radius: 3
                    color: theme.emitterColor

                    Text {
                        id: nameLabel
                        anchors.fill: parent
                        text: "+"
                        color: theme.textColor
                        font.pointSize: theme.textSize * 2
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: addEmitter.color = theme.hoverColor
                        onExited: addEmitter.color = theme.emitterColor
                        onClicked: {
                            emitterCreate()
                        }
                    }
                }
            }
        }
    }

    Component {
        id: emitterDelegate
        Rectangle {
            id: emitter
            width: 200
            height: parent.height
            radius: 3
            color: theme.emitterColor

            Image {
                source: "file:///" + _IconPath
                width: 48
                height: 48
                anchors.margins: 8
                anchors.left: parent.left
                anchors.top: parent.top
            }

            Text {
                id: nameLabel
                anchors.left: parent.left
                anchors.right: parent.right
                height: 64
                text: Name
                font.pointSize: theme.textSize
                color: theme.textColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: emitter.color = theme.hoverColor
                    onExited: emitter.color = theme.emitterColor
                    onClicked: {
                        emitterSelected(Name)
                    }
                }
            }

            ListView {
                id: functionList
                anchors.fill: parent
                anchors.margins: 3
                anchors.topMargin: 64
                spacing: 3
                clip: true
                model: _ChildModel

                delegate: Component {
                    Rectangle {
                        id: emitterFunction
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: 32
                        radius: 3
                        color: theme.functionColor

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: emitterFunction.color = theme.hoverColor
                            onExited: emitterFunction.color = theme.functionColor
                            onClicked: {
                                functionSelected(Name, modelData)
                            }
                        }

                        Image {
                            id: remove
                            anchors.top: parent.top
                            anchors.right: parent.right
                            anchors.margins: 4
                            width: 24
                            height: 24
                            source: "qrc:/Images/fontawesome/close.svg"
                            visible: false
                        }

                        ColorOverlay {
                            anchors.fill: remove
                            source: remove
                            color: "#60ffffff"

                            MouseArea {
                                anchors.fill: parent
                                hoverEnabled: true
                                onEntered: parent.color = "#80ffffff"
                                onExited: parent.color = "#60ffffff"
                                onClicked: {
                                    functionDelete(Name, modelData)
                                }
                            }
                        }

                        Text {
                            id: nameLabel
                            anchors.fill: parent
                            anchors.margins: 3
                            text: modelData
                            color: theme.textColor
                            font.pointSize: theme.textSize
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                    }
                }

                Rectangle {
                    id: deleteEmitter
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.topMargin: 3
                    height: 32
                    radius: theme.frameRadius
                    color: "#00000000"
                    border.color: theme.red

                    Text {
                        id: deleteLabel
                        anchors.fill: parent
                        text: "Delete Emitter"
                        color: theme.red
                        font.pointSize: theme.textSize
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: {
                            deleteEmitter.color = theme.redHover
                            deleteLabel.color = theme.textColor
                            deleteEmitter.border.color = "#00000000"
                        }
                        onExited: {
                            deleteEmitter.color = "#00000000"
                            deleteLabel.color = theme.red
                            deleteEmitter.border.color = theme.red
                        }
                        onClicked: {
                            emitterDelete(Name)
                        }
                    }
                }

                header: Component {
                    Item {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: 32

                        Rectangle {
                            id: addFunction
                            anchors.fill: parent
                            anchors.bottomMargin: 3
                            radius: theme.frameRadius
                            color: theme.green

                            Text {
                                id: nameLabel
                                anchors.fill: parent
                                text: "Add Modifier"
                                color: theme.textColor
                                font.pointSize: theme.textSize
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            MouseArea {
                                anchors.fill: parent
                                hoverEnabled: true
                                onEntered: addFunction.color = theme.greenHover
                                onExited: addFunction.color = theme.green
                                onClicked: {
                                    menu.open()
                                }

                                Menu {
                                    id: menu
                                    y: parent.height

                                    MenuItem {
                                        text: "Lifetime"
                                        onTriggered: functionCreate(Name, text)
                                    }
                                    MenuItem {
                                        text: "StartSize"
                                        onTriggered: functionCreate(Name, text)
                                    }
                                    MenuItem {
                                        text: "StartColor"
                                        onTriggered: functionCreate(Name, text)
                                    }
                                    MenuItem {
                                        text: "StartAngle"
                                        onTriggered: functionCreate(Name, text)
                                    }
                                    MenuItem {
                                        text: "StartPosition"
                                        onTriggered: functionCreate(Name, text)
                                    }

                                    MenuItem {
                                        text: "ScaleSize"
                                        onTriggered: functionCreate(Name, text)
                                    }
                                    MenuItem {
                                        text: "ScaleColor"
                                        onTriggered: functionCreate(Name, text)
                                    }
                                    MenuItem {
                                        text: "ScaleAngle"
                                        onTriggered: functionCreate(Name, text)
                                    }
                                    MenuItem {
                                        text: "Velocity"
                                        onTriggered: functionCreate(Name, text)
                                    }

                                }
                            }
                        }
                    }
                }
            }

        }
    }

}
