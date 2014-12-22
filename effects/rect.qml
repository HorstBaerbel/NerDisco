Rectangle {
    property real time: 0.0
    property real valueA: 1.0
    property real valueB: 1.0
    property real valueC: 1.0
    property bool trigger: false
    color: "black"
	Rectangle {
        x: width/2 + width * parent.valueA * Math.sin(parent.time*3)
        y: height/2 + height * parent.valueB * Math.cos(parent.time)
width:60+20*Math.sin(time)
height:40
		ColorAnimation on color {
			from: "red"
			to: "blue"
			duration: 5000
			loops: Animation.Infinite
		}
	}
}
