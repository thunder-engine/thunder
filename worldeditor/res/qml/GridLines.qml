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

    property string subLineColor: theme.greyLight
    property string lineColor: theme.greyDark

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
        context.translate(translateX, translateY)

        context.strokeStyle = subLineColor

        // Vertical lines
        context.beginPath()
        if(drawVerical) {
            var beforeX = Math.round(-translateX / cellX)
            var afterX = Math.round((grid.width - translateX) / cellX)
            for(var x = beforeX; x < afterX; x++) {
                if(x % subItemsX != 0) {
                    context.moveTo(x * cellX, -translateY)
                    context.lineTo(x * cellX, grid.height - translateY)
                }
            }
        }

        // Horizontal lines
        if(drawHorizontal) {
            var beforeY = Math.round(-translateY / cellY)
            var afterY = Math.round((grid.height + translateY) / cellY)
            for(var y = beforeY; y < afterY; y++) {
                if(y % subItemsY != 0) {
                    context.moveTo(-translateX, y * cellY)
                    context.lineTo(grid.width - translateX, y * cellY)
                }
            }
        }
        context.closePath()
        context.stroke()

        context.strokeStyle = lineColor

        // Vertical lines
        context.beginPath()
        if(drawVerical) {
            beforeX = Math.round(-translateX / cellX / subItemsX)
            afterX = Math.round((grid.width - translateX) / cellX / subItemsX)
            for(x = beforeX; x < afterX; x++) {
                context.moveTo(x * cellX * subItemsX, -translateY)
                context.lineTo(x * cellX * subItemsX, grid.height - translateY)
            }
        }

        // Horizontal lines
        if(drawHorizontal) {
            beforeY = Math.round(-translateY / cellY / subItemsY)
            afterY = Math.round((grid.height + translateY) / cellY / subItemsY)
            for(y = beforeY; y < afterY; y++) {
                context.moveTo(-translateX, y * cellY * subItemsY)
                context.lineTo(grid.width - translateX, y * cellY * subItemsY)
            }
        }
        context.closePath()
        context.stroke()

        context.setTransform(1, 0, 0, 1, 0, 0)
    }
}
