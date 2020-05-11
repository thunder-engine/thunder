import QtQuick 2.0

Canvas {
    id: grid
    contextType: "2d"
    antialiasing: false

    objectName: "Canvas"

    property int translateX: 0
    property int translateY: 0
    property int cellX: 16
    property int cellY: 16
    property int subItemsX: 9
    property int subItemsY: 9

    property bool drawVerical: true
    property bool drawHorizontal: true

    property string lineColor: theme.greyLight

    onTranslateXChanged: {
        requestPaint()
    }
    onTranslateYChanged: {
        requestPaint()
    }
    onCellXChanged: {
        requestPaint()
    }
    onCellYChanged: {
        requestPaint()
    }
    onDrawVericalChanged: {
        requestPaint()
    }
    onDrawHorizontalChanged: {
        requestPaint()
    }

    onPaint: {
        context.clearRect(0, 0, grid.width, grid.height)
        context.strokeStyle = lineColor
        context.translate(translateX, translateY)

        // Vertical lines
        if(drawVerical) {
            var beforeX = Math.round(-translateX / cellX)
            var afterX = Math.round((grid.width - translateX) / cellX)
            for(var x = beforeX; x < afterX; x++) {
                context.lineWidth = (x % subItemsX == 0) ? 2 : 1

                context.beginPath()
                context.moveTo(x * cellX, -translateY)
                context.lineTo(x * cellX, grid.height - translateY)
                context.stroke()
            }
        }

        // Horizontal lines
        if(drawHorizontal) {
            var beforeY = Math.round(-translateY / cellY)
            var afterY = Math.round((grid.height + translateY) / cellY)
            for(var y = beforeY; y < afterY; y++) {
                context.lineWidth = (y % subItemsY == 0) ? 2 : 1

                context.beginPath()
                context.moveTo(-translateX, y * cellY)
                context.lineTo(grid.width - translateX, y * cellY)
                context.stroke()
            }
        }

        context.setTransform(1, 0, 0, 1, 0, 0)
    }
}
