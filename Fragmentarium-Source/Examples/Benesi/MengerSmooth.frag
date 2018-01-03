#info Menger Distance Estimator.
#define providesInit
#include "MathUtils.frag"
#include "DE-Raytracer.frag"
//#include "Fast-Raytracer.frag"
#group Menger
// Based on Knighty's Kaleidoscopic IFS 3D Fractals, described here:
// http://www.fractalforums.com/3d-fractal-generation/kaleidoscopic-%28escape-time-ifs%29/
//float sr12=sqrt(1.5);   // sqrt(3/2)
//float magic=.955316618124509278164;
float sr32=1.2247448713915890491;   // sqrt(3/2)
// Scale parameter. A perfect Menger is 3.0
uniform float Scale; slider[0.00,3.0,4.00]
uniform float s; slider[0.000,0.005,0.100]

// Total Sphere Iteration tranforms the coordinate system for the whole iteration, starting with cube to sphere, ending with sphere to cube
uniform int Total_Sphere_Iter; slider[0,11,20]
// Applies cube to sphere then sphere to cube on first section
uniform int SphereiterationT1; slider[0,12,20]
uniform int SphereiterationT2; slider[0,12,20]
uniform int SphereiterationT3; slider[0,12,20]
// Unsphere starts out with the unspheric formula, then ends with spheric (returning coordinates too normal.. well.. normal weird...
uniform int Total_unSphere_Iter; slider[0,11,20]
uniform int unSphereiterationT1; slider[0,12,20]
uniform int unSphereiterationT2; slider[0,12,20]
uniform int unSphereiterationT3; slider[0,12,20]
uniform bool Sphere;checkbox[false]
uniform bool HoleSphere; checkbox[false]
uniform bool unSphere;checkbox[false]
uniform bool Octalize;checkbox[false]
uniform bool unOctalize;checkbox[false]
uniform vec3 RotVector; slider[(0,0,0),(1,1,1),(1,1,1)]
uniform float RotAngle; slider[0.00,0,180]
// Scaling center
uniform vec3 Offset; slider[(0,0,0),(1,1,1),(1,1,1)]
uniform bool tetralixinatorer;checkbox[false]
uniform bool test;checkbox[false]
mat3 rot;

void init() {
	rot = rotationMatrix3(normalize(RotVector), RotAngle);
}

// Number of fractal iterations.
uniform int Iterations;  slider[0,8,100]
uniform int ColorIterations;  slider[0,8,100]
//float sr13=sqrt(1./3.);
//float sr23=sqrt (2./3.);
float magic=.955316618124509278164;
void tetralixinator (inout vec3 z) {
	//rotate y and z by sqrt(1/3) and sqrt(2/3)
	///float r1=z.y*sr23-z.z*sr13;

	///	if (test)	{z.z=z.y*sr13+z.z*sr23;
		//				z.y=r1;}

  // float r1=sqrt(z.y*z.y+z.z*z.z);
	//float r2=length(z);
	//float rCyz=abs(atan(z.z/z.y));
	//float rCxyz=
//	if  (rCyz>(2.*magic)) {
	//	rCyz=tan(rCyz-2.*magic);
	//	rCyz=sqrt(1.+rCyz*rCyz);
//	} else if (rCyz>magic) {
	///	rCyz=tan(abs(2.*magic-rCyz));
//		rCyz=sqrt(1.+rCyz*rCyz);
//	} else {
	//	rCyz=tan(rCyz);
	//	rCyz=sqrt(1.+rCyz*rCyz);
//	}
	//z.yz/=rCyz;
		if(z.x+z.y<0.0) z.xy = -z.yx;
		if(z.x+z.z<0.0) z.xz = -z.zx;
		if(z.y+z.z<0.0)z.zy = -z.yz;

		z=z*2.0-vec3(1.,1.,1.);
//	if (test) {	r1=z.y*sr23+z.z*sr13;
//	z.z=-z.y*sr13+z.z*sr23;
//	z.y=r1;}
}

void Octalizer (inout vec3 z) {
	
float rCxyz= (abs(z.z)+abs(z.y))/abs(z.x);
if (rCxyz<1.) {rCxyz=sqrt(rCxyz+1.);} else {rCxyz=sqrt(1./rCxyz+1.);}
z.xyz*=rCxyz/sr32;
float rCyz= abs(z.y)/abs(z.z);
//float rCxyz= sqrt(z.z*z.z+z.y*z.y)/sqrt(z.x*z.x);
if (rCyz<1.) {rCyz=sqrt(rCyz+1.);} else {rCyz=sqrt(1./rCyz+1.);}
z.yz*=rCyz;
}
void unOctalizer (inout vec3 z) {
	
float rCyz= abs(z.y)/abs(z.z);
if (rCyz<1.) {rCyz=1./sqrt(rCyz+1.);} else {rCyz=1./sqrt(1./rCyz+1.);}
z.yz*=rCyz;
//float rCxyz= sqrt(z.z*z.z+z.y*z.y)/sqrt(z.x*z.x);
float rCxyz= (abs(z.z)+abs(z.y))/abs(z.x);
if (rCxyz<1.) {rCxyz=1./sqrt(rCxyz+1.);} else {rCxyz=1./sqrt(1./rCxyz+1.);}

z.xyz*=rCxyz/sr32;
}



void Spheric (inout vec3 z) {
//  changes cubes centered at origin
//  with faces centered at x, y, and
//  z axes into spheres of radius ????
//  1/cos(atan(x)) = sqrt(x^2+1)  1/sin(atan(x))= sqrt(x^2+1)/x

// CAN Switch back from Sphere of radius (x^2+y^2+z^2) to cube with face 
// centers at x, y, and z.. next
//float r=length(z);
float rCyz= (z.y*z.y)/(z.z*z.z);
float rCxyz= (z.y*z.y+z.z*z.z)/(z.x*z.x);
if (rCyz<1.) {rCyz=sqrt(rCyz+1.);} else {rCyz=sqrt(1./rCyz+1.);}
if (rCxyz<1.) {rCxyz=sqrt(rCxyz+1.);} else {rCxyz=sqrt(1./rCxyz+1.);}

z.yz*=rCyz;  //  y and z divided by cosine of angle between them
 				//  if y<z, or sin if z<y
z*=rCxyz/sr32;  //  x,y, and z divided by cosine of angle between x and
// |y^2+z^2| if x> |magyz|, else divided by sin of angle...
}
void unSpheric (inout vec3 z) {
	
float rCyz= (z.y*z.y)/(z.z*z.z);
if (rCyz<1.) {rCyz=1./sqrt(rCyz+1.);} else {rCyz=1./sqrt(1./rCyz+1.);}
z.yz*=rCyz;
float rCxyz= (z.y*z.y+z.z*z.z)/(z.x*z.x);
if (rCxyz<1.) {rCxyz=1./sqrt(rCxyz+1.);} else {rCxyz=1./sqrt(1./rCxyz+1.);}
z.xyz*=rCxyz*sr32; 
}

float DE(vec3 z)
{
	float t=9999.0;
	 float sc=Scale;
	 float sc1=sc-1.0;
	float sc2=sc1/sc;
	vec3 C=vec3(1.0,1.0,.5);
	float w=1.;
	int n = 0;
	float r=length(z);
	if(Octalize) {Octalizer(z);}
	//if(Sphere) {Spheric(z);}
	if(unOctalize) unOctalizer(z);
	//if(Sphere) {Spheric(z);}
	if(unSphere) {unSpheric(z);}
		if(Sphere || HoleSphere) Spheric(z);
	if (tetralixinatorer) tetralixinator(z);
	while (n < Iterations) {
		z*=rot;
		if (n==Total_Sphere_Iter) {Spheric(z);}
		if (n==Total_unSphere_Iter) {unSpheric(z);}
		z = vec3(sqrt(z.x*z.x+s),sqrt(z.y*z.y+s),sqrt(z.z*z.z+s));
	
		//z = rot *z;
		if (n==SphereiterationT1-1) {Spheric(z);}
		if (n==unSphereiterationT1-1 || HoleSphere) {unSpheric(z);}
		//if (HoleSphere) {unSpheric(z);}
		t=z.x-z.y;  t= .5*(t-sqrt(t*t+s));
		z.x=z.x-t;	z.y=z.y+t;
		
		t=z.x-z.z; t= 0.5*(t-sqrt(t*t+s));
  		z.x=z.x-t;	 z.z= z.z+t;
	
		t=z.y-z.z;  t= 0.5*(t-sqrt(t*t+s));
  		z.y=z.y-t;  z.z= z.z+t;
		if (n==SphereiterationT1-1) {unSpheric(z);}
		if (n==unSphereiterationT1-1 || HoleSphere) {Spheric(z);}
	
		if (n==SphereiterationT2-1) {Spheric(z);}
		if (n==unSphereiterationT2-1) {unSpheric(z);}
		z.z = z.z-C.z*sc2;
		z.z=-sqrt(z.z*z.z+s);
		z.z=z.z+C.z*sc2;
		if (n==SphereiterationT2-1) {unSpheric(z);}
		if (n==unSphereiterationT2-1) {unSpheric(z);}
		if (n==SphereiterationT3-1) {Spheric(z);}
		if (n==unSphereiterationT3-1) {unSpheric(z);}
		z.x=sc*z.x-C.x*sc1;
		z.y=sc*z.y-C.y*sc1;
		z.z=sc*z.z;
		if (n==SphereiterationT3-1) {unSpheric(z);}
		if (n==unSphereiterationT3-1) {Spheric(z);}
	
		w=w*sc;
		if (n<ColorIterations) orbitTrap = min(orbitTrap, (vec4(abs(z),dot(z,z))));
		if (n==Total_Sphere_Iter) {unSpheric(z);}
		if (n==Total_unSphere_Iter) {Spheric(z);}
		n++;
	}
	return abs(length(z)-0.0 ) /w;

	//return abs(length(z)-0.0 ) * pow(Scale, float(-n));
}

#preset Default
FOV = 0.4
Eye = 2.803797,2.012923,-2.72889
Target = -3.568471,-2.561902,3.473134
Up = -0.1934184,0.8739289,0.4459123
EquiRectangular = false
AutoFocus = false
FocalPlane = 1
Aperture = 0
Gamma = 2
ToneMapping = 4
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
DepthToAlpha = false
ShowDepth = false
DepthMagnitude = 1
Detail = -2.3
DetailAO = -0.5
FudgeFactor = 1
MaxDistance = 1000
MaxRaySteps = 56
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,0.7
Specular = 0.4
SpecularExp = 16
SpecularMax = 10
SpotLight = 1,1,1,0.4
SpotLightDir = 0.1,0.1
CamLight = 1,1,1,1
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 20
Fog = 0
HardShadow = 0
ShadowSoft = 2
QualityShadows = false
Reflection = 0
DebugSun = false
BaseColor = 1,1,1
OrbitStrength = 0
X = 0.5,0.6,0.6,0.7
Y = 1,0.6,0,0.4
Z = 0.8,0.78,1,0.5
R = 0.4,0.7,1,0.12
BackgroundColor = 0.6,0.6,0.45
GradientBackground = 0.3
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,1
FloorHeight = 0
FloorColor = 1,1,1
Scale = 3
s = 0.005
Total_Sphere_Iter = 11
SphereiterationT1 = 12
SphereiterationT2 = 12
SphereiterationT3 = 12
Total_unSphere_Iter = 11
unSphereiterationT1 = 12
unSphereiterationT2 = 12
unSphereiterationT3 = 12
Sphere = false
HoleSphere = false
unSphere = false
Octalize = false
unOctalize = false
RotVector = 1,1,1
RotAngle = 0
Offset = 1,1,1
tetralixinatorer = false
test = false
Iterations = 8
ColorIterations = 8
#endpreset

#preset Balls
FOV = 0.4
Eye = -1.926871,-2.362987,-2.728804
Target = -1.369221,-1.594053,-1.860413
DepthToAlpha = false
Detail = -3
DetailAO = -0.5
FudgeFactor = 0.46
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,0.7
SpecularExp = 31.37255
SpotLight = 0.1098039,1,0.8980392,0.5319149
SpotLightDir = -0.2,1
CamLight = 0.7764706,0.7764706,0.7764706,0.875
CamLightMin = 0.0645161
Glow = 1,1,1,0.0144928
GlowMax = 17
Fog = 0
HardShadow = 0
ShadowSoft = 2
Reflection = 0
BaseColor = 1,1,1
OrbitStrength = 0.3571429
X = 0.3098039,0.6,0.6784314,0.7
Y = 0.05490196,0.7803922,1,0.4
Z = 0.7019608,1,0.8392157,0.5
R = 0.827451,0.654902,1,0.0617284
BackgroundColor = 0.01960784,0.01176471,0.05882353
GradientBackground = 0.3
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
depthMag = 1
ShowDepth = false
AntiAlias = 1
MaxRaySteps = 111
BoundingSphere = 2
Specular = 3.2
Scale = 3
s = 0.0025641
Sphere = false
unSphere = false
RotVector = 1,0,0
RotAngle = 0
Offset = 1,1,1
Iterations = 8
ColorIterations = 8
Octalize = false
HoleSphere = false
SphereiterationT1 = 3
SphereiterationT2 = 3
SphereiterationT3 = 3
Total_Sphere_Iter = 3
unSphereiterationT1 = 11
unSphereiterationT2 = 11
unSphereiterationT3 = 4
Total_unSphere_Iter = 11
Up = 0.3343058,0.3954327,-0.5648231
#endpreset

#preset Water1
FOV = 0.4
Eye = -1.983899,-1.239903,-2.429089
Target = -1.103384,-0.754737,-1.62557
Up = 0.3417636,0.3532848,-0.5878269
DepthToAlpha = false
depthMag = 1
ShowDepth = false
AntiAlias = 1
Detail = -2.5
DetailAO = -0.5
FudgeFactor = 0.45
MaxRaySteps = 111
BoundingSphere = 2
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,0.7
Specular = 3.2
SpecularExp = 31.37255
SpotLight = 0.1098039,1,0.8980392,0.5319149
SpotLightDir = -0.2,1
CamLight = 0.7764706,0.7764706,0.7764706,0.875
CamLightMin = 0.0645161
Glow = 1,1,1,0.0144928
GlowMax = 17
Fog = 0
HardShadow = 0
ShadowSoft = 2
Reflection = 0
BaseColor = 1,1,1
OrbitStrength = 0.3571429
X = 0.3098039,0.6,0.6784314,0.7
Y = 0.05490196,0.7803922,1,0.4
Z = 0.7019608,1,0.8392157,0.5
R = 0.827451,0.654902,1,0.0617284
BackgroundColor = 0.01960784,0.01176471,0.05882353
GradientBackground = 0.3
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Scale = 2.117647
s = 0.0004
Total_Sphere_Iter = 2
SphereiterationT1 = 1
SphereiterationT2 = 3
SphereiterationT3 = 4
Total_unSphere_Iter = 10
unSphereiterationT1 = 9
unSphereiterationT2 = 7
unSphereiterationT3 = 7
Sphere = false
HoleSphere = false
unSphere = true
Octalize = true
RotVector = 1,0,0.0952381
RotAngle = 180
Offset = 1,1,1
Iterations = 8
ColorIterations = 8
#endpreset

#preset Water2
FOV = 0.4
Eye = -1.983899,-1.239903,-2.429089
Target = -1.103384,-0.754737,-1.62557
DepthToAlpha = false
depthMag = 1
ShowDepth = false
AntiAlias = 1
Detail = -3
DetailAO = -0.5
FudgeFactor = 0.45
MaxRaySteps = 111
BoundingSphere = 2
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,0.7
Specular = 3.2
SpecularExp = 31.37255
SpotLight = 0.1098039,1,0.8980392,0.5319149
SpotLightDir = -0.2,1
CamLight = 0.7764706,0.7764706,0.7764706,0.875
CamLightMin = 0.0645161
Glow = 1,1,1,0.0144928
GlowMax = 17
Fog = 0
HardShadow = 0
ShadowSoft = 2
Reflection = 0
BaseColor = 1,1,1
OrbitStrength = 0.3571429
X = 0.3098039,0.6,0.6784314,0.7
Y = 0.05490196,0.7803922,1,0.4
Z = 0.7019608,1,0.8392157,0.5
R = 0.827451,0.654902,1,0.0617284
BackgroundColor = 0.01960784,0.01176471,0.05882353
GradientBackground = 0.3
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Scale = 2.070588
s = 0.0019048
Total_Sphere_Iter = 11
SphereiterationT1 = 11
SphereiterationT2 = 4
SphereiterationT3 = 11
Total_unSphere_Iter = 10
unSphereiterationT1 = 1
unSphereiterationT2 = 6
unSphereiterationT3 = 2
Sphere = false
HoleSphere = false
unSphere = true
Octalize = false
RotVector = 1,0,0
RotAngle = 0
Offset = 1,1,1
Iterations = 8
ColorIterations = 8
Up = 0.3417636,0.3532848,-0.5878269
#endpreset

#preset Wetontop
FOV = 0.4
Eye = -2.32518,-3.450239,-2.358
Target = -1.727264,-2.529349,-1.686589
DepthToAlpha = false
Detail = -3
DetailAO = -0.5
FudgeFactor = 0.45
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,0.7
SpecularExp = 31.37255
SpotLight = 0.1098039,1,0.8980392,0.5319149
SpotLightDir = -0.2,1
CamLight = 0.7764706,0.7764706,0.7764706,0.875
CamLightMin = 0.0645161
Glow = 1,1,1,0.0144928
GlowMax = 17
Fog = 0
HardShadow = 0
ShadowSoft = 2
Reflection = 0
BaseColor = 1,1,1
OrbitStrength = 0.3571429
X = 0.3098039,0.6,0.6784314,0.7
Y = 0.05490196,0.7803922,1,0.4
Z = 0.7019608,1,0.8392157,0.5
R = 0.827451,0.654902,1,0.0617284
BackgroundColor = 0.01960784,0.01176471,0.05882353
GradientBackground = 0.3
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
depthMag = 1
ShowDepth = false
AntiAlias = 1
MaxRaySteps = 111
BoundingSphere = 2
Specular = 3.2
Scale = 1.938144
s = 0
Sphere = false
unSphere = false
RotVector = 0,0,1
RotAngle = 180
Offset = 1,1,1
Iterations = 8
ColorIterations = 8
Octalize = false
HoleSphere = false
SphereiterationT1 = 4
SphereiterationT2 = 11
SphereiterationT3 = 4
Total_Sphere_Iter = 11
unSphereiterationT1 = 1
unSphereiterationT2 = 0
unSphereiterationT3 = 11
Total_unSphere_Iter = 10
Up = 0.2761576,0.2952769,-0.6509234
#endpreset

#preset GoodLight
FOV = 0.4
Eye = -1.580232,-1.885012,-2.189006
Target = -1.022582,-1.116078,-1.320615
DepthToAlpha = false
Detail = -3
DetailAO = -0.5
FudgeFactor = 0.4
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,0.7
SpecularExp = 31.37255
SpotLight = 0.1098039,1,0.8980392,0.5319149
SpotLightDir = -0.2,1
CamLight = 0.7764706,0.7764706,0.7764706,0.875
CamLightMin = 0.0645161
Glow = 1,1,1,0.0144928
GlowMax = 17
Fog = 0
HardShadow = 0
ShadowSoft = 2
Reflection = 0
BaseColor = 1,1,1
OrbitStrength = 0.3571429
X = 0.3098039,0.6,0.6784314,0.7
Y = 0.05490196,0.7803922,1,0.4
Z = 0.7019608,1,0.8392157,0.5
R = 0.827451,0.654902,1,0.0617284
BackgroundColor = 0.01960784,0.01176471,0.05882353
GradientBackground = 0.3
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
depthMag = 1
ShowDepth = false
AntiAlias = 1
MaxRaySteps = 111
BoundingSphere = 2
Specular = 3.2
Scale = 3
s = 0
Sphere = false
unSphere = false
RotVector = 1,1,1
RotAngle = 0
Offset = 1,1,1
Iterations = 8
ColorIterations = 8
Octalize = false
HoleSphere = false
Up = 0.3343059,0.3954328,-0.564823
SphereiterationT1 = 0
SphereiterationT2 = 0
#endpreset
