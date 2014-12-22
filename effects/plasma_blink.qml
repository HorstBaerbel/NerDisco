Item {
	id:container
		anchors.fill: parent

    property real time: 0.0
    property real valueA: 0.0
    property real valueB: 0.0
    property real valueC: 1.0
    property bool trigger: false

	Canvas {
		id:canvas
		anchors.fill: parent
		//renderTarget:Canvas.FramebufferObject
		renderTarget:Canvas.Image
		renderStrategy: Canvas.Threaded
		onPaint: {
			var ctx = getContext('2d');
			var imageData;
			//Try to create image data from scratch
			//If that doesn't work, try to load it from the context
			if (ctx.getImageData) {
				imageData = ctx.getImageData(0, 0, width, height);
			}
			//If that fails too, create an array of the same size and pray
			else {
				imageData = {'width' : width, 'height' : height, 'data' : new Array(width*height*4)};
			}
			//get actual pixel data
			var pixels = imageData.data;
			//pixels.mirror();
			for (var y = 0; y < height; ++y) {
				for (var x = 0; x < width; ++x) {
					var index = (y * width + x) * 4;
					var v = 0;
					var xf=x/width;
					var yf=y/height;
					v += Math.sin((xf*10.0*parent.valueB+time));
					v += Math.sin((yf*10.0+time)/2.0);
					v += Math.sin((xf*10.0+yf*10*parent.valueA+time)/2.0);
					var cx = xf + .5 * Math.sin(time*0.3) * 5.0 * parent.valueC;
					var cy = yf + .5 * Math.cos(time*0.4) * 5.0 * parent.valueC;
					v += Math.sin(Math.sqrt(100*(cx*cx+cy*cy)+1)+time);
					v = v/2.0;
					var brightness = 100 + Math.sin(parent.time*5) * 155;
					pixels[index] = brightness*(.5+.5*Math.sin(Math.PI*v));
					pixels[index+1] = brightness*(.5+.5*Math.sin(Math.PI*v+2*Math.PI/3));
					pixels[index+2] = brightness*(.5+.5*Math.sin(Math.PI*v+4*Math.PI/3));
					pixels[index + 3]=255;
				}
			}
			//pixels.filter(ctx.GrayScale);
			//pixels.filter(ctx.Threshold, 100); //default 127
			//pixels.filter(ctx.Blur, 20); //default 10
			//pixels.filter(ctx.Opaque);
			//pixels.filter(ctx.Invert);
			//pixels.filter(ctx.Convolute, [0,-1,0,
			//                                 -1,5,-1,
			//                                 0,-1,0]);
			//ctx.putImageData(imageData, 0, 0, canvas.width, canvas.height);
			ctx.putImageData(imageData, 0, 0);
		}
	}

	Timer {
		interval: 50
		running: true
		triggeredOnStart: true
		repeat: true
		onTriggered: canvas.requestPaint()
	}
}
