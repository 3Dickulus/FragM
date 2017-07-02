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
uniform int AlternateVersion;  slider[0,1,2]

uniform vec3 RotVector; slider[(0,0,0),(1,1,1),(1,1,1)]

uniform float RotAngle; slider[0.00,0,180]

uniform bool Julia; checkbox[false]
uniform vec3 JuliaC; slider[(-2,-2,-2),(0,0,0),(2,2,2)]

uniform float time;
mat3 rot;
void init() {
	 rot = rotationMatrix3(normalize(RotVector), RotAngle);
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
// Roquen's domain mash up coordinate to invert to infinity
uniform vec3 InvertC; slider[(-5,-5,-5),(0,0,0),(5,5,5)] Locked;
// http://www.fractalforums.com/index.php?topic=16963.msg65053#msg65053
// performs the active c = T(s)
vec3 domainMap(vec3 c)
{
  float s = dot(c,c);
  return c/s + InvertC;
}

// looks exactly like mandelbrot/julia @ power 2
void powN3(inout vec3 z, float zr0, inout float dr) {
	float zo0 = atan( z.z,zr0 );
	float zi0 = atan( z.y,z.x );
	float zr = pow( zr0, Power-1.0 );
	float zo = zo0 * Power;
	float zi = zi0 * Power;
	dr = zr*dr*Power + 1.0;
	zr *= zr0;
	z  = zr*vec3( cos(zo)*cos(zi), cos(zo)*sin(zi), sin(zo) );
}


// Compute the distance from `pos` to the Mandelbox.
float DE(vec3 pos) {
	vec3 z=pos;
	float r;
	float dr=1.0;
	int i=0;
	r=length(z);
	while(r<Bailout && (i<Iterations)) {
		if (AlternateVersion == 0) {
			powN1(z,r,dr);
		} else if (AlternateVersion == 1) {
			powN2(z,r,dr);
		} else if (AlternateVersion == 2) {
			powN3(z,r,dr);
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

#preset Default
FOV = 0.62536
Eye = -0.1736473,0.142806,-1.967487
Target = -0.9352716,-0.3634446,6.846835
EquiRectangular = false
AutoFocus = false
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
DepthToAlpha = false
ShowDepth = false
DepthMagnitude = 1
Detail = -4.5
DetailAO = -2
FudgeFactor = 1
MaxDistance = 10
MaxRaySteps = 1000
Dither = 1
NormalBackStep = 0 NotLocked
AO = 0,0,0,0.85185
Specular = 1
SpecularExp = 16.364
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = -0.5714286,-0.4761905
CamLight = 1,1,1,1.53846
CamLightMin = 0.12121
Glow = 1,1,1,0.43836
GlowMax = 52
Fog = 0
HardShadow = 0.35385
ShadowSoft = 12.5806
QualityShadows = false
Reflection = 0
DebugSun = false
BaseColor = 1,1,1
OrbitStrength = 0.65
X = 1,1,1,1
Y = 0.345098,0.666667,0,0.02912
Z = 1,0.666667,0,1
R = 0.0784314,1,0.941176,-0.0194
BackgroundColor = 0.607843,0.866667,0.560784
GradientBackground = 0.3261
CycleColors = true
Cycles = 1.633333
EnableFloor = true
FloorNormal = 0,1,0
FloorHeight = -1.161
FloorColor = 1,1,1
Iterations = 50
ColorIterations = 16
Power = 2
Bailout = 16
RotVector = 0,1,0
RotAngle = 0
Julia = false
JuliaC = 0.3928571,0.3214286,0
AlternateVersion = 2
InvertC = 0.047619,0,-0.047619 NotLocked
Up = -0.0140738,0.9972658,0.0560619
#endpreset

#preset P3
FOV = 0.62536
Eye = 0.059807,0.1271008,-2.172995
Target = -0.0615594,-0.1992429,6.681801
EquiRectangular = false
AutoFocus = false
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
DepthToAlpha = false
ShowDepth = false
DepthMagnitude = 1
Detail = -4.5
DetailAO = -2
FudgeFactor = 1
MaxDistance = 10
MaxRaySteps = 1000
Dither = 1
NormalBackStep = 0 NotLocked
AO = 0,0,0,0.85185
Specular = 1
SpecularExp = 16.364
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = -0.7222222,-0.5
CamLight = 1,1,1,1.53846
CamLightMin = 0.12121
Glow = 1,1,1,0.43836
GlowMax = 52
Fog = 0
HardShadow = 0.35385
ShadowSoft = 12.5806
QualityShadows = false
Reflection = 0
DebugSun = false
BaseColor = 1,1,1
OrbitStrength = 0.5
X = 1,1,1,1
Y = 0.345098,0.666667,0,0.02912
Z = 1,0.666667,0,1
R = 0.0784314,1,0.941176,-0.0194
BackgroundColor = 0.607843,0.866667,0.560784
GradientBackground = 0.3261
CycleColors = true
Cycles = 1.633333
EnableFloor = true
FloorNormal = 0,1,0
FloorHeight = -1.161
FloorColor = 1,1,1
Iterations = 50
ColorIterations = 16
Power = 3
Bailout = 16
RotVector = 0,1,0
RotAngle = 0
Julia = false
JuliaC = 0.3928571,0.3214286,0
AlternateVersion = 2
InvertC = 0.047619,0,-0.047619 NotLocked
Up = 0.0150665,0.9981406,0.036993
#endpreset

#preset P4
FOV = 0.62536
Eye = 0.059807,0.1271008,-2.172995
Target = -0.0615594,-0.1992429,6.681801
EquiRectangular = false
AutoFocus = false
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
DepthToAlpha = false
ShowDepth = false
DepthMagnitude = 1
Detail = -4.5
DetailAO = -2
FudgeFactor = 1
MaxDistance = 10
MaxRaySteps = 1000
Dither = 1
NormalBackStep = 0 NotLocked
AO = 0,0,0,0.85185
Specular = 1
SpecularExp = 16.364
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = -0.7222222,-0.5
CamLight = 1,1,1,1.53846
CamLightMin = 0.12121
Glow = 1,1,1,0.43836
GlowMax = 52
Fog = 0
HardShadow = 0.35385
ShadowSoft = 12.5806
QualityShadows = false
Reflection = 0
DebugSun = false
BaseColor = 1,1,1
OrbitStrength = 0.5
X = 1,1,1,1
Y = 0.345098,0.666667,0,0.02912
Z = 1,0.666667,0,1
R = 0.0784314,1,0.941176,-0.0194
BackgroundColor = 0.607843,0.866667,0.560784
GradientBackground = 0.3261
CycleColors = true
Cycles = 1.633333
EnableFloor = true
FloorNormal = 0,1,0
FloorHeight = -1.161
FloorColor = 1,1,1
Iterations = 50
ColorIterations = 16
Power = 4
Bailout = 16
RotVector = 0,1,0
RotAngle = 0
Julia = false
JuliaC = 0.3928571,0.3214286,0
AlternateVersion = 2
InvertC = 0.047619,0,-0.047619 NotLocked
Up = 0.0150665,0.9981406,0.036993
#endpreset

#preset P8
FOV = 0.62536
Eye = -2.177085,0.0364717,-0.02467
Target = 6.684417,0.0829674,-0.0403418
EquiRectangular = false
AutoFocus = false
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
DepthToAlpha = false
ShowDepth = false
DepthMagnitude = 1
Detail = -4.5
DetailAO = -2
FudgeFactor = 1
MaxDistance = 10
MaxRaySteps = 1000
Dither = 1
NormalBackStep = 0 NotLocked
AO = 0,0,0,0.85185
Specular = 1
SpecularExp = 16.364
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = -0.7222222,-0.5
CamLight = 1,1,1,1.53846
CamLightMin = 0.12121
Glow = 1,1,1,0.43836
GlowMax = 52
Fog = 0
HardShadow = 0.35385
ShadowSoft = 12.5806
QualityShadows = false
Reflection = 0
DebugSun = false
BaseColor = 1,1,1
OrbitStrength = 0.5
X = 1,1,1,1
Y = 0.345098,0.666667,0,0.02912
Z = 1,0.666667,0,1
R = 0.0784314,1,0.941176,-0.0194
BackgroundColor = 0.607843,0.866667,0.560784
GradientBackground = 0.3261
CycleColors = true
Cycles = 1.633333
EnableFloor = true
FloorNormal = 0,1,0
FloorHeight = -1.161
FloorColor = 1,1,1
Iterations = 50
ColorIterations = 16
Power = 8
Bailout = 16
RotVector = 0,1,0
RotAngle = 0
Julia = false
JuliaC = 0.3928571,0.3214286,0
AlternateVersion = 2
InvertC = 0.047619,0,-0.047619 NotLocked
Up = -0.0052569,0.9988858,-0.0089445
#endpreset
