uniform vec2 renderSize;

varying vec2 texcoordVar;

void main() {
	gl_FragColor = vec4(texcoordVar, 0.0, 1.0);
}