Rectangle {
    property real time: 0.0
    property real valueA: 1.0
    property real valueB: 1.0
    property real valueC: 1.0
    property bool trigger: false
    color: "black"
	Text {
        x: width * parent.valueA * Math.sin(parent.time)
        y: height * parent.valueB * Math.cos(parent.time)
        text: parent.trigger ? "Hello World" : "Foobar"
        font.pixelSize: 10 + 100 * parent.valueC
		ColorAnimation on color {
			from: "red"
			to: "blue"
			duration: 5000
			loops: Animation.Infinite
		}
	}
}
