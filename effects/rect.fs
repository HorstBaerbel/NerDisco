#version 120

uniform vec2 renderSize;
uniform float time;
uniform float valueA;
uniform float valueB;
uniform float valueC;
uniform float trigger;

varying vec2 texcoordVar;

void main() {
	float l = texcoordVar.x > 0.2+0.1*sin(time) ? (texcoordVar.x < 0.8+0.2*sin(0.88*time+1) ? 1 : 0) : 0;
	l = texcoordVar.y > 0.2+0.1*sin(0.69*time) ? (texcoordVar.y < 0.8+0.2*sin(0.45*time) ? l : 0) : 0;
	float r = l * valueA + trigger;
	float g = l * valueB + trigger;
	float b = l * valueC + trigger;
	gl_FragColor = vec4(r, g, b, 1.0);
}