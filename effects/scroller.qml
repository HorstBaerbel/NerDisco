Rectangle {
    property real time: 0.0
    property real valueA: 1.0
    property real valueB: 1.0
    property real valueC: 1.0
    property bool trigger: false
    color: "black"
			FontLoader { id: fixedFont; name: "Commodore 64" }
	Text {
        x: width * parent.valueA * Math.sin(parent.time/3) - width/2
        y: height * parent.valueB * Math.cos(parent.time/4)
        text: "DISCO"
        font { family: fixedFont.name;  pixelSize: 10 + 100 * parent.valueC }
		ColorAnimation on color {
			from: "red"
			to: "blue"
			duration: 5000
			loops: Animation.Infinite
		}
	}
}
