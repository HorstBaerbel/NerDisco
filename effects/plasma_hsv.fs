uniform vec2 renderSize;
uniform float time;
uniform float valueA;
uniform float valueB;
uniform float valueC;
uniform float trigger;

varying vec2 texcoordVar;

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
					const float PI = 3.1415926;
      float v = 0.0;
					v += sin((texcoordVar.x*10.0*valueB+time));
					v += sin((texcoordVar.y*10.0+time)/2.0);
					v += sin((texcoordVar.x*10.0+texcoordVar.y*10.0*valueA+time)/2.0);
					float cx = texcoordVar.x + .5 * sin(time*0.3) * 8.0 * valueC;
					float cy = texcoordVar.y + .5 * cos(time*0.4) * 8.0 * valueC;
					v += 0.5*sin(sqrt((cx*cx+cy*cy)+1.0)+time);
					float brightness = 0.01;//1.0 +  sin(time*5) * 0.5;
					vec3 hsv;
					hsv.x = 0.5 + 0.5*sin(PI*cx);
					hsv.y = 0.5 + 0.5*sin(PI*cy+ 2.0*PI/3.0);
					hsv.z = 0.2*v;
					hsv = hsv2rgb(hsv);
					gl_FragColor = vec4(hsv, 1.0);
}