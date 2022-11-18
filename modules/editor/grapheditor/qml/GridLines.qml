import QtQuick 2.0

ShaderEffect {
    id: grid

    property real translateX: 0
    property real translateY: 0
    property real cellX: 16
    property real cellY: 16
    property real subItemsX: 10
    property real subItemsY: 10

    property bool drawVerical: true
    property bool drawHorizontal: true

    fragmentShader: "
        uniform float cellX;
        uniform float cellY;

        uniform float translateX;
        uniform float translateY;

        uniform float subItemsX;
        uniform float subItemsY;

        void main() {
            float offsetX = gl_FragCoord.x - translateX;
            float offsetY = gl_FragCoord.y + translateY + cellY * 4.5;
            if(mod(offsetX, cellX) < 1. || mod(offsetY, cellY) < 1.) {
                if(mod(offsetX, cellX * subItemsX) < 1. || mod(offsetY, cellY * subItemsY) < 1.) {
                    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.6);
                } else {
                    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.3);
                }
            } else {
                discard;
            }
        }"
}
