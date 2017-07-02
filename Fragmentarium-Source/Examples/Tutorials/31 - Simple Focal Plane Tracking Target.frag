#info Mandelbulb Anim Fragment
#info Use with 3Dickulus SplinePath Hack
#info http://www.fractalforums.com/index.php?topic=16405.0
#define providesInit

#group Raytracer
// Sets focal plane to Target location 
//uniform bool AutoFocus; checkbox[false]

#include "DE-Raytracer-v0.9.10.frag"
#include "MathUtils.frag"

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

#group Translation
uniform vec3 TransVector; slider[(-1,-1,-1),(1,0,0),(1,1,1)]
uniform float TransSpeed; slider[0,0,1]
uniform float ImpulseStrength; slider[0,0,10]
uniform float ImpulseRate; slider[0,0,10]
uniform float ImpulseOffset; slider[0,0,10]
uniform vec3 TRotVector; slider[(-1,-1,-1),(1,0,0),(1,1,1)]
uniform float TRotSpeed; slider[0,0,45]

mat3 rot;
uniform float time;

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



uniform bool Julia; checkbox[false]
uniform vec3 JuliaC; slider[(-2,-2,-2),(0,0,0),(2,2,2)]

// Compute the distance from `pos` to the Mandelbox.
float DE(vec3 pos) {
	vec3 z=pos;
	float r;
	float dr=1.0;
	int i=0;

	if (TRotSpeed>0.0) {
		rot=rotationMatrix3(normalize(TRotVector),TRotSpeed*time);
		z*=rot;
	}
	if (TransSpeed>0.0) {
		z+=normalize(TransVector)*time*TransSpeed*10.0;
	}
	if (ImpulseStrength>0.0) {
		z+=normalize(TransVector)*(0.8+sin(time*ImpulseRate+ImpulseOffset))*ImpulseStrength;
	}

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

float dummy(vec3 p){
p*=time;
return time;
}

#preset Default
FOV = 0.62536
Eye = 1.65826,-1.22975,0.277736
Target = -5.2432,4.25801,-0.607125
Up = 0,1,0
EquiRectangular = false
AutoFocus = true
FocalPlane = 1
Aperture = 0.05
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
BoundingSphere = 10
Dither = 0.51754
NormalBackStep = 1
AO = 0,0,0,0.85185
Specular = 1.6456
SpecularExp = 16.364
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = -0.22666,0.5
CamLight = 1,1,1,1.53846
CamLightMin = 0.12121
Glow = 1,1,1,0.43836
GlowMax = 52
Fog = 0
HardShadow = 0.35385
ShadowSoft = 12.5806
Reflection = 0
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
EnableFloor = true
FloorNormal = 0,1,0
FloorHeight = -2
FloorColor = 1,1,1
Iterations = 12
ColorIterations = 8
Power = 8
Bailout = 6.279
AlternateVersion = true
RotVector = 1,1,1
RotAngle = 0
TransVector = 1,0,0
TransSpeed = 0
ImpulseStrength = 0
ImpulseRate = 0
ImpulseOffset = 0
TRotVector = 1,0,0
TRotSpeed = 0
Julia = false
JuliaC = 0,0,0
#endpreset


#preset KeyFrame.001
FOV = 0.62536
Eye = 2,0,0
Target = 1,0,0
Up = 0,1,0
#endpreset

#preset KeyFrame.002
FOV = 0.62536
Eye = 0.73087,0.0286388,1.42303
Target = 0.31975,-0.0253,0.62258
Up = 0,1,0
#endpreset

#preset KeyFrame.003
FOV = 0.62536
Eye = 0.034575,-0.00331298,1.59962
Target = 0.0,0.0,0.0
Up = -0.368,1,0
#endpreset

#preset KeyFrame.004
FOV = 0.62536
Eye = 0.034575,-0.00331298,1.59962
Target = 0.05491,-0.00217,0.99997
Up = 0.17544,1,0
#endpreset

#preset KeyFrame.005
FOV = 0.62536
Eye = 0.0244074,-0.00388447,1.89945
Target = 0.05491,-0.00217,0.99997
Up = 0,1,0
#endpreset

#preset KeyFrame.006
FOV = 0.62536
Eye = 0.0108507,-0.00464646,2.29922
Target = 0.05491,-0.00217,0.99997
Up = 0,1,0
#endpreset

#preset KeyFrame.007
FOV = 0.62536
Eye = 0.0108507,-0.00464646,2.29922
Target = 0.02745,-0.0018,0.454545
Up = 0,1,0
#endpreset

#preset KeyFrame.008
FOV = 0.62536
Eye = 0.0108507,-0.00464646,2.29922
Target = 0,0,0
Up = 0,1,0
#endpreset

#preset KeyFrame.009
FOV = 0.62536
Eye = 0.0108507,-0.00464646,2.29922
Target = 0.05491,-0.00217,0.99997
Up = 0,1,0
#endpreset

#preset KeyFrame.010
FOV = 0.62536
Eye = 0.0108507,-0.00464646,2.29922
Target = 0.05491,-0.00217,0.99997
Up = 0,1,0
#endpreset

