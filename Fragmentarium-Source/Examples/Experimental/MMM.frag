#version 330 compatibility
// Output generated from file: Fragmentarium/Examples/Experimental/3Dickulus.frag
// Created: Fri Nov 13 21:43:17 2015
#info Mandelbulb Distance Estimator + Menger Sphere (knighty) + MandelBox (originaly devised by Buddhi)
#define providesInit
#define MULTI_SAMPLE_AO
#define WANG_HASH
#include "MathUtils.frag"
//#define USE_IQ_CLOUDS
uniform float time;
#include "DE-Kn2cr11.frag"

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

// Compute the distance from `pos` to the Mandelbulb.
float mandelDE(vec3 pos) {
	vec3 z=pos;
	float r;
	float dr=1.0;
	int i=0;
	r=length(z);
	while(r<Bailout && (i<Iterations)) {
		if (AlternateVersion) {
			powN2(z,r,dr);
		} else {
			powN1(z,r,dr);
		}
		z+=(Julia ? JuliaC : pos);
		r=length(z);
		z*=rot;
		if (i<ColorIterations) orbitTrap = min(orbitTrap, abs(vec4(z.x,z.y,z.z,r*r)));
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
		if (i<msColorIterations) orbitTrap = min(orbitTrap, abs(vec4(p.x,p.y,p.z,r2*r2)));
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
The distance estimator below was originaly devised by Buddhi.
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
//		if (i<mbColorIterations) orbitTrap = min(orbitTrap, abs(vec4(domainMap(-p.xyz),r2)));
		p *= clamp(max(mbMinRad2/r2, mbMinRad2), 0.0, 1.0);  // dp3,div,max.sat,mul
		p = p*mbscale + p0;
		if ( r2>1000.0) break;
		if (i<mbColorIterations) orbitTrap = min(orbitTrap, abs(vec4(p.x,p.y,p.z,r2*r2)));
	}
	return ((length(p.xyz) - absScalem1) / p.w - AbsScaleRaisedTo1mIters);
}

float DE(vec3 pos) {
  float rd = 0.0;
  orbitTrap=vec4(10000.0);
  // this can be + or - or another function
  if(Iterations > 0) rd = mandelDE(abs(pos));
  if(msIterations > 0) rd += mengersDE(abs(pos));
  if(mbIterations > 0) rd += mandelboxDE(abs(pos));

 return rd;
};

#preset Default
FOV = 0.56376
Eye = -0.0590995,-0.6552233,-0.6347637
Target = 2.11322,7.913226,-0.0091506
Up = -0.1662658,-0.0296897,0.9839576
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
#endpreset

#preset OctopusGarden
FOV = 0.5
Eye = -0.0005569,-0.8117748,-0.4108088
Target = -0.0006804,8.014729,0.620401
FocalPlane = 1
Aperture = 0
InFocusAWidth = 0
ApertureNbrSides = 5
ApertureRot = 0
ApStarShaped = false
Gamma = 2.08335
ToneMapping = 3
Exposure = 0.6522
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 6
AntiAliasScale = 1.5
Bloom = false
BloomIntensity = 0.25
BloomPow = 2
BloomTaps = 4
Detail = -6.646465
DetailAO = -2.951807
FudgeFactor = 0.0454545
MaxRaySteps = 5000
MaxDistance = 10
Dither = 1
NormalBackStep = 1
AO = 0,0,0,1
AoCorrect = 0.5189873
Specular = 1.552941
SpecularExp = 10
CamLight = 1,1,1,0.5
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 52
Reflection = 0,0,0
ReflectionsNumber = 0
SpotGlow = false
SpotLight = 1,1,1,3
LightPos = -5.526316,-3.421053,-0.2631579
LightSize = 0
LightFallOff = 0
LightGlowRad = 0
LightGlowExp = 1
HardShadow = 1
ShadowSoft = 20
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
EnableFloor = false
FloorNormal = 0,0,1
FloorHeight = 0
FloorColor = 1,1,1
HF_Fallof = 0.8251598
HF_Const = 0.0212766
HF_Intensity = 0.0128205
HF_Dir = 0,0,1
HF_Offset = 4.320988
HF_Color = 0.05882353,0.2196078,0.3607843,1
Iterations = 9
ColorIterations = 0
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
Up = 0.0003646,-0.1160122,0.992991
#endpreset

#preset OnLoftyPeak
FOV = 0.5
Eye = 0.4073507,0.1433923,0.4106582
Target = 4.536118,1.406503,1.044394
FocalPlane = 1
Aperture = 0
InFocusAWidth = 0
ApertureNbrSides = 5
ApertureRot = 0
ApStarShaped = false
Gamma = 2.08335
ToneMapping = 3
Exposure = 0.6522
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 6
AntiAliasScale = 1.5
Bloom = false
BloomIntensity = 0.25
BloomPow = 2
BloomTaps = 4
Detail = -6.6
DetailAO = -2.783133
FudgeFactor = 0.06
MaxRaySteps = 5000
MaxDistance = 10
Dither = 1
NormalBackStep = 1
AO = 0,0,0,1
AoCorrect = 0.5189873
Specular = 2
SpecularExp = 16.364
CamLight = 1,1,1,1.038461
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 52
Reflection = 0,0,0
ReflectionsNumber = 0
SpotGlow = true
SpotLight = 1,1,1,3.555556
LightPos = -2.368421,-1.052632,6.31579
LightSize = 0
LightFallOff = 0
LightGlowRad = 0
LightGlowExp = 1
HardShadow = 1
ShadowSoft = 20
BaseColor = 1,1,1
OrbitStrength = 1
X = 0.5,0.6,0.6,0.1489362
Y = 1,0.6,0,0.212766
Z = 0.8,0.78,1,0.1521739
R = 0.4,0.7,1,0.4680851
BackgroundColor = 0.05882353,0.07843137,0.1019608
GradientBackground = 0
CycleColors = true
Cycles = 2.707692
EnableFloor = false
FloorNormal = 0,0,1
FloorHeight = 0
FloorColor = 1,1,1
HF_Fallof = 3.453763
HF_Const = 0
HF_Intensity = 0.0384615
HF_Dir = 0,0.0280374,1
HF_Offset = 0.8641975
HF_Color = 0.5411765,0.5411765,0.5411765,1.522388
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
Up = -0.1235818,-0.0224279,0.8498326
#endpreset

#preset investigate
FOV = 0.2857143
Eye = -0.5923999,0.0312734,-0.0696783
Target = -5.225647,7.196972,2.209335
FocalPlane = 1
Aperture = 0
InFocusAWidth = 0
ApertureNbrSides = 5
ApertureRot = 0
ApStarShaped = false
Gamma = 2.08335
ToneMapping = 3
Exposure = 0.6522
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 6
AntiAliasScale = 1.5
Bloom = false
BloomIntensity = 0.25
BloomPow = 2
BloomTaps = 4
Detail = -6.646465
DetailAO = -2.951807
FudgeFactor = 0.0454545
MaxRaySteps = 5000
MaxDistance = 10
Dither = 1
NormalBackStep = 1
AO = 0,0,0,1
AoCorrect = 0.5189873
Specular = 1.552941
SpecularExp = 10
CamLight = 1,1,1,1
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 52
Reflection = 0,0,0
ReflectionsNumber = 0
SpotGlow = false
SpotLight = 1,1,1,2.555556
LightPos = -2.368421,-2.105263,-0.2631579
LightSize = 0.5058824
LightFallOff = 0
LightGlowRad = 0
LightGlowExp = 1
HardShadow = 1
ShadowSoft = 9
BaseColor = 1,1,1
OrbitStrength = 0.8793103
X = 0.5,0.6,0.6,0.1914894
Y = 1,0.6,0,0.0851064
Z = 0.8,0.78,1,0.1521739
R = 0.4,0.7,1,0.0425532
BackgroundColor = 0.05882353,0.07843137,0.1019608
GradientBackground = 0
CycleColors = true
Cycles = 9.1
EnableFloor = false
FloorNormal = 0,0,1
FloorHeight = 0
FloorColor = 1,1,1
HF_Fallof = 0.8251598
HF_Const = 0.0212766
HF_Intensity = 0.0128205
HF_Dir = 0,0,1
HF_Offset = 4.320988
HF_Color = 0.05882353,0.2196078,0.3607843,1
Iterations = 9
ColorIterations = 0
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
Up = -0.156149,0.1965736,-0.9355207
#endpreset

#preset IguanaQueen
FOV = 0.5
Eye = 0.256894,0.001596,0.6281675
Target = 4.549426,-0.0191476,-0.1413791
FocalPlane = 0.351471
Aperture = 0.002
InFocusAWidth = 0.3090909
ApertureNbrSides = 5
ApertureRot = 0
ApStarShaped = false
Gamma = 2.08335
ToneMapping = 3
Exposure = 0.6522
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 6
AntiAliasScale = 1.5
Bloom = false
BloomIntensity = 0.25
BloomPow = 2
BloomTaps = 4
Detail = -6.6
DetailAO = -2.783133
FudgeFactor = 0.06
MaxRaySteps = 3200
MaxDistance = 10
Dither = 1
NormalBackStep = 1
AO = 0,0,0,1
AoCorrect = 0.5189873
Specular = 2
SpecularExp = 16.364
CamLight = 1,1,1,1.038461
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 52
Reflection = 0,0,0
ReflectionsNumber = 0
SpotGlow = true
SpotLight = 1,1,1,3.555556
LightPos = -2.368421,-1.315789,6.052632
LightSize = 0
LightFallOff = 0
LightGlowRad = 0
LightGlowExp = 1
HardShadow = 1
ShadowSoft = 20
BaseColor = 1,1,1
OrbitStrength = 1
X = 0.5,0.6,0.6,0.1489362
Y = 1,0.6,0,0.212766
Z = 0.8,0.78,1,0.1521739
R = 0.4,0.7,1,0.4680851
BackgroundColor = 0.05882353,0.07843137,0.1019608
GradientBackground = 0
CycleColors = true
Cycles = 2.707692
EnableFloor = false
FloorNormal = 0,0,1
FloorHeight = 0
FloorColor = 1,1,1
HF_Fallof = 5
HF_Const = 0
HF_Intensity = 0.0384615
HF_Dir = 0,0,1
HF_Offset = -2.098765
HF_Color = 0.5411765,0.5411765,0.5411765,1.522388
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
Up = -0.1764684,-0.0015172,-0.9843001
#endpreset

#preset TigersAndPythons
FOV = 0.5
Eye = -0.6304394,0,0.7836212
Target = 8.178603,0,0.0183695
FocalPlane = 1
Aperture = 0
InFocusAWidth = 0
ApertureNbrSides = 5
ApertureRot = 0
ApStarShaped = false
Gamma = 1
ToneMapping = 2
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 6
AntiAliasScale = 1.5
Bloom = false
BloomIntensity = 0.25
BloomPow = 2
BloomTaps = 4
Detail = -6.6
DetailAO = -2.783133
FudgeFactor = 0.06
MaxRaySteps = 5000
MaxDistance = 10
Dither = 1
NormalBackStep = 1
AO = 0,0,0,1
AoCorrect = 0.5189873
Specular = 2
SpecularExp = 16.364
CamLight = 1,1,1,1.5
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 52
Reflection = 0,0,0
ReflectionsNumber = 0
SpotGlow = true
SpotLight = 1,1,1,3
LightPos = -2.105263,0,7.105263
LightSize = 0.5058824
LightFallOff = 0
LightGlowRad = 0
LightGlowExp = 1
HardShadow = 1
ShadowSoft = 0
BaseColor = 1,1,1
OrbitStrength = 0.6206897
X = 1,1,1,-0.787234
Y = 0.345098,0.666667,0,0.4893617
Z = 1,0.666667,0,-0.5652174
R = 0.0784314,1,0.941176,-0.212766
BackgroundColor = 0,0,0
GradientBackground = 0
CycleColors = true
Cycles = 5.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
HF_Fallof = 0.1
HF_Const = 0
HF_Intensity = 0
HF_Dir = 0,0,1
HF_Offset = 0
HF_Color = 1,1,1,1
Iterations = 8
ColorIterations = 4
Power = 8
Bailout = 3.679245
AlternateVersion = true
RotVector = 1,0,0
RotAngle = 0
Julia = true
JuliaC = -0.12,-0.12,-1.04
msIterations = 10
msColorIterations = 1
msScale = 2.57144
msRotVector = 1,0,0
msRotAngle = 0
msOffset = 1,1.00008,1.28204
mbIterations = 0
mbColorIterations = 0
mbMinRad2 = 1.292683
mbScale = 3
mbRotVector = 1,0,0
mbRotAngle = 0
Up = 0.243943,-8.18e-05,0.9316214
#endpreset

#preset SnakeInTheGrass
FOV = 0.375
Eye = -0.695532,2.28e-05,0.78805
Target = 7.858263,0.027783,-1.45174
FocalPlane = 1
Aperture = 0
InFocusAWidth = 0
ApertureNbrSides = 5
ApertureRot = 0
ApStarShaped = false
Gamma = 1
ToneMapping = 2
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 6
AntiAliasScale = 1.5
Bloom = false
BloomIntensity = 0.25
BloomPow = 2
BloomTaps = 4
Detail = -6.6
DetailAO = -2.783133
FudgeFactor = 0.06
MaxRaySteps = 5000
MaxDistance = 10
Dither = 1
NormalBackStep = 1
AO = 0,0,0,1
AoCorrect = 0.5189873
Specular = 2
SpecularExp = 16.364
CamLight = 1,1,1,1.5
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 52
Reflection = 0,0,0
ReflectionsNumber = 0
SpotGlow = true
SpotLight = 1,1,1,3
LightPos = -2.105263,0,7.105263
LightSize = 0.5058824
LightFallOff = 0
LightGlowRad = 0
LightGlowExp = 1
HardShadow = 1
ShadowSoft = 0
BaseColor = 1,1,1
OrbitStrength = 0.75
X = 1,1,1,-1
Y = 0.345098,0.666667,0,-0.5
Z = 1,0.666667,0,1
R = 0.0784314,1,0.941176,0
BackgroundColor = 0,0,0
GradientBackground = 0
CycleColors = true
Cycles = 5.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
HF_Fallof = 0.1
HF_Const = 0
HF_Intensity = 0
HF_Dir = 0,0,1
HF_Offset = 0
HF_Color = 1,1,1,1
Iterations = 8
ColorIterations = 2
Power = 8
Bailout = 3.679245
AlternateVersion = true
RotVector = 1,0,0
RotAngle = 0
Julia = true
JuliaC = -0.12,-0.12,-1.04
msIterations = 12
msColorIterations = 2
msScale = 2.57144
msRotVector = 1,0,0
msRotAngle = 0
msOffset = 1,1.00008,1.28204
mbIterations = 0
mbColorIterations = 0
mbMinRad2 = 1.292683
mbScale = 3
mbRotVector = 1,0,0
mbRotAngle = 0
Up = 0.243943,-8.18e-05,0.9316214
#endpreset

#preset Nudibranch1
FOV = 0.0100001
Eye = -0.9372891,-0.0011118,0.8796393
Target = -0.0281535,-0.297624,0.587041
Up = 0.0918849,-0.0015185,0.2868449
EquiRectangular = false
Gamma = 1
ToneMapping = 1
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 6
AntiAliasScale = 2
DepthToAlpha = false
ShowDepth = false
DepthMagnitude = 1
Detail = -4.5
FudgeFactor = 0.02
NormalBackStep = 1
CamLight = 1,1,1,0.5
BaseColor = 1,1,1
OrbitStrength = 0.79
X = 1,1,1,0.1818182
Y = 0.345098,0.666667,0,-0.5909091
Z = 1,0.666667,0,1
R = 0.0784314,1,0.941176,0.0151515
BackgroundColor = 0,0,0
GradientBackground = 0
CycleColors = true
Cycles = 6
EnableFloor = false NotLocked
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 8
ColorIterations = 10
Power = 8
Bailout = 30
AlternateVersion = true
RotVector = 1,0,0
Julia = true
JuliaC = -0.12,-0.12,-1.04
FocalPlane = 1.324811
Aperture = 0.0121951
InFocusAWidth = 0.25
DofCorrect = true
ApertureNbrSides = 5
ApertureRot = 0
ApStarShaped = false
Bloom = false
BloomIntensity = 0.25
BloomPow = 2
BloomTaps = 4
BloomStrong = 1
RefineSteps = 1
MaxRaySteps = 12000
MaxDistance = 10
Dither = 1
DetailAO = -1.176471
coneApertureAO = 0.5
maxIterAO = 10
FudgeAO = 1
AO_ambient = 2
AO_camlight = 1
AO_pointlight = 2
AoCorrect = 1
Specular = 1
SpecularExp = 16.364
AmbiantLight = 1,1,1,0.5
Reflection = 0.4980392,0.4980392,0.4980392
ReflectionsNumber = 0
SpotGlow = true
SpotLight = 1,1,1,1
LightPos = -3.142857,0,2.642857
LightSize = 0.5058824
LightFallOff = 0
LightGlowRad = 0
LightGlowExp = 1
HardShadow = 1
ShadowSoft = 0
ShadowBlur = 0
perf = false
SSS = false
sss1 = 0.1
sss2 = 0.5
HF_Fallof = 5
HF_Const = 0.0247934
HF_Intensity = 1
HF_Dir = 0,0,1
HF_Offset = 0.5555556
HF_Color = 1,1,1,1
HF_Scatter = 0
HF_Anisotropy = 0,0,0
HF_FogIter = 1
HF_CastShadow = false
EnCloudsDir = false
CloudDir = 0,0,1
CloudScale = 1
CloudFlatness = 0
CloudTops = 1
CloudBase = -1
CloudDensity = 1
CloudRoughness = 1
CloudContrast = 1
CloudColor = 0.65,0.68,0.7
CloudColor2 = 0.07,0.17,0.24
SunLightColor = 0.7,0.5,0.3
Cloudvar1 = 0.99
Cloudvar2 = 1
CloudIter = 5
CloudBgMix = 1
WindDir = 0,0,1
WindSpeed = 1
RotAngle = 0
msIterations = 12
msColorIterations = 0
msScale = 2.57144
msRotVector = 1,0,0
msRotAngle = 60
msOffset = 1,1,1.28204
mbIterations = 0
mbColorIterations = 0
mbMinRad2 = 1.292683
mbScale = 3
mbRotVector = 1,0,0
mbRotAngle = 0
#endpreset

#preset Nudibranch2
FOV = 0.6133333
Eye = -0.6451511,-0.0963917,0.7856169
Target = 0.294021,-0.329636,0.518771
Up = 0.0918342,-0.001502,0.2868612
EquiRectangular = false
Gamma = 1.25
ToneMapping = 1
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 6
AntiAliasScale = 1.5
DepthToAlpha = true
ShowDepth = false
DepthMagnitude = 1
Detail = -3
FudgeFactor = 0.03
NormalBackStep = 1
CamLight = 1,1,1,2
BaseColor = 1,1,1
OrbitStrength = 0.79
X = 1,1,1,0.1818182
Y = 0.345098,0.666667,0,-0.5909091
Z = 1,0.666667,0,1
R = 0.0784314,1,0.941176,0.0151515
BackgroundColor = 0,0,0
GradientBackground = 0
CycleColors = true
Cycles = 6
EnableFloor = false NotLocked
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 8
ColorIterations = 10
Power = 8
Bailout = 30
AlternateVersion = true
RotVector = 1,0,0
Julia = true
JuliaC = -0.12,-0.12,-1.04
FocalPlane = 0.9588113
Aperture = 0
InFocusAWidth = 1
DofCorrect = false
ApertureNbrSides = 5
ApertureRot = 0
ApStarShaped = false
Bloom = false
BloomIntensity = 0.25
BloomPow = 2
BloomTaps = 4
BloomStrong = 1
RefineSteps = 3
MaxDistance = 10
Dither = 3
DetailAO = -2.058823
coneApertureAO = 0.1428571
maxIterAO = 6
FudgeAO = 1
AO_ambient = 2
AO_camlight = 1
AO_pointlight = 1
AoCorrect = 0.5
Specular = 1
SpecularExp = 16.364
AmbiantLight = 1,1,1,1
Reflection = 0.4980392,0.4980392,0.4980392
ReflectionsNumber = 0
SpotGlow = true
SpotLight = 1,1,1,3
LightPos = -3.142857,0,2.642857
LightSize = 0.5058824
LightFallOff = 0
LightGlowRad = 0
LightGlowExp = 1
HardShadow = 1
ShadowSoft = 0
ShadowBlur = 0
perf = false
SSS = false
sss1 = 0.1
sss2 = 0.5
HF_Fallof = 5
HF_Const = 0.0247934
HF_Intensity = 1
HF_Dir = 0,0,1
HF_Offset = 0.5555556
HF_Color = 1,1,1,1
HF_Scatter = 0
HF_Anisotropy = 0,0,0
HF_FogIter = 1
HF_CastShadow = false
EnCloudsDir = false
CloudDir = 0,0,1
CloudScale = 1
CloudFlatness = 0
CloudTops = 1
CloudBase = -1
CloudDensity = 1
CloudRoughness = 1
CloudContrast = 1
CloudColor = 0.65,0.68,0.7
CloudColor2 = 0.07,0.17,0.24
SunLightColor = 0.7,0.5,0.3
Cloudvar1 = 0.99
Cloudvar2 = 1
CloudIter = 5
CloudBgMix = 1
WindDir = 0,0,1
WindSpeed = 1
RotAngle = 0
msIterations = 12
msColorIterations = 0
msScale = 1.99
msRotVector = 1,0,0
msRotAngle = 60
msOffset = 1,1,1.28204
mbIterations = 0
mbColorIterations = 0
mbMinRad2 = 1.292683
mbScale = 3
mbRotVector = 1,0,0
mbRotAngle = 0
MaxRaySteps = 9000
#endpreset

#preset MengersInferno
FOV = 0.25
Eye = 0,-0.514477,1.460829
Target = 0,-0.0434643,0.5785431
Up = -0.0031481,0.8040356,0.4292388
EquiRectangular = false
Gamma = 1
ToneMapping = 3
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 6
AntiAliasScale = 2
DepthToAlpha = false
ShowDepth = false
DepthMagnitude = 1
Detail = -7
FudgeFactor = 0.03
NormalBackStep = 1
CamLight = 1,1,1,0.5
BaseColor = 1,1,1
OrbitStrength = 0.575
X = 1,1,1,-1
Y = 0.345098,0.666667,0,1
Z = 1,0.666667,0,1
R = 0.0784314,1,0.941176,-0.2765957
BackgroundColor = 0,0,0
GradientBackground = 0
CycleColors = false
Cycles = 0.1
EnableFloor = false NotLocked
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 8
ColorIterations = 8
Power = 8
Bailout = 3.679245
AlternateVersion = false
RotVector = 1,0,0
Julia = false
JuliaC = 0.32,0.2,0.36
FocalPlane = 1.7
Aperture = 0
InFocusAWidth = 0.5
DofCorrect = true
ApertureNbrSides = 5
ApertureRot = 0
ApStarShaped = false
Bloom = false
BloomIntensity = 0.25
BloomPow = 2
BloomTaps = 4
BloomStrong = 1
RefineSteps = 8
MaxRaySteps = 15000
MaxDistance = 5
Dither = 2.5
DetailAO = -3.5
coneApertureAO = 0.25
maxIterAO = 8
FudgeAO = 1
AO_ambient = 1
AO_camlight = 1
AO_pointlight = 0.5
AoCorrect = 1
Specular = 2
SpecularExp = 20
AmbiantLight = 1,1,1,0.5
Reflection = 0.4941176,0.4941176,0.4941176
ReflectionsNumber = 0
SpotGlow = true
SpotLight = 1,1,1,5
LightPos = -0.3571429,2.857143,5.357143
LightSize = 0.5058824
LightFallOff = 0
LightGlowRad = 0
LightGlowExp = 1
HardShadow = 1
ShadowSoft = 20
ShadowBlur = 0.1619048
perf = false
SSS = true
sss1 = 0.1027397
sss2 = 0.1862069
HF_Fallof = 5
HF_Const = 0.0744681
HF_Intensity = 1
HF_Dir = 0,0,1
HF_Offset = 0
HF_Color = 1,1,1,1
HF_Scatter = 0
HF_Anisotropy = 0,0,0
HF_FogIter = 1
HF_CastShadow = false
EnCloudsDir = false
CloudDir = 0,0,1
CloudScale = 0.5868852
CloudFlatness = 0
CloudTops = 1
CloudBase = -1
CloudDensity = 1
CloudRoughness = 2
CloudContrast = 10
CloudColor = 0.65,0.68,0.7
CloudColor2 = 0.07,0.17,0.24
SunLightColor = 0.7,0.5,0.3
Cloudvar1 = 0.99
Cloudvar2 = 1
CloudIter = 5
CloudBgMix = 1
WindDir = 0,0,1
WindSpeed = 1
RotAngle = 0
msIterations = 14
msColorIterations = 14
msScale = 2.530612
msRotVector = 1,0,0
msRotAngle = 0
msOffset = 1,1,1.822222
mbIterations = 0
mbColorIterations = 0
mbMinRad2 = 1.292683
mbScale = 3
mbRotVector = 1,0,0
mbRotAngle = 0
RotAngle1:Linear:0:352:360:81:90:0.3:1:1.7:1:0
#endpreset
