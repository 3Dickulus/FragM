#include "Progressive2D.frag"
#info Mandelbrot
#group Mandelbrot

// Number of iterations
uniform int  Iterations; slider[1,200,2000]

uniform float Phase; slider[0,0,1]
uniform float Variance; slider[0,.05,0.25]

uniform float R; slider[0,0,1]
uniform float G; slider[0,0.4,1]
uniform float B; slider[0,0.7,1]

uniform bool Julia; checkbox[false]
uniform float JuliaX; slider[-2,-0.6,2]
uniform float JuliaY; slider[-2,1.3,2]

vec2 c2 = vec2(JuliaX,JuliaY);


vec2 complexMul(vec2 a, vec2 b) {
	return vec2( a.x*b.x -  a.y*b.y,a.x*b.y + a.y * b.x);
}


float dist =0.;

vec2 iterate(vec2 z, vec2 c) {
	vec2 J = z;
	J = complexMul(J,J) + z;
	J =  complexMul(J,J) + z;	
	J = abs( J) + c;
	dist = max(dist, dot(J,J));
	return J;
}

float wave(float x) {
	return cos( x + 2.0*  cos(8.*x));
}

vec2 mapCenter = vec2(0.5,0.5);
float mapRadius =0.4;
uniform bool ShowMap; checkbox[true]
uniform float MapZoom; slider[0.01,2.1,6]

vec2 Z1=vec2(-0.11965507329885812,-0.4289368132975893);
vec2 Z2=vec2(-0.11965507329885812,0.4289368132975893);
vec2 Z3 =vec2(-1.2606898534022837,0.0);

vec3 color(vec2 c) {
	if (Julia) {
		Z1 = c;
		c = c2;
	}	
	
	int i = 0;
	for (i = 0; i < Iterations; i++) {
		dist = 0.;
		
		Z1 = iterate(Z1,c);
		
		if (dist>100.0) break;
	}
	if (i < Iterations) {
		// The color scheme here is based on one
		// from Inigo Quilez's Shader Toy:
		float co = float( i) + 1.0 - log2(.5*log2(dist) / log2(4.0))/log2(4.0);
		co = log(co) /16. + Phase;
		vec3 RGB = vec3( .5+.5*wave(6.2832*co+R) ,
			.5+.5*wave(6.2832*co + G) ,
			.5+.5*wave(6.2832*co+B) );
		RGB = max(RGB + Variance * vec3(wave(6.2831*10.0*co)),0.0);
		return RGB;
	}  else {
		return vec3(0.0);
	}
}

#preset Default
Center = -0.285288,-0.0120426
Zoom = 0.854514
Iterations = 328
R = 0.35
G = 0.4
B = 0.52
Phase = 0.5
Variance = .05
Julia = false
JuliaX = -0.6
JuliaY = 1.3
#endpreset
