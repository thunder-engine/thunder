import QtQuick 2.0
import QtQuick.Controls 2.4

Menu {
    id: menu

    topPadding: 0
    bottomPadding: 0

    delegate: MenuItem {
        id: menuItem
        implicitWidth: 200
        implicitHeight: 40

        arrow: Canvas {
            x: parent.width - width
            implicitWidth: 40
            implicitHeight: 40
            visible: menuItem.subMenu
            onPaint: {
                var ctx = getContext("2d")
                ctx.fillStyle = theme.textColor
                ctx.moveTo(15, 15)
                ctx.lineTo(width - 15, height / 2)
                ctx.lineTo(15, height - 15)
                ctx.closePath()
                ctx.fill()
            }
        }

        indicator: Item {
            implicitWidth: 40
            implicitHeight: 40
            Rectangle {
                width: 26
                height: 26
                anchors.centerIn: parent
                visible: menuItem.checkable
                border.color: "#21be2b"
                radius: 3
                Rectangle {
                    width: 14
                    height: 14
                    anchors.centerIn: parent
                    visible: menuItem.checked
                    color: "#21be2b"
                    radius: 2
                }
            }
        }

        contentItem: Text {
            text: menuItem.text
            font: menuItem.font
            opacity: enabled ? 1.0 : 0.3
            color: theme.textColor
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        background: Rectangle {
            implicitWidth: 200
            implicitHeight: 40
            opacity: enabled ? 1 : 0.3
            color: menuItem.highlighted ? "#21be2b" : "transparent"
        }
    }

    background: Rectangle {
        implicitWidth: 200
        implicitHeight: 40
        color: theme.backColor
    }
}
