// Tslil 'Hiato' Clingman, 2012

// 03 Jan 2013 update by subblue:
// Rewritten for GPU optimisation to remove conditionals, 
// fixed syntax for the stricter AMD GPU compiler,
// improved rotation behaviour, added julia mode, 
// separated linear and nonlinear params,
// and added exponential smooth colouring.

#include "2D.frag"
#group mandelex

uniform vec2 linear; slider[(0.001,0.001),(2,2),(10,10)] 
uniform vec2 nonlinear; slider[(0.001,0.001),(2,2),(10,10)] 
uniform float wrap; slider[0.1,0.5,10]
uniform float radius; slider[0.1,0.5,10];
uniform float scale; slider[1,2,10];
uniform float angle; slider[0,180,360];
uniform float intensity; slider[0,1,10];
uniform int iters; slider[1,13,500];
uniform float bailout; slider[1,13,1000];
uniform int julia_mode; slider[0,0,1];
uniform vec2 seed; slider[(-2,-2),(0,0),(2,2)]

const float deg2rad = 3.1415926 / 180.0;
mat2 rotation;
float r2;

void init() {
	float rc = cos(angle * deg2rad);
  	float rs = sin(angle * deg2rad);
  	rotation = mat2(rc, rs, -rs, rc);
	r2 = radius * radius;
}


vec2 formula(in vec2 a, in vec2 p) {

	// Linear pull
	// when abs(a.x) > linear.y then step will == 1 else 0
	vec2 lp = step(linear.xy, abs(a));
	a -= sign(a) * 2.0 * linear.xy * lp.x * lp.y;

	// non linear pull
	// when abs(a.x) < linear.x then step will == 1 else 0
	vec2 nlp = step(abs(p), nonlinear);
	lp = step(nonlinear, abs(a));
	vec2 l = 2.0 * nonlinear;
	vec2 f = floor(a / l);
	vec2 c = ceil(a / l);
	a -= l * mix(f, c, step(a, vec2(0.0))) * lp.x * lp.y * nlp.x * nlp.y;
	
	// wrap box
	a = clamp(a, -vec2(wrap), vec2(wrap)) * 2.0 - a;
	
	// circle invert
	float l2 = dot(a, a);
	// multiply by r2 / l2 if l2 < r2 otherwise multiply by 1
	a *= mix(1.0, r2/l2, step(l2, r2));

	a *= scale;
	a *= rotation;
	a += p;
	
	// Linear pull
	// when abs(a.x) > linear.x then step will == 1 else 0
	lp = step(linear.xy, abs(a));
	a -= sign(a) * 2.0 * linear.xy * lp.x * lp.y;

  	return a;
}

vec3 color(in vec2 p) {
	init();
	vec2 c = mix(p, seed, float(julia_mode));
	int k=0;
	float d1 = 0.0, d0, v = 0.0;

	while (k<iters) {
		d0 = d1;
		d1 = dot(p, p);
		v += exp(-1.0 / abs(d1 - d0));
		if (d1 > bailout) break;
		p = formula(p, c);
		k+=1;
	}

	v = pow(v / float(k), intensity);
	return vec3(max(1.0 - v, 0.0));
}


#preset default
Center = 0,0
Zoom = 0.12359
AntiAliasScale = 1
AntiAlias = 3
wrap = 0.5
radius = 0.5
scale = 2
angle = 180
iters = 25
bailout = 500
linear = 2,2
nonlinear = 1.75003,1.75003
intensity = 2.3118
julia_mode = 0
seed = 0,0
#endpreset

#preset Flower
Center = -0.0117119,-0.00777377
Zoom = 1.00566
AntiAliasScale = 1
AntiAlias = 3
wrap = 0.28949
radius = 0.397
scale = 1.95193
angle = 180
iters = 22
bailout = 500
linear = 1.21623,1.21623
nonlinear = 1.75003,1.75003
intensity = 2.3118
julia_mode = 0
seed = 0,0
#endpreset

#preset Swirl
Center = -0.0117119,-0.00777377
Zoom = 0.574987
AntiAliasScale = 1
AntiAlias = 3
wrap = 0.28949
radius = 0.8425
scale = 1.38943
angle = 204.671
iters = 15
bailout = 500
linear = 3.92221,5.39266
nonlinear = 0.001,0.001
intensity = 5.0538
julia_mode = 0
seed = -1.16588,-1.07108
#endpreset

#preset Leaves
Center = -0.0117119,-0.00777377
Zoom = 2.67508
AntiAliasScale = 1
AntiAlias = 3
wrap = 0.28949
radius = 0.397
scale = 1.95193
angle = 105.991
iters = 9
bailout = 500
linear = 1.21623,1.21623
nonlinear = 0.001,0.001
intensity = 0.8065
julia_mode = 0
seed = 0.02844,0
#endpreset