import QtQuick 2.0
import QtQuick.Controls 2.4

Item {
    property string textColor: "#ffffff"

    property string backColor: "#606060"

    property string emitterColor: "#40000000"
    property string functionColor: "#40000000"
    property string hoverColor: "#60000000"

    property string blueColor: "#0277bd"
    property string hoverBlueColor: "#0288d1"

    property string greenColor: "#2e7d32"
    property string hoverGreenColor: "#388e3c"

    property string redColor: "#c62828"
    property string hoverRedColor: "#d32f2f"

    property var colors: [Qt.rgba(1,0,0), Qt.rgba(0,1,0), Qt.rgba(0,0,1), Qt.rgba(1,1,0), Qt.rgba(1,0,1), Qt.rgba(0,1,1)]
}
