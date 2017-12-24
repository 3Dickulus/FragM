#version 400 compatibility
#info 3Dickulus Mandelbulb Distance Estimator + Menger Sphere (knighty)
#define providesInit
#include "MathUtils.frag"
#include "DE-Kn2.frag"

#group Coloring
// Roquen's domain mash up coordinate to invert to infinity
uniform vec3 InvertC; slider[(-5,-5,-5),(0,0,0),(5,5,5)] Locked;
// http://www.fractalforums.com/index.php?topic=16963.msg65053#msg65053
// performs the active c = T(s)
vec3 domainMap(vec3 c)
{
  float s = dot(c,c);
  return c/s + InvertC;
}

#group Mandelbulb

// Number of fractal iterations.
uniform int Iterations;  slider[0,9,100]

// Number of color iterations.
uniform int ColorIterations;  slider[0,9,100]

// Mandelbulb exponent (8 is standard)
uniform float Power; slider[0,8,16]

// Bailout radius
uniform float Bailout; slider[0,5,30]

// Alternate is slightly different, but looks more like a Mandelbrot for Power=2
uniform bool AlternateVersion; checkbox[false]

uniform vec3 RotVector; slider[(0,0,0),(1,1,1),(1,1,1)]

uniform float RotAngle; slider[0.00,0,360]

uniform bool Julia; checkbox[false]
uniform vec3 JuliaC; slider[(-2,-2,-2),(0,0,0),(2,2,2)]

#group MengerSphere
// Number of iterations.
uniform int msIterations;  slider[0,10,20]

// Number of color iterations.
uniform int msColorIterations;  slider[0,9,100]

// Scale parameter. A perfect Menger is 3.0
uniform float msScale; slider[0.00,3,4.00]

uniform vec3 msRotVector; slider[(0,0,0),(1,0,0),(1,1,1)]

// Scale parameter. A perfect Menger is 3.0
uniform float msRotAngle; slider[0.00,00,360]

// Scaling center
uniform vec3 msOffset; slider[(-2,-2,-2),(1,1,1),(2,2,2)]

#group MandelBox
// Number of fractal iterations.
uniform int mbIterations;  slider[0,17,300]
uniform int mbColorIterations;  slider[0,3,300]

uniform float mbMinRad2;  slider[0,0.25,2.0]

// Scale parameter. A perfect Menger is 3.0
uniform float mbScale;  slider[-3.0,3.0,5.0]
vec4 mbscale = vec4(mbScale, mbScale, mbScale, abs(mbScale)) / mbMinRad2;

// precomputed constants

uniform vec3 mbRotVector; slider[(0,0,0),(1,1,1),(1,1,1)]

// Scale parameter. A perfect Menger is 3.0
uniform float mbRotAngle; slider[0.00,0,360]


mat3 rot;
mat3 msrot;
mat3 mbrot;

uniform float time;
float sc,sr;
void init() {
	 rot = rotationMatrix3(normalize(RotVector), RotAngle);
	 msrot = rotationMatrix3(normalize(msRotVector),msRotAngle);
	 mbrot = rotationMatrix3(normalize(mbRotVector), mbRotAngle);
	vec3 o=abs(msOffset);
	sc = max(o.x,max(o.y,o.z));
	sr=sqrt(dot(o,o)+1.);
}

// This is my power function, based on the standard spherical coordinates as defined here:
// http://en.wikipedia.org/wiki/Spherical_coordinate_system
//
// It seems to be similar to the one Quilez uses:
// http://www.iquilezles.org/www/articles/mandelbulb/mandelbulb.htm
//
// Notice the north and south poles are different here.
void powN1(inout vec3 z, float r, inout float dr) {
	// extract polar coordinates
	float theta = acos(z.z/r);
	float phi = atan(z.y,z.x);
	dr =  pow( r, Power-1.0)*Power*dr + 1.0;

	// scale and rotate the point
	float zr = pow( r,Power);
	theta = theta*Power;
	phi = phi*Power;

	// convert back to cartesian coordinates
	z = zr*vec3(sin(theta)*cos(phi), sin(phi)*sin(theta), cos(theta));
}

// This is a power function taken from the implementation by Enforcer:
// http://www.fractalforums.com/mandelbulb-implementation/realtime-renderingoptimisations/
//
// I cannot follow its derivation from spherical coordinates,
// but it does give a nice mandelbrot like object for Power=2
void powN2(inout vec3 z, float zr0, inout float dr) {
	float zo0 = asin( z.z/zr0 );
	float zi0 = atan( z.y,z.x );
	float zr = pow( zr0, Power-1.0 );
	float zo = zo0 * Power;
	float zi = zi0 * Power;
	dr = zr*dr*Power + 1.0;
	zr *= zr0;
	z  = zr*vec3( cos(zo)*cos(zi), cos(zo)*sin(zi), sin(zo) );
}

// Compute the distance from `pos` to the Mandelbox.
float mandelDE(vec3 pos) {
	vec3 z=pos;
	float r;
	float dr=1.0;
	int i=0;
	r=length(z);orbitTrap=vec4(10000.0);
	while(r<Bailout && (i<Iterations)) {
		if (AlternateVersion) {
			powN2(z,r,dr);
		} else {
			powN1(z,r,dr);
		}
		z+=(Julia ? JuliaC : pos);
		r=length(z);
		z*=rot;
		if (i<ColorIterations) orbitTrap = min(orbitTrap, abs(vec4(domainMap(-z),r*r)));
		i++;
	}
//	if ((type==1) && r<Bailout) return 0.0;
	return 0.5*log(r)*r/dr;
	/*
	Use this code for some nice intersections (Power=2)
	float a =  max(0.5*log(r)*r/dr, abs(pos.y));
	float b = 1000;
	if (pos.y>0)  b = 0.5*log(r)*r/dr;
	return min(min(a, b),
		max(0.5*log(r)*r/dr, abs(pos.z)));
	*/
}



// knighty's Menger-Sphere
float mengersDE(vec3 p)
{
#if 1
	vec3 ap=abs(p);
	float Linf=max(max(ap.x,ap.y),ap.z);//infinity norm
	float L2=length(p);//euclidean norm
	float multiplier=L2/Linf;
	p*=multiplier;//Spherify transform.
	float dd=multiplier*1.6;//to correct the DE. Found by try and error. there should be better formula.
#else
	float dd=1.;
#endif
orbitTrap=vec4(10000.0);
	float r2=dot(p,p);
	for(int i = 0; i<msIterations && r2<100.; i++){
		p=abs(p);
		if(p.y>p.x) p.xy=p.yx;
      		if(p.z>p.y) p.yz=p.zy;
      		if(p.y>p.x) p.xy=p.yx;
		p.z=abs(p.z-1./3.*msOffset.z)+1./3.*msOffset.z;
		p=p*msScale-msOffset*(msScale-1.); dd*=msScale;
		p=msrot*p;
		r2=dot(p,p);
//		if (i<msColorIterations) orbitTrap = min(orbitTrap, abs(vec4(p.x,p.y,p.z,r2*r2)));
if (i<msColorIterations) orbitTrap = min(orbitTrap, abs(vec4(domainMap(-p),r2)));
	}
#if 1
	return (sqrt(r2)-sr)/dd;//bounding volume is a sphere
#else
	p=abs(p); return (max(p.x,max(p.y,p.z))-sc)/dd;//bounding volume is a cube
#endif
}
/*
float mengerDE(vec3 p)
{
	p=p*0.5+vec3(0.5);
	vec3 pp= abs(p-0.5)-0.5;
	float k=1.0;
	float d1 = max(pp.x,max(pp.y,pp.z));
	float d=d1;
	for (int i = 0; i < mIterations ; i++)
	{
		vec3 pa = mod(3.0*p*k, 3.0);

		pp = 0.5-abs(pa-1.5)+mOffset;
             pp*=rot;
		d1= (1/k) * min(max(pp.x,pp.z),min(max(pp.x,pp.y),max(pp.y,pp.z)));//distance inside the 3 axis aligned square tubes

		d=max(d,d1); k *= mScale;
		if (i<mColorIterations) orbitTrap = min(orbitTrap, abs(vec4(domainMap(-p),length(p))));

	}

	// Use this to crop to a sphere:
	//  float e = clamp(length(z)-2.0, 0.0,100.0);
	//  return max(d,e);// distance estimate
	return d;
}
*/
/*
The distance estimator below was originalled devised by Buddhi.
This optimized version was created by Rrrola (Jan Kadlec), http://rrrola.wz.cz/

See this thread for more info: http://www.fractalforums.com/3d-fractal-generation/a-mandelbox-distance-estimate-formula/15/
*/

float absScalem1 = abs(mbScale - 1.0);
float AbsScaleRaisedTo1mIters = pow(abs(mbScale), float(1-mbIterations));

// Compute the distance from `pos` to the Mandelbox.
float mandelboxDE(vec3 pos) {
	vec4 p = vec4(pos,1.), p0 = p;  // p.w is the distance estimate

	for (int i=0; i<mbIterations; i++) {
		p.xyz*=mbrot;
		p.xyz = clamp(p.xyz, -1.0, 1.0) * 2.0 - p.xyz;  // min;max;mad
		float r2 = dot(p.xyz, p.xyz);
		if (i<mbColorIterations) orbitTrap = min(orbitTrap, abs(vec4(domainMap(-p.xyz),r2)));
		p *= clamp(max(mbMinRad2/r2, mbMinRad2), 0.0, 1.0);  // dp3,div,max.sat,mul
		p = p*mbscale + p0;
             if ( r2>1000.0) break;
	}
	return ((length(p.xyz) - absScalem1) / p.w - AbsScaleRaisedTo1mIters);
}

float DE(vec3 pos) {
  float rd = 0.0; // this can be + or - or another function all together
/*
  if(Iterations > 0) rd = mandelDE(abs(pos));
  if(msIterations > 0) rd += mengersDE(abs(pos));
  if(mbIterations > 0) rd += mandelboxDE(abs(pos));
*/
  if(Iterations > 0) rd = max(rd,mandelDE(abs(pos)));
  if(msIterations > 0) rd = max(rd,mengersDE(abs(pos)));
  if(mbIterations > 0) rd = max(rd,mandelboxDE(abs(pos)));

 return rd;
};


#preset Default
FOV = 0.62536
Eye = 1.65826,-1.22975,0.277736
Target = -5.2432,4.25801,-0.607125
EquiRectangular = false
FocalPlane = 1
Aperture = 0
Gamma = 2.08335
ToneMapping = 3
Exposure = 0.6522
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
Detail = -2.84956
DetailAO = -1.35716
FudgeFactor = 1
MaxRaySteps = 164
Dither = 0.51754
NormalBackStep = 1
AO = 0,0,0,0.85185
Specular = 1.6456
SpecularExp = 16.364
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = 0.63626,0.5
CamLight = 1,1,1,1.53846
CamLightMin = 0.12121
Glow = 1,1,1,0.43836
GlowMax = 52
Fog = 0
HardShadow = 0.35385
ShadowSoft = 12.5806
Reflection = 0
DebugSun = false
BaseColor = 1,1,1
OrbitStrength = 0.14286
X = 1,1,1,1
Y = 0.345098,0.666667,0,0.02912
Z = 1,0.666667,0,1
R = 0.0784314,1,0.941176,-0.0194
BackgroundColor = 0.607843,0.866667,0.560784
GradientBackground = 0.3261
CycleColors = false
Cycles = 4.04901
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 12
ColorIterations = 8
Power = 8
Bailout = 6.279
AlternateVersion = true
RotVector = 1,1,1
RotAngle = 0
Julia = false
JuliaC = 0,0,0
msIterations = 10
msScale = 3
msRotVector = 1,0,0
msRotAngle = 0
msOffset = 1,1,1
mbIterations = 0
Up = 0,1,0
#endpreset

#preset MountainTop
FOV = 0.45112
Eye = -0.788211,-0.407889,-0.610453
Target = 7.00624,3.66612,-1.69566
EquiRectangular = false
FocalPlane = 1
Aperture = 0
Gamma = 2.08335
ToneMapping = 3
Exposure = 0.6522
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
Detail = -3.83705
DetailAO = -1.35716
FudgeFactor = 0.01905
MaxRaySteps = 2743
Dither = 1
NormalBackStep = 1
AO = 0,0,0,0.85185
SpecularExp = 16.364
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = 0,0.54216
CamLight = 1,1,1,1.53846
CamLightMin = 0.12121
Glow = 1,1,1,0.43836
GlowMax = 52
Fog = 0
HardShadow = 0.35385
ShadowSoft = 12.5806
Reflection = 0
BaseColor = 1,1,1
OrbitStrength = 0.6125
X = 0.5,0.6,0.6,0.31578
Y = 1,0.6,0,0.35652
Z = 0.8,0.78,1,0.38596
R = 0.4,0.7,1,1
BackgroundColor = 0.6,0.6,0.45
GradientBackground = 0.3
CycleColors = true
Cycles = 2.9497
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 12
ColorIterations = 8
Power = 8
Bailout = 6.279
AlternateVersion = true
RotVector = 1,1,1
RotAngle = 0
Julia = false
JuliaC = 0,0,0
msIterations = 10
msScale = 3
msRotVector = 1,0,0
msRotAngle = 0
msOffset = 1,1.02804,1
mbIterations = 0
InvertC = -1.0714,-1.0714,0.8929
Specular = 4
Up = -0.00697481,0.02942,-1
#endpreset

#preset allup
FOV = 0.62536
Eye = 1.89012,0.406295,0.471234
Target = -6.42928,-1.63015,-1.80234
EquiRectangular = false
FocalPlane = 1
Aperture = 0
Gamma = 2.08335
ToneMapping = 3
Exposure = 0.6522
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
Detail = -3.21482
DetailAO = -1.35716
FudgeFactor = 0.55238
MaxRaySteps = 164
Dither = 0.51754
NormalBackStep = 1
AO = 0,0,0,0.85185
Specular = 1.6456
SpecularExp = 16.364
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = 0.63626,0.5
CamLight = 1,1,1,1.53846
CamLightMin = 0.12121
Glow = 1,1,1,0.43836
GlowMax = 52
Fog = 0
HardShadow = 0.35385
ShadowSoft = 12.5806
Reflection = 0
DebugSun = false
BaseColor = 1,1,1
OrbitStrength = 0.44792
X = 1,1,1,-1
Y = 0.345098,0.666667,0,0.02912
Z = 1,0.666667,0,1
R = 0.0784314,1,0.941176,0
BackgroundColor = 0.607843,0.866667,0.560784
GradientBackground = 1.2931
CycleColors = false
Cycles = 4.04901
EnableFloor = true
FloorNormal = 0,0.0002,0
FloorHeight = -2.9612
FloorColor = 1,1,1
Iterations = 6
ColorIterations = 8
Power = 8
Bailout = 6.279
AlternateVersion = false
RotVector = 1,0,0
RotAngle = 0
Julia = true
JuliaC = 1e-05,1e-05,1e-05
InvertC = 0,0,0
msIterations = 6
msColorIterations = 6
msScale = 3
msRotVector = 1,0,0
msRotAngle = 0
msOffset = 1,1,1
mbIterations = 8
mbColorIterations = 6
mbMinRad2 = 0.33962
mbScale = 2.93336
mbRotVector = 1,0,0
mbRotAngle = 90
Up = -0.0775633,0.996987,-0.000275951
#endpreset


#preset MengerJuliaBulb
FOV = 0.42954
Eye = 1.93214,0.513854,0.5545
Target = -6.33374,-1.74854,-1.70059
EquiRectangular = false
FocalPlane = 1
Aperture = 0
Gamma = 2.08335
ToneMapping = 3
Exposure = 0.6522
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
Detail = -6.63705
DetailAO = -1.35716
FudgeFactor = 0.07619
MaxRaySteps = 2743
Dither = 1
NormalBackStep = 1
AO = 0,0,0,0.85185
Specular = 4
SpecularExp = 16.364
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = 0.2771,-0.61446
CamLight = 1,1,1,1.53846
CamLightMin = 0.12121
Glow = 1,1,1,0.43836
GlowMax = 52
Fog = 0
HardShadow = 0.35385
ShadowSoft = 12.5806
Reflection = 0
DebugSun = false
BaseColor = 1,1,1
OrbitStrength = 1
X = 0.5,0.6,0.6,0.31578
Y = 1,0.6,0,0.35652
Z = 0.8,0.78,1,0.38596
R = 0.4,0.7,1,1
BackgroundColor = 0.6,0.6,0.45
GradientBackground = 2.06895
CycleColors = true
Cycles = 2.59614
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 6
ColorIterations = 8
Power = 8
Bailout = 1.5
AlternateVersion = true
RotVector = 1,1,1
RotAngle = 0
Julia = true
JuliaC = -1.00828,0,0
InvertC = 0,0,0
msIterations = 6
msColorIterations = 6
msScale = 2.57144
msRotVector = 1,0,0
msRotAngle = 0
msOffset = 1,1.00008,1.28204
mbIterations = 0
mbColorIterations = 6
mbMinRad2 = 0.3585
mbScale = 2.6
mbRotVector = 1,0,0
mbRotAngle = 0
Up = 0.23765,0.0811309,0.967957
#endpreset


#preset NativeArtifact
FOV = 0.42954
Eye = 1.86148,-0.000394087,1.48799
Target = -6.99979,-0.0793728,1.47353
EquiRectangular = false
FocalPlane = 1
Aperture = 0
Gamma = 2.08335
ToneMapping = 3
Exposure = 0.6522
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
Detail = -6.06669
DetailAO = -7.1429
FudgeFactor = 1
MaxRaySteps = 973
Dither = 1
NormalBackStep = 1
AO = 0,0,0,0.85185
SpecularExp = 16.364
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = -0.71084,0.01204
CamLight = 1,1,1,1.30556
CamLightMin = 0
Glow = 1,1,1,1
GlowMax = 74
Fog = 0
HardShadow = 0.35385
ShadowSoft = 12.5806
Reflection = 0
BaseColor = 1,1,1
OrbitStrength = 1
X = 0.5,0.6,0.6,-0.31578
Y = 1,0.6,0,0.35652
Z = 0.8,0.78,1,0.61404
R = 0.4,0.7,1,0.4386
BackgroundColor = 0.6,0.6,0.45
GradientBackground = 2.06895
CycleColors = true
Cycles = 3.23467
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 0
ColorIterations = 8
Power = 8
Bailout = 1.5
AlternateVersion = true
RotVector = 1,1,1
RotAngle = 0
Julia = false
JuliaC = -1.00828,0,0
InvertC = 0,0,0
msIterations = 0
msColorIterations = 6
msScale = 2.57144
msRotVector = 1,0,0
msRotAngle = 0
msOffset = 1,1.00008,1.28204
mbIterations = 12
mbColorIterations = 6
mbMinRad2 = 0.26416
mbScale = -2.06664
mbRotVector = 1,0,0
mbRotAngle = 0
Up = -0.00162308,-0.000165406,0.995117
BoundingSphere = 10.606
Specular = 4
#endpreset

#preset Corner
FOV = 0.42954
Eye = 1.17079,-1.56053,-1.56537
Target = 2.04729,4.63624,4.70841
EquiRectangular = false
FocalPlane = 1
Aperture = 0
Gamma = 2.08335
ToneMapping = 3
Exposure = 0.6522
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
Detail = -6.94813
DetailAO = 0
FudgeFactor = 1
MaxRaySteps = 2566
Dither = 1
NormalBackStep = 1
AO = 0,0,0,0.85185
SpecularExp = 16.364
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = -0.15662,-0.73494
CamLight = 1,1,1,2
CamLightMin = 0
Glow = 1,1,1,1
GlowMax = 74
Fog = 0
HardShadow = 0.35385
ShadowSoft = 12.5806
Reflection = 0
BaseColor = 1,1,1
OrbitStrength = 1
X = 0.5,0.6,0.6,-0.38596
Y = 1,0.6,0,1
Z = 0.8,0.78,1,-0.19298
R = 0.4,0.7,1,1
BackgroundColor = 0.6,0.6,0.45
GradientBackground = 2.06895
CycleColors = true
Cycles = 1.80982
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 0
ColorIterations = 8
Power = 8
Bailout = 1.5
AlternateVersion = true
RotVector = 1,1,1
RotAngle = 0
Julia = false
JuliaC = -1.00828,0,0
InvertC = 0.0893,0.8036,0
msIterations = 0
msColorIterations = 6
msScale = 2.57144
msRotVector = 1,0,0
msRotAngle = 0
msOffset = 1,1.00008,1.28204
mbIterations = 16
mbColorIterations = 6
mbMinRad2 = 0.32076
mbScale = -2.15176
mbRotVector = 1,0,0
mbRotAngle = 0
BoundingSphere = 10.606
Specular = 4
Up = 1,0,0
#endpreset


#preset Staged
FOV = 0.67114
Eye = -1.89098,1.27889,0.000408219
Target = 6.79053,-0.495082,0.114506
EquiRectangular = false
FocalPlane = 1
Aperture = 0
Gamma = 2.08335
ToneMapping = 3
Exposure = 0.6522
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
Detail = -6.63705
DetailAO = 0
FudgeFactor = 1
MaxRaySteps = 2478
Dither = 1
NormalBackStep = 1
AO = 0,0,0,0.85185
SpecularExp = 16.364
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = -0.15662,-0.73494
CamLight = 1,1,1,2
CamLightMin = 0
Glow = 1,1,1,1
GlowMax = 74
Fog = 0
HardShadow = 0.35385
ShadowSoft = 12.5806
Reflection = 0
BaseColor = 1,1,1
OrbitStrength = 1
X = 0.5,0.6,0.6,-0.38596
Y = 1,0.6,0,1
Z = 0.8,0.78,1,-0.19298
R = 0.4,0.7,1,1
BackgroundColor = 0.6,0.6,0.45
GradientBackground = 2.06895
CycleColors = true
Cycles = 1.80982
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 0
ColorIterations = 8
Power = 8
Bailout = 1.5
AlternateVersion = true
RotVector = 1,1,1
RotAngle = 0
Julia = false
JuliaC = -1.00828,0,0
InvertC = 0.0893,0.8036,0
msIterations = 0
msColorIterations = 6
msScale = 2.57144
msRotVector = 1,0,0
msRotAngle = 0
msOffset = 1,1.00008,1.28204
mbIterations = 16
mbColorIterations = 6
mbMinRad2 = 0.32076
mbScale = -2.15176
mbRotVector = 1,0,0
mbRotAngle = 0
BoundingSphere = 10.606
Specular = 4
Up = -0.102593,0.994723,-0.000825327
#endpreset

#preset orangee
FOV = 0.56376
Eye = 0.000464542,-0.268025,-0.29852
Target = 0.00316212,8.54426,-1.2325
EquiRectangular = false
FocalPlane = 1
Aperture = 0
Gamma = 2.08335
ToneMapping = 3
Exposure = 0.6522
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
Detail = -6.63705
DetailAO = -1.35716
FudgeFactor = 0.07619
MaxRaySteps = 2743
Dither = 1
NormalBackStep = 1
AO = 0,0,0,0.85185
SpecularExp = 16.364
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = 0.2771,-0.61446
CamLight = 1,1,1,1.53846
CamLightMin = 0.12121
Glow = 1,1,1,0.43836
GlowMax = 52
Fog = 0
HardShadow = 0.35385
ShadowSoft = 12.5806
Reflection = 0.0
BaseColor = 1,1,1
OrbitStrength = 1
X = 0.5,0.6,0.6,0.31578
Y = 1,0.6,0,0.35652
Z = 0.8,0.78,1,0.38596
R = 0.4,0.7,1,1
BackgroundColor = 0.6,0.6,0.45
GradientBackground = 2.06895
CycleColors = true
Cycles = 2.59614
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
InvertC = -0.5357,1.6964,0.4464
Iterations = 6
ColorIterations = 8
Power = 8
Bailout = 1.5
AlternateVersion = true
RotVector = 1,1,1
RotAngle = 0
Julia = true
JuliaC = -1.00828,-0.08264,0.01652
msIterations = 10
msColorIterations = 8
msScale = 2.57144
msRotVector = 1,0,0
msRotAngle = 0
msOffset = 1,1.00008,1.28204
mbIterations = 7
mbColorIterations = 8
mbMinRad2 = 0.17778
mbScale = 2.4
mbRotVector = 1,0,0
mbRotAngle = 0
Specular = 4
DebugSun = false
Up = 0,-1,0
#endpreset

#preset Traverse
FOV = 0.51006
Eye = 3.53269,-1.56411,0
Target = 0.666,-0.666,0
EquiRectangular = false
FocalPlane = 1
Aperture = 0
Gamma = 2.08335
ToneMapping = 3
Exposure = 0.6522
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
Detail = -6.63705
DetailAO = 0
FudgeFactor = 1
MaxRaySteps = 2478
BoundingSphere = 10.606
Dither = 1
NormalBackStep = 1
AO = 0,0,0,0.85185
Specular = 4
SpecularExp = 16.364
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = -0.5,-0.5
CamLight = 1,1,1,2
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 82
Fog = 0
HardShadow = 0.35385
ShadowSoft = 12.5806
Reflection = 0
BaseColor = 1,1,1
OrbitStrength = 1
X = 0.5,0.6,0.6,-0.38596
Y = 1,0.6,0,1
Z = 0.8,0.78,1,-0.19298
R = 0.4,0.7,1,1
BackgroundColor = 0.6,0.6,0.45
GradientBackground = 5
CycleColors = true
Cycles = 3.5
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
InvertC = -0.7812,0.7813,0.4688
Iterations = 0
ColorIterations = 8
Power = 8
Bailout = 1.5
AlternateVersion = true
RotVector = 1,1,1
RotAngle = 0
Julia = false
JuliaC = -1.00828,0,0
msIterations = 0
msColorIterations = 6
msScale = 2.57144
msRotVector = 1,0,0
msRotAngle = 0
msOffset = 1,1.00008,1.28204
mbIterations = 16
mbColorIterations = 6
mbMinRad2 = 0.32076
mbScale = -2.4
mbRotVector = 1,0,0
mbRotAngle = 0
Up = 0.200036,0.979787,-0.00189497
#endpreset

#preset Carasel
FOV = 0.8859
Eye = 1.32998,-2.58711,1e-05
Target = -50,0.00261887,1e-05
EquiRectangular = false
FocalPlane = 1
Aperture = 0
Gamma = 0.00208552
ToneMapping = 3
Exposure = 0.6522
Brightness = 1.0015
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
Detail = -5
DetailAO = 0
FudgeFactor = 0.5
MaxRaySteps = 1024
BoundingSphere = 3.606
Dither = 1
NormalBackStep = 1
AO = 0,0,0,0.85185
Specular = 4
SpecularExp = 16.364
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = -0.46988,-0.3735
CamLight = 1,1,1,2
CamLightMin = 0
Glow = 1,1,1,0.07143
GlowMax = 103
Fog = 0.16176
HardShadow = 0.35385
ShadowSoft = 12.5806
Reflection = 0
BaseColor = 1,1,1
OrbitStrength = 1
X = 0.5,0.6,0.6,-0.38596
Y = 1,0.6,0,1
Z = 0.8,0.78,1,-0.19298
R = 0.4,0.7,1,1
BackgroundColor = 0.6,0.6,0.45
GradientBackground = 2.0238
CycleColors = true
Cycles = 3.5
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
InvertC = -0.7812,0.7813,0.4688
Iterations = 0
ColorIterations = 8
Power = 8
Bailout = 1.5
AlternateVersion = true
RotVector = 1,1,1
RotAngle = 0
Julia = false
JuliaC = -1.00828,0,0
msIterations = 0
msColorIterations = 6
msScale = 2.57144
msRotVector = 1,0,0
msRotAngle = 0
msOffset = 1,1.00008,1.28204
mbIterations = 16
mbColorIterations = 6
mbMinRad2 = 0.32076
mbScale = 3
mbRotVector = 0,1,0
mbRotAngle = 9.22547e-13
Up = 0,1,0
/// mbRotAngle1:InOutCubic:7:0:360:50:2950:0.3:1:1.7:1:0
/// Target2:CosineCurve:44:-50:50:1:3000:0.3:1:1.7:1:0
/// Gamma1:Linear:0:0:2.0021:1:25:0.3:1:1.7:1:0
/// Brightness1:Linear:0:1.0025:0:2975:3000:0.3:1:1.7:1:0
#endpreset

#preset Portal
FOV = 1.00672
Eye = 2.11092,-0.234671,0
Target = 0.110918,0.0653294,0
EquiRectangular = false
FocalPlane = 1
Aperture = 0
Gamma = 2
ToneMapping = 3
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
Detail = -2.84956
DetailAO = -1.35716
FudgeFactor = 1
MaxRaySteps = 752
BoundingSphere = 12
Dither = 0.51754
NormalBackStep = 1
AO = 0,0,0,1
Specular = 1.6456
SpecularExp = 16.364
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = -0.54216,0.42168
CamLight = 1,1,1,1
CamLightMin = 0
Glow = 1,1,1,0.5
GlowMax = 82
Fog = 0
HardShadow = 0.35385 NotLocked
ShadowSoft = 12.5806
Reflection = 0 NotLocked
BaseColor = 1,1,1
OrbitStrength = 1
X = 0.5,0.6,0.6,0.7
Y = 1,0.6,0,0.4
Z = 0.8,0.78,1,0.5
R = 0.4,0.7,1,0.33334
BackgroundColor = 0.6,0.6,0.45
GradientBackground = 1.89655
CycleColors = true
Cycles = 4
EnableFloor = false NotLocked
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
InvertC = 0.5,-0.33,0 NotLocked
Iterations = 0 NotLocked
ColorIterations = 8 NotLocked
Power = 8 NotLocked
Bailout = 6.279 NotLocked
AlternateVersion = true NotLocked
RotVector = 1,1,1 NotLocked
RotAngle = 0 NotLocked
Julia = false NotLocked
JuliaC = 0,0,0 NotLocked
msIterations = 12 NotLocked
msScale = 2.5 NotLocked
msRotVector = 1,0,0 NotLocked
msRotAngle = 0 NotLocked
msOffset = 2,2,2 NotLocked
mbIterations = 12 NotLocked
mbColorIterations = 6 NotLocked
mbMinRad2 = 0.5 NotLocked
mbRotVector = 1,1,1 NotLocked
mbRotAngle = 0 NotLocked
mbScale = 3
msColorIterations = 6
Up = 0.146699,0.977995,0
#endpreset

#preset VeryCool
FOV = 0.56376
Eye = -0.0590995,-0.6552233,-0.6347637
Target = 2.11322,7.913226,-0.0091506
Gamma = 2.08335
ToneMapping = 3
Exposure = 0.6522
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 6
AntiAliasScale = 1.5
Detail = -6.6
DetailAO = -2.783133
FudgeFactor = 0.06
MaxRaySteps = 5000
Dither = 1
NormalBackStep = 1
AO = 0,0,0,1
CamLight = 1,1,1,1.038461
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 52
BaseColor = 1,1,1
OrbitStrength = 0.7931034
X = 0.5,0.6,0.6,0.3191489
Y = 1,0.6,0,0.1276596
Z = 0.8,0.78,1,0.38596
R = 0.4,0.7,1,0.1914894
BackgroundColor = 0.05882353,0.07843137,0.1019608
GradientBackground = 0
CycleColors = true
Cycles = 2.707692
EnableFloor = false NotLocked
FloorNormal = 0,0,1
FloorHeight = 0
FloorColor = 1,1,1
InvertC = 0,-0.346154,-0.5 NotLocked
Iterations = 7
ColorIterations = 2
Power = 8
Bailout = 1.5
AlternateVersion = true
RotVector = 1,1,1
RotAngle = 0
Julia = true
JuliaC = -0.12,-0.12,-1.04
msIterations = 8
msColorIterations = 1
msScale = 2.57144
msRotVector = 1,0,0
msRotAngle = 0
msOffset = 1,1.00008,1.28204
mbIterations = 8
mbColorIterations = 1
mbMinRad2 = 0.17778
mbScale = 2.4
mbRotVector = 1,0,0
mbRotAngle = 0
FocalPlane = 1
Aperture = 0
InFocusAWidth = 0
ApertureNbrSides = 5
ApertureRot = 0
ApStarShaped = false
Bloom = false
BloomIntensity = 0.25
BloomPow = 2
BloomTaps = 4
MaxDistance = 10
AoCorrect = 0.5189873
Specular = 2
SpecularExp = 16.364
Reflection = 0,0,0
ReflectionsNumber = 0
SpotGlow = true
SpotLight = 1,1,1,1
LightPos = -10,-10,3
LightSize = 0.5058824
LightFallOff = 0
LightGlowRad = 0
LightGlowExp = 1
HardShadow = 1
ShadowSoft = 20
HF_Fallof = 0.1
HF_Const = 0
HF_Intensity = 0
HF_Dir = 0,0,1
HF_Offset = 0
HF_Color = 1,1,1,1
Up = -0.1662658,-0.0296897,0.9839576
mbRotAngle1:InOutCubic:7:0:360:50:2950:0.3:1:1.7:1:0
Gamma1:Linear:0:0:2.0021:1:25:0.3:1:1.7:1:0
Brightness1:Linear:0:1.0025:0:2975:3000:0.3:1:1.7:1:0
#endpreset
