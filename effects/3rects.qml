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
color: "green"
}
	Rectangle {
        x: width/2 + width * parent.valueA * Math.sin(20+parent.time*3)
        y: height/2 + height * parent.valueB * Math.cos(100-parent.time/2)
width:60+20*Math.sin(time)
height:40
color: "blue"
}
	Rectangle {
        x: width/2 + width * parent.valueA * Math.sin(80-parent.time*2)
        y: height/2 + height * parent.valueB * Math.cos(100-parent.time/3)
width:60+20*Math.sin(time)
height:40
color: "red"
	}
}