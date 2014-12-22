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
width:  trigger ? 128 : 128*(1.0+Math.sin(time*6))
height: 72*(1.0+Math.cos(time*6))
color: "yellow"
opacity: 0.5*Math.sin(time*6)
}
	Rectangle {
        x: 0
        y: 0
width:  128*(1.0+Math.sin(time*6))
height: trigger ? 72 : 72*(1.0+Math.cos(time*6))
color: "magenta"
opacity: 0.5*Math.sin(3.14159+time*6)
}
	Rectangle {
        x: 0
        y: 0
width:  trigger ? 128 : 128*(1.0+Math.sin(time*6))
height: 72*(1.0+Math.cos(time*6))
color: "cyan"
opacity: 0.5*Math.sin(2*3.14159+time*6)
}
}