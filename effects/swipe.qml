Rectangle {
    property real time: 0.0
    property real valueA: 1.0
    property real valueB: 1.0
    property real valueC: 1.0
    property bool trigger: false
    color: "black"
	Rectangle {
        x: 0
        y: 0
width:  128*(1.0+Math.sin(time*5))
height: 72;//*(1.0+Math.cos(time/3))
color: "yellow"
opacity: 0.8*Math.sin(time*6)
}
	Rectangle {
        x: 0
        y: 0
width:  128*(1.0+Math.sin(time*5))
height: 72;//*(1.0+Math.cos(time/3))
color: "magenta"
opacity: 0.8*Math.sin(3.14159+time*6)
}
	Rectangle {
        x: 0
        y: 0
width:  128*(1.0+Math.sin(time*5))
height: 72;//*(1.0+Math.cos(time/3))
color: "cyan"
opacity: 0.8*Math.sin(2*3.14159+time*6)
}
}