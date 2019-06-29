import QtQuick 2.0
import QtQuick.XmlListModel 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

Item {
    id: element

    ColumnLayout {
        id: view
        anchors.fill: parent
        anchors.margins: spacing
        spacing: 30

        Rectangle {
            id: tile00
            height: 300
            color: theme.greyDark
            Layout.fillWidth: true

            property variant headerData: undefined

            Row {
                spacing: view.spacing
                anchors.fill: parent

                Image {
                    anchors.bottom: parent.bottom
                    anchors.top: parent.top
                    width: parent.width / 2
                    fillMode: Image.PreserveAspectCrop
                    clip: true
                    source: (typeof(tile00.headerData) !== "undefined") ? tile00.headerData.thumbnail : ""
                }
                Column {
                    anchors.top: parent.top
                    anchors.topMargin: 30
                    spacing: 20
                    width: parent.width / 2
                    anchors.margins: spacing
                    Text {
                        width: parent.width - parent.spacing * 2
                        font.pixelSize: 24
                        color: theme.textColor
                        text: (typeof(tile00.headerData) !== "undefined") ? tile00.headerData.title : ""
                        wrapMode: Text.WordWrap
                    }
                    Text {
                        width: parent.width - parent.spacing * 2
                        font.pixelSize: 16
                        color: theme.textColor
                        text: (typeof(tile00.headerData) !== "undefined") ? tile00.headerData.summary : ""
                        wrapMode: Text.WordWrap
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: parent.color = "#404040"
                onExited: parent.color = theme.greyDark
                onClicked: {
                    if(typeof(headerData) !== "undefined") {
                        Qt.openUrlExternally(blogData.link)
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: view.spacing

            BlogTile {
                id: tile11
                Layout.fillWidth: true
            }
            BlogTile {
                id: tile12
                Layout.fillWidth: true
            }
            BlogTile {
                id: tile13
                Layout.fillWidth: true
            }
            BlogTile {
                id: tile14
                Layout.fillWidth: true
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }

    XmlListModel {
        source: "http://thunderengine.org/feed.xml"
        query: "/feed/entry"
        namespaceDeclarations: "declare default element namespace 'http://www.w3.org/2005/Atom';declare namespace media='http://search.yahoo.com/mrss/';"

        XmlRole { name: "title"; query: "title/string()" }
        XmlRole { name: "summary"; query: "fn:replace(summary/string(), '\&lt;a href=.*\/a\&gt;', '')" }
        XmlRole { name: "thumbnail"; query: "media:thumbnail/@url/string()" }
        XmlRole { name: "link"; query: "link/@href/string()" }

        onStatusChanged: {
            if(status == XmlListModel.Ready) {
                tile00.headerData = get(0)

                tile11.blogData = get(1)
                tile12.blogData = get(2)
                tile13.blogData = get(3)
                tile14.blogData = get(4)
            }
        }
        onProgressChanged: {
            progressBar.value = progress
        }
    }

    ProgressBar {
        id: progressBar
        height: 4
        anchors.right: parent.right
        anchors.left: parent.left

        background: Rectangle {
            color: "#00000000"
        }
        contentItem: Item {
            Rectangle {
                width: progressBar.visualPosition * parent.width
                height: parent.height
                color: theme.blue
            }
        }

    }
}
