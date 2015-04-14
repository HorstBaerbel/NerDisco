#version 120

uniform vec2 renderSize;
uniform float time;
uniform float valueA;
uniform float valueB;
uniform float valueC;
uniform float triggerA;

varying vec2 texcoordVar;

void main() {
const float PI = 3.1415926;
      float v = 0.0;
					v += sin((texcoordVar.x*10.0*valueB+time));
					v += sin((texcoordVar.y*10.0+time)/2.0);
					v += sin((texcoordVar.x*10.0+texcoordVar.y*10*valueA+time)/2.0);
					float cx = texcoordVar.x + .5 * sin(time*0.3) * 5.0 * valueC;
					float cy = texcoordVar.y + .5 * cos(time*0.4) * 5.0 * valueC;
					v += 0.5*sin(sqrt((cx*cx+cy*cy)+1)+time);
					float brightness = 0.05;//1.0 +  sin(time*5) * 0.5;
 float r = brightness * 0.5 + 0.5*sin(PI*v);
 float g = brightness * 0.5 + 0.5*sin(PI*v+ 2.0*PI/3.0);
 float b = brightness * 0.5 + 0.5*sin(PI*v+ 4.0*PI/3.0);
	gl_FragColor = vec4(r, g, b, 1.0);
}