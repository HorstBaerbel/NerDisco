uniform vec2 renderSize;
uniform float time;
uniform float valueA;
uniform float valueB;
uniform float valueC;
uniform float triggerA;

varying vec2 texcoordVar;

float circle(vec2 p, float size) {
    return length(p)*length(p)-size;
}

float torus(vec2 p, float size) {
 vec2 q = vec2(length(p)-1.6*size,1.);
float l = length(q);
return pow(l*l*l*l-size,3.0);
}

float cross(vec2 p, float size) {
    float k = length(p)-size;
    return 1.-abs(k*k);
}

float opRep( vec2 p, vec2 c, float size, float pattern ) {
    vec2 q = mod(p,c)-0.5*c;
    return (pattern < 0.33) ? cross( q, size ) : ((pattern < 0.66) ? circle( q, size ) : torus( q, size ));
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
const float PI = 3.1415926;
float pattern = 0.01*mod(time, 100.0);
float rad = PI*sin(valueC*time);
vec2 shift = vec2(sin(1.3*time), sin(0.8*time+2.0));
float cx = texcoordVar.x - 0.5;
float cy = texcoordVar.y - 0.5;
float px = cx*cos(rad)+cy*sin(rad);
float py = -cx*sin(rad)+cy*cos(rad);
float multiplier = 6.0+5.0*sin(time)*valueB;
float v = 1.-opRep(multiplier*vec2(px, py)+shift, vec2(3,3), 0.5, pattern);
					float brightness = triggerA < 1.0 ? 1.5 : 3.0;
					vec3 hsv;
					hsv.x = 0.5 + 0.5*sin(PI*time);
					hsv.y = 0.5 + 0.5*sin(PI*time+ 2.0*PI/3.0);
					hsv.z = brightness * 0.2*v;
					hsv = hsv2rgb(hsv);
	gl_FragColor = vec4(hsv, 1.0);
}