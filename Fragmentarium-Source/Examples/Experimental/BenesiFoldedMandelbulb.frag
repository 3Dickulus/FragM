// http://www.fractalforums.com/index.php?topic=22583.msg88215#msg88215
// modifications to Benesi's original formula hacked into mandelbulb by 3Dickulus
#info Mandelbulb Distance Estimator
#define providesInit

#include "MathUtils.frag"
#include "DE-Raytracer.frag"
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

uniform float RotAngle; slider[0.00,0,180]

uniform bool Julia; checkbox[false]
uniform vec3 JuliaC; slider[(-2,-2,-2),(0,0,0),(2,2,2)]

uniform float time;
mat3 rot;
void init() {
	 rot = rotationMatrix3(normalize(RotVector), RotAngle);
}


// keep scale values the same for each variable, for a more orderly fractal
uniform vec3 bscale; slider[(-5,-5,-5),(2,2,2),(5,5,5)]
//only use x component, for a more orderly fractal
uniform vec3 boffset; slider[(-5,-5,-5),(2,0,0),(5,5,5)]
// Pine formula will execute at this bulb iteration
uniform int PineIterAt;slider[0,10,10]

/*const*/	float sr23=sqrt(2./3.);
/*const*/	float sr13=sqrt(1./3.);
/*const*/	float sr12=sqrt(.5);

void BenesiFold(inout vec3 b) {
// STEP1: "Benesi fold 1"
float tx=(b.x*sr23-b.z*sr13)*sr12;
b.z=abs(b.x*sr13 + b.z*sr23);
b.x=abs(tx-b.y*sr12);
b.y=abs(tx+b.y*sr12);

tx=b.x*sr12+b.y*sr12;
b.z=-tx*sr13+b.z*sr23;
b.y=-b.x*sr12+b.y*sr12;
b.x=tx*sr23+b.z*sr13;

b.x=bscale.x*b.x-boffset.x;  //scale =2    offset=2
b.y=bscale.y*b.y-boffset.y;;
b.z=bscale.z*b.z-boffset.z;;
}

void BenesiPine(inout vec3 b) {
vec3 tb = b;
// STEP2: "Benesi pinetree"
float xt = b.x*b.x;
float yt = b.y*b.y;
float zt = b.z*b.z;
float t = b.x/sqrt(yt+zt); // removed 2.0*
b.x = xt-yt-zt+sr23;
b.z = t*(yt-zt)+sr13;
b.y = t*b.y*b.z+sr12; // removed 2.0*
// captures escapees
b = length(b)>length(tb)?tb:b;
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
float DE(vec3 pos) {
	vec3 z=pos;
	float r;
	float dr=1.0;
	int i=0;

   BenesiFold(z);

	r=length(z);

	while(r<Bailout && (i<Iterations)) {

		if (AlternateVersion) {
			powN2(z,r,dr);
		} else {
			powN1(z,r,dr);
		}

     if(PineIterAt == i) BenesiPine(z);

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

#preset Default
FOV = 0.6
Eye = 1.94002,0.0302759,0.263316
Target = -6.72831,-0.132973,-1.48656
EquiRectangular = false
FocalPlane = 1
Aperture = 0
Gamma = 2
ToneMapping = 3
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 6
AntiAliasScale = 2
Detail = -3.5
DetailAO = -1.47
FudgeFactor = 1
MaxDistance = 10
MaxRaySteps = 300
Dither = 1
NormalBackStep = 0 NotLocked
AO = 0,0,0,0.85185
Specular = 1
SpecularExp = 21.56863
SpecularMax = 30.61225
SpotLight = 1,1,1,1
SpotLightDir = 0.6774194,0.2258065
CamLight = 1,1,1,1.53846
CamLightMin = 0.12121
Glow = 1,1,1,0.43836
GlowMax = 52
Fog = 0
HardShadow = 1
ShadowSoft = 20
QualityShadows = false
Reflection = 0
DebugSun = false
BaseColor = 1,1,1
OrbitStrength = 0.8510638
X = 1,1,1,1
Y = 0.345098,0.666667,0,0.02912
Z = 1,0.666667,0,1
R = 0.0784314,1,0.941176,-0.0194
BackgroundColor = 0.607843,0.866667,0.560784
GradientBackground = 0.3261
CycleColors = true
Cycles = 4.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 8
ColorIterations = 8
Power = 8
Bailout = 3.679245
AlternateVersion = false
RotVector = 1,0,0
RotAngle = 0
Julia = false
JuliaC = 0,0,0
bscale = 1,1,1
boffset = 0,0,0
PineIterAt = 1
Up = -0.1352621,-0.0433757,0.6740923
#endpreset

#preset Dragon
FOV = 0.5
Eye = 1.316464,0.0100845,-0.1731201
Target = -5.784148,-0.2563029,-5.454163
EquiRectangular = false
FocalPlane = 1
Aperture = 0
Gamma = 2
ToneMapping = 3
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 6
AntiAliasScale = 2
Detail = -4.5
DetailAO = -1.47
FudgeFactor = 1
MaxDistance = 10
MaxRaySteps = 833
Dither = 1
NormalBackStep = 1 NotLocked
AO = 0,0,0,1
Specular = 0.1294118
SpecularExp = 43.13726
SpecularMax = 44.89796
SpotLight = 1,1,1,1
SpotLightDir = 0.483871,0
CamLight = 1,1,1,1
CamLightMin = 0
Glow = 1,1,1,1
GlowMax = 96
Fog = 0
HardShadow = 0.5125
ShadowSoft = 10.38961
QualityShadows = false
Reflection = 0
DebugSun = false
BaseColor = 1,1,1
OrbitStrength = 1
X = 1,1,1,0.2972973
Y = 0.345098,0.666667,0,1
Z = 1,0.666667,0,1
R = 0.0784314,1,0.941176,0.1171171
BackgroundColor = 0.2078431,0.254902,0.8666667
GradientBackground = 5
CycleColors = true
Cycles = 10.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 8
ColorIterations = 8
Power = 8
Bailout = 2
AlternateVersion = false
RotVector = 1,0,0
RotAngle = 0
Julia = false
JuliaC = 0,0,0
bscale = 1,1,1
boffset = 0,0,0
PineIterAt = 3
Up = 0.509901,0.0170828,-0.6864475
#endpreset

#preset Bat
FOV = 0.5
Eye = 1.240107,0.0010595,0.4508742
Target = -7.206362,-0.0063251,3.103455
EquiRectangular = false
FocalPlane = 1
Aperture = 0
Gamma = 2
ToneMapping = 3
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 6
AntiAliasScale = 2
Detail = -4.5
DetailAO = -1.47
FudgeFactor = 1
MaxDistance = 10
MaxRaySteps = 833
Dither = 1
NormalBackStep = 1 NotLocked
AO = 0,0,0,1
Specular = 0.1294118
SpecularExp = 43.13726
SpecularMax = 44.89796
SpotLight = 1,1,1,1
SpotLightDir = 0.483871,0
CamLight = 1,1,1,1
CamLightMin = 0
Glow = 1,1,1,1
GlowMax = 96
Fog = 0
HardShadow = 0.5125
ShadowSoft = 10.38961
QualityShadows = false
Reflection = 0
DebugSun = false
BaseColor = 1,1,1
OrbitStrength = 1
X = 1,1,1,0.2972973
Y = 0.345098,0.666667,0,1
Z = 1,0.666667,0,1
R = 0.0784314,1,0.941176,0.1171171
BackgroundColor = 0.2078431,0.254902,0.8666667
GradientBackground = 5
CycleColors = true
Cycles = 10.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 8
ColorIterations = 8
Power = 8
Bailout = 2
AlternateVersion = false
RotVector = 1,0,0
RotAngle = 0
Julia = false
JuliaC = 0,0,0
bscale = 1,1,1
boffset = 0,0,0
PineIterAt = 3
Up = 0.2861,0.0016559,0.9110173
#endpreset

#preset Mask
FOV = 0.6
Eye = -1.052818,-0.0313323,-1.479453
Target = 4.062474,0.0874193,5.735002
EquiRectangular = false
FocalPlane = 1
Aperture = 0
Gamma = 2
ToneMapping = 3
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 6
AntiAliasScale = 2
Detail = -4
DetailAO = -0.6746988
FudgeFactor = 0.5454545
MaxDistance = 10
MaxRaySteps = 300
Dither = 1
NormalBackStep = 1 NotLocked
AO = 0,0,0,0.85185
Specular = 1
SpecularExp = 21.56863
SpecularMax = 30.61225
SpotLight = 1,1,1,1
SpotLightDir = 0.7818182,0.7309091
CamLight = 1,1,1,1.346154
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 52
Fog = 0
HardShadow = 1
ShadowSoft = 20
QualityShadows = false
Reflection = 0
DebugSun = false
BaseColor = 1,1,1
OrbitStrength = 0.8510638
X = 1,1,1,0.2978723
Y = 0.345098,0.666667,0,0.1702128
Z = 1,0.666667,0,1
R = 0.0784314,1,0.941176,-0.0425532
BackgroundColor = 0,0,0
GradientBackground = 0
CycleColors = true
Cycles = 5.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 8
ColorIterations = 8
Power = 8
Bailout = 3.679245
AlternateVersion = false
RotVector = 1,0,0
RotAngle = 0
Julia = false
JuliaC = 0,0,0
bscale = 1,1,1
boffset = 0,0,0
PineIterAt = 2
Up = 0.561967,0.0004085,-0.3984603
#endpreset

#preset Details
FOV = 0.3035714
Eye = -0.1679802,-0.8463627,-0.5674738
Target = -2.568647,4.250884,6.261432
Gamma = 2
ToneMapping = 3
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 6
AntiAliasScale = 2
Detail = -5
DetailAO = -1.47
FudgeFactor = 0.5
MaxRaySteps = 833
Dither = 1
NormalBackStep = 1 NotLocked
AO = 0,0,0,1
CamLight = 1,1,1,1
CamLightMin = 0
Glow = 1,1,1,1
GlowMax = 78
BaseColor = 0.003921569,0.5764706,0.4901961
OrbitStrength = 1
X = 1,1,1,0.3404255
Y = 0.345098,0.666667,0,0.3404255
Z = 1,0.666667,0,0.3478261
R = 0.6078431,0.9960784,1,0.106383
BackgroundColor = 0.2078431,0.254902,0.8666667
GradientBackground = 5
CycleColors = true
Cycles = 10.1
EnableFloor = false NotLocked
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
EquiRectangular = false
FocalPlane = 1
Aperture = 0
MaxDistance = 10
Specular = 0.1294118
SpecularExp = 43.13726
SpecularMax = 44.89796
SpotLight = 1,1,1,1
SpotLightDir = 0.7419355,-0.3548387
Fog = 0
HardShadow = 0.5238095
ShadowSoft = 10.38961
QualityShadows = false
Reflection = 0
DebugSun = false
Iterations = 8
ColorIterations = 8
Power = 8
Bailout = 1.5
AlternateVersion = false
RotVector = 1,0,0
RotAngle = 0
Julia = false
JuliaC = 0,0,0
bscale = 1.5,1.5,1.5
boffset = 0.5,0.5,0.5
PineIterAt = 3
Up = 0.4798738,-0.5717763,0.5954837
#endpreset

#preset Detail2
FOV = 0.5
Eye = 0.4609637,-0.5406448,-0.0739111
Target = -2.475684,2.612179,7.660098
Gamma = 2
ToneMapping = 3
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 6
AntiAliasScale = 2
Detail = -5
DetailAO = -1.47
FudgeFactor = 0.2
MaxRaySteps = 2000
Dither = 1
NormalBackStep = 1 NotLocked
AO = 0,0,0,1
CamLight = 1,1,1,1
CamLightMin = 0
Glow = 1,1,1,1
GlowMax = 78
BaseColor = 0.003921569,0.5764706,0.4901961
OrbitStrength = 1
X = 1,1,1,0.3404255
Y = 0.345098,0.666667,0,0.3404255
Z = 1,0.666667,0,0.3478261
R = 0.6078431,0.9960784,1,0.106383
BackgroundColor = 0.2078431,0.254902,0.8666667
GradientBackground = 5
CycleColors = true
Cycles = 10.1
EnableFloor = false NotLocked
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
EquiRectangular = false
FocalPlane = 1
Aperture = 0
MaxDistance = 10
Specular = 0.1294118
SpecularExp = 43.13726
SpecularMax = 44.89796
SpotLight = 1,1,1,1
SpotLightDir = 0.7419355,-0.3548387
Fog = 0
HardShadow = 0.5238095
ShadowSoft = 10.38961
QualityShadows = false
Reflection = 0
DebugSun = false
Iterations = 6
ColorIterations = 6
Power = 8
Bailout = 1.5
AlternateVersion = false
RotVector = 1,0,0
RotAngle = 0
Julia = false
JuliaC = 0,0,0
bscale = 1,1,1
boffset = 0.5,0.5,0.5
PineIterAt = 2
Up = 0.4197892,0.8380746,-0.1822506
#endpreset

#preset Detail3
FOV = 0.3214286
Eye = 0.2023075,-1.360878,-0.8446037
Target = -1.015039,6.250408,3.659893
Gamma = 2
ToneMapping = 3
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 6
AntiAliasScale = 2
Detail = -4.5
DetailAO = -1.47
FudgeFactor = 1
MaxRaySteps = 833
Dither = 1
NormalBackStep = 1 NotLocked
AO = 0,0,0,1
CamLight = 1,1,1,1.230769
CamLightMin = 0
Glow = 1,1,1,1
GlowMax = 70
BaseColor = 1,1,1
OrbitStrength = 1
X = 1,1,1,0.2972973
Y = 0.345098,0.666667,0,1
Z = 1,0.666667,0,1
R = 0.0784314,1,0.941176,0.1171171
BackgroundColor = 0.2078431,0.254902,0.8666667
GradientBackground = 5
CycleColors = true
Cycles = 10.1
EnableFloor = false NotLocked
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
EquiRectangular = false
FocalPlane = 1
Aperture = 0
MaxDistance = 10
Specular = 0.1294118
SpecularExp = 43.13726
SpecularMax = 44.89796
SpotLight = 1,1,1,1
SpotLightDir = 0.483871,0
Fog = 0
HardShadow = 0.5125
ShadowSoft = 10.38961
QualityShadows = false
Reflection = 0
DebugSun = false
Iterations = 8
ColorIterations = 8
Power = 8
Bailout = 2
AlternateVersion = false
RotVector = 1,0,0
RotAngle = 0
Julia = false
JuliaC = 0,0,0
bscale = 2,2,2
boffset = 1,0,0
PineIterAt = 3
Up = 0.9273798,-0.0010947,0.2524754
#endpreset

#preset FireWithin
FOV = 0.6
Eye = -0.6732608,0,1.127768
Target = 1.390234,0,-7.470531
Gamma = 2
ToneMapping = 3
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 6
AntiAliasScale = 2
Detail = -4
DetailAO = -0.6746988
FudgeFactor = 0.5454545
MaxRaySteps = 300
Dither = 1
NormalBackStep = 1 NotLocked
AO = 0,0,0,0.85185
CamLight = 1,1,1,2
CamLightMin = 0.5079365
Glow = 1,1,1,0
GlowMax = 52
BaseColor = 1,1,1
OrbitStrength = 0.8510638
X = 1,1,1,0.2978723
Y = 0.345098,0.666667,0,0.1702128
Z = 1,0.666667,0,1
R = 0.0784314,1,0.941176,-0.0425532
BackgroundColor = 0,0,0
GradientBackground = 0
CycleColors = true
Cycles = 5.1
EnableFloor = false NotLocked
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
EquiRectangular = false
FocalPlane = 1
Aperture = 0
MaxDistance = 10
Specular = 1
SpecularExp = 21.56863
SpecularMax = 30.61225
SpotLight = 1,1,1,1
SpotLightDir = -0.3870968,0
Fog = 0
HardShadow = 0.047619 NotLocked
ShadowSoft = 20
QualityShadows = false
Reflection = 0
DebugSun = false
Iterations = 8
ColorIterations = 8
Power = 8
Bailout = 3.679245
AlternateVersion = false
RotVector = 1,0,0
RotAngle = 0
Julia = false
JuliaC = 0,0,0
bscale = 1,1,1
boffset = 0,0,0
PineIterAt = 3
Up = -0.6698487,0.0109214,-0.1605024
#endpreset

#preset ChantingShaman
FOV = 0.5535714
Eye = -0.4739225,0,0.8878425
Target = 6.718133,0,-4.239
EquiRectangular = false
FocalPlane = 1
Aperture = 0
Gamma = 2
ToneMapping = 3
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 6
AntiAliasScale = 2
Detail = -4
DetailAO = -0.6746988
FudgeFactor = 0.5454545
MaxDistance = 10
MaxRaySteps = 300
Dither = 1
NormalBackStep = 1 NotLocked
AO = 0,0,0,0.85185
Specular = 1
SpecularExp = 21.56863
SpecularMax = 30.61225
SpotLight = 1,1,1,1
SpotLightDir = -0.2903226,0
CamLight = 1,1,1,1
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 52
Fog = 0
HardShadow = 1 NotLocked
ShadowSoft = 20
QualityShadows = false
Reflection = 0
DebugSun = false
BaseColor = 1,1,1
OrbitStrength = 0.8510638
X = 1,1,1,0.2978723
Y = 0.345098,0.666667,0,0.1702128
Z = 1,0.666667,0,1
R = 0.0784314,1,0.941176,-0.0425532
BackgroundColor = 0,0,0
GradientBackground = 0
CycleColors = true
Cycles = 5.1
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
RotAngle = 0
Julia = false
JuliaC = 0,0,0
bscale = 2,2,2
boffset = 2,0,0
PineIterAt = 1
Up = -0.3997693,0.0080755,-0.5609782
#endpreset

#preset Crown
FOV = 0.15
Eye = 2.669461,-0.0038903,0.2085639
Target = -5.774709,0.0099149,2.808958
Gamma = 2
ToneMapping = 3
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 6
AntiAliasScale = 1.5
Detail = -5
DetailAO = -0.7590361
FudgeFactor = 0.5
MaxRaySteps = 1000
Dither = 1
NormalBackStep = 1 NotLocked
AO = 0,0,0,0.85185
CamLight = 1,1,1,0.9230769
CamLightMin = 0
Glow = 1,1,1,0.0909091
GlowMax = 504
BaseColor = 1,1,1
OrbitStrength = 0.8510638
X = 1,1,1,0.2978723
Y = 0.345098,0.666667,0,0.1702128
Z = 1,0.666667,0,1
R = 0.0784314,1,0.941176,-0.0425532
BackgroundColor = 0,0,0
GradientBackground = 0
CycleColors = true
Cycles = 5.1
EnableFloor = false NotLocked
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
EquiRectangular = false
FocalPlane = 1
Aperture = 0
MaxDistance = 10
Specular = 0.2
SpecularExp = 11.76471
SpecularMax = 20.40816
SpotLight = 1,1,1,1
SpotLightDir = 0.516129,0.1290323
Fog = 0
HardShadow = 0.5238095 NotLocked
ShadowSoft = 10
QualityShadows = false
Reflection = 0 NotLocked
DebugSun = false
Iterations = 8
ColorIterations = 8
Power = 8
Bailout = 3.679245
AlternateVersion = false
RotVector = 1,0,0
RotAngle = 0
Julia = false
JuliaC = 0,0,0
bscale = 1,1,1
boffset = 0.1648352,0,-0.0549451
PineIterAt = 4
Up = 0.2812768,0,0.9133804
#endpreset
