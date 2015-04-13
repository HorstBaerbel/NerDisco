#version 120

uniform vec2 renderSize;
uniform float time;
uniform float valueA;
uniform float valueB;
uniform float valueC;
uniform float trigger;

varying vec2 texcoordVar;

void main() {
float brightness = 0.5;//sin(10*valueC*time);
float v = brightness*(sin(100*valueA*texcoordVar.x+10*valueB*texcoordVar.y+valueC*time));
float r = sin(time);
float g = sin(1.5*time-1.4);
float b = cos(time+0.87);
	gl_FragColor = vec4(v*r, v*g, v*b, 1.0);
}