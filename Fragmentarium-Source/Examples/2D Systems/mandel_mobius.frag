#include "MathUtils.frag"
#include "Progressive2D.frag"
#group MobiusCylinder

uniform float bailout; slider[1,6,8];
uniform float width; slider[0.000001,1,10];

uniform int iters; slider[1,123,500];

uniform vec2 julia_pos; slider[(-10,-10),(-3,-2),(10,10)];
uniform bool julia_mode; checkbox[false];
uniform bool leftWrap; checkbox[true];
uniform bool rightWrap; checkbox[true];

vec2 mobius(vec2 p) {
	float dist=abs(p.x)-width;

	if (rightWrap && p.x>width) {
		p.x=-width+dist;
		p.y*=-1.0;
	};

	if (leftWrap && p.x<-width) {
		p.x=width-dist;
		p.y*=-1.0;
	};

	return p;
};

vec2 formula(in vec2 p, in vec2 c) {
	float X=p.x, Y=p.y;
	p.x=X*X-Y*Y;
	p.y=2.0*X*Y;
	p+=c;
	p=mobius(p);
	return p;
}

vec3 color(in vec2 p) {
	vec2 c = (julia_mode)?julia_pos:p;
	int k=-1;
	while (k<iters) {
		if (length(p)>bailout) break;
		p = formula(p,c);
		k+=1;
	};
	float f =float(k)/float(iters+1);
	return vec3(sin(f*3.1415926/2.0));
};

#preset default
Center = 0,0
Zoom = 0.4
Gamma = 2.2
ToneMapping = 1
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
AARange = 2
AAExp = 1
GaussianAA = true
bailout = 6
width = 1
iters = 123
julia_pos = -1.5518,0.6896
julia_mode = false
leftWrap = true
rightWrap = true
#endpreset
