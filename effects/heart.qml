Item {
    id:container
    width: 320
    height: 480
	
	property real time: 0.0
    property real valueA: 0.0
    property real valueB: 0.1
    property real valueC: 0.0
    property bool trigger: false
	
	Canvas {
		id:canvas
		width:320
		height:280
		property color strokeStyle:  Qt.darker(fillStyle, 1.4)
		property color fillStyle: "#b40000" // red
		property int lineWidth: 1.0 + 10.0 * valueA
		property bool fill: true
		property bool stroke: true
		property real alpha: 1.0
		property real scale : 0.1 + 7.0 * valueB * (1.5 + 0.2 * Math.sin(5.0*time)) + 0.1 * trigger
		property real rotate : Math.PI*2 * valueC
		antialiasing: true

		onLineWidthChanged:requestPaint();
		onFillChanged:requestPaint();
		onStrokeChanged:requestPaint();
		onScaleChanged:requestPaint();
		onRotateChanged:requestPaint();

		onPaint: {
			var ctx = canvas.getContext('2d');
			var originX = 10
			var originY = -50
			ctx.save();
			ctx.clearRect(0, 0, canvas.width, canvas.height);
			ctx.translate(originX, originX);
			ctx.globalAlpha = canvas.alpha;
			ctx.strokeStyle = canvas.strokeStyle;
			ctx.fillStyle = canvas.fillStyle;
			ctx.lineWidth = canvas.lineWidth;

			ctx.translate(originX, originY)
			ctx.scale(canvas.scale, canvas.scale);
			ctx.rotate(canvas.rotate);
			ctx.translate(-originX, -originY)

			ctx.beginPath();
			ctx.moveTo(75,40);
			ctx.bezierCurveTo(75,37,70,25,50,25);
			ctx.bezierCurveTo(20,25,20,62.5,20,62.5);
			ctx.bezierCurveTo(20,80,40,102,75,120);
			ctx.bezierCurveTo(110,102,130,80,130,62.5);
			ctx.bezierCurveTo(130,62.5,130,25,100,25);
			ctx.bezierCurveTo(85,25,75,37,75,40);
			ctx.closePath();
			if (canvas.fill)
				ctx.fill();
			if (canvas.stroke)
				ctx.stroke();
			ctx.restore();
		}
	}
}