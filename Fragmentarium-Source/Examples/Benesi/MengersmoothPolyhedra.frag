#info Menger Distance Estimator.
#define providesInit
#include "MathUtils.frag"
//#include "DE-Raytracer.frag"
#include "Fast-Raytracer.frag"
#group Menger
// Based on Knighty's Kaleidoscopic IFS 3D Fractals, described here:
// http://www.fractalforums.com/3d-fractal-generation/kaleidoscopic-%28escape-time-ifs%29/

const float pi=2.*1.570796326794897;
const float pi2=2.*pi;
const float sr13=sqrt(1./3.);
const float sr23=sqrt (2./3.);
const float sr83=sqrt(8/3);
const float sr3=1./sr13;
const float sr2=sqrt(2);
uniform float Scale; slider[0.00,3.0,4.00]
uniform float s; slider[0.000,0.000,0.100]
uniform bool Sphere;checkbox[false]
uniform bool HoleSphere; checkbox[false]
uniform bool unSphere;checkbox[false]
// Scaling center
//uniform vec3 Offset; slider[(0,0,0),(1,1,1),(1,1,1)]
uniform bool TotallyTubular;checkbox[false]

uniform bool polygonate;checkbox[false]
//set both sides to 3 for a tetra hedron, 4 for a cube, and that's it for the Platonics from this set
uniform float sides;slider[2.0,3.0,15.0]
//use with sphere or holespher with polygonate
uniform bool polyhedronate;checkbox[false]
uniform float sides2;slider[2.0,3.0,15.0]

uniform bool test;checkbox[false]
uniform bool test2;checkbox[false]
uniform bool test3;checkbox[false]

uniform bool gravity;checkbox[false]
uniform float gravitation;slider[-3,0,3]

mat3 rot;
uniform vec3 RotVector;slider[(0,0,0),(1,1,1),(1,1,1)]
uniform float RotAngle;slider[-360,0,360]
void init() {
	rot = rotationMatrix3(normalize(RotVector), RotAngle);
}

// Number of fractal iterations.
uniform int Iterations;  slider[0,8,100]
uniform int ColorIterations;  slider[0,8,100]



void Spheric (inout vec3 z) {

float rCyz= (z.y*z.y)/(z.z*z.z);
float rCxyz= (z.y*z.y+z.z*z.z)/(z.x*z.x);
if (rCyz<1.) {rCyz=sqrt(rCyz+1.);} else {rCyz=sqrt(1./rCyz+1.);}
if (rCxyz<1.) {rCxyz=sqrt(rCxyz+1.);} else {rCxyz=sqrt(1./rCxyz+1.);}

z.yz*=rCyz;  
z*=rCxyz;  
}
void unSpheric (inout vec3 z) {
	
float rCyz= (z.y*z.y)/(z.z*z.z);
if (rCyz<1.) {rCyz=1./sqrt(rCyz+1.);} else {rCyz=1./sqrt(1./rCyz+1.);}
z.yz*=rCyz;
float rCxyz= (z.y*z.y+z.z*z.z)/(z.x*z.x);
if (rCxyz<1.) {rCxyz=1./sqrt(rCxyz+1.);} else {rCxyz=1./sqrt(1./rCxyz+1.);}
z.xyz*=rCxyz; 
}

void Tubular (inout vec3 z) {
float rCyz= (z.y*z.y)/(z.z*z.z);
if (rCyz<1.) {rCyz=sqrt(rCyz+1.);}
	 else {rCyz=sqrt(1./rCyz+1.);}
z.yz*=rCyz;  
}

void polygonator (inout vec3 z) {
// use with tubular or (polyhedronator & spheric)	
	float rCyz=(atan(z.z,z.y));
	float i=1.;
	while (rCyz>pi/sides && i<sides) {rCyz-=pi2/sides; i++;}
	 while (rCyz<-pi/sides && i<sides) {rCyz+=pi2/sides;i++;}
	z.yz*=cos(rCyz);
}
uniform float scale;slider[0,1.5,3]
void polyhedronator (inout vec3 z) {  
	if (test2) {
		if (test3) {z.x*=sqrt(2.);}
		else {z.x*=scale;}
	}
		float i=1.;
		z.yz*=tan(pi/sides2);
		float rCxyz= atan(sqrt(z.y*z.y+z.z*z.z),z.x);
		i=1.;
		while (rCxyz>pi/sides2 && i<sides2) {rCxyz-=pi2/sides2; i++;}
		 while (rCxyz<-pi/sides2 && i<sides2) {rCxyz+=pi2/sides2;i++;}
		z*=cos(rCxyz);
}

uniform bool test4;checkbox[false]
void fullpolytest (inout vec3 z) {  
	

		float  rCyz=(atan(z.z,z.y));
		float i=1.;
	if (test3) {
		while (rCyz>pi/sides2 && i<sides2) {rCyz-=pi2/sides2; i++;}
	 	while (rCyz<-pi/sides2 && i<sides2) {rCyz+=pi2/sides2;i++;}
	} else {
		while (rCyz>pi/sides && i<sides) {rCyz-=pi2/sides; i++;}
	 	while (rCyz<-pi/sides && i<sides) {rCyz+=pi2/sides;i++;}
	}
		z.yz*=cos(rCyz);
		i=1.;
		if (test2) {
		//float rCxyz=tan(pi/(sides2));
			if (test4) {
 				//z.x/=scale;
				z.yz*=scale;
			} else {
				//z.x/=sr2;
				z.yz*=sr2;
			}
		}
		float  rCxyz= atan(sqrt(z.y*z.y+z.z*z.z),z.x);
		i=1.;
	//float sides3=sqrt(sides2);
		while (rCxyz>pi/sides2 && i<sides2) {rCxyz-=pi2/sides2; i++;}
		 while (rCxyz<-pi/sides2 && i<sides2) {rCxyz+=pi2/sides2;i++;}
		z*=cos(rCxyz);
		
		rCyz= (z.y*z.y)/(z.z*z.z);
		rCxyz= (z.y*z.y+z.z*z.z)/(z.x*z.x);
		if (rCyz<1.) {rCyz=sqrt(rCyz+1.);} else {rCyz=sqrt(1./rCyz+1.);}
		if (rCxyz<1.) {rCxyz=sqrt(rCxyz+1.);} else {rCxyz=sqrt(1./rCxyz+1.);}

		z.yz*=rCyz;  
		z*=rCxyz;  
}


void unpolygonator (inout vec3 z) {
// use with tubular or (polyhedronator & spheric)	
	float rCyz=(atan(z.z,z.y));
	float i=1.;
	while (rCyz>pi/sides && i<sides) {rCyz-=pi2/sides; i++;}
	 while (rCyz<-pi/sides && i<sides) {rCyz+=pi2/sides;i++;}
	z.yz/=cos(rCyz);
}

void unpolyhedronator (inout vec3 z) {  
		float i=1.;
		float rCxyz= atan(sqrt(z.y*z.y+z.z*z.z),z.x);
		i=1.;
		while (rCxyz>pi/sides2 && i<sides2) {rCxyz-=pi2/sides2; i++;}
		 while (rCxyz<-pi/sides2 && i<sides2) {rCxyz+=pi2/sides2;i++;}
		z/=cos(rCxyz);
}


void gravitate (inout vec3 z) {
	float yz=sqrt(z.x*z.x+z.y*z.y+z.z*z.z);
	float tanyz=(gravitation)/yz;
	tanyz=sr13-sr23*tanyz;
	tanyz*=tanyz;
	yz=sqrt(tanyz+2./3.);
	z.xyz*=yz;
}




float DE(vec3 z)
{
	float t=9999.0;
	 float sc=Scale;
	 float sc1=sc-1.0;
	float sc2=sc1/sc;
	vec3 C=vec3(1.0,1.0,.5);
	float w=1.;
	int n = 1;
	float r=length(z);
	
	//z*=rot;
	if(polygonate) {polygonator(z);}
	//z*=rot;
	if (polyhedronate) polyhedronator(z);
	//z*=rot;
	if(TotallyTubular) Tubular(z);
	z*=rot;
	if(Sphere || HoleSphere) Spheric(z);
	if(unSphere) {unSpheric(z);}
	if (gravity) gravitate(z);
	if (test) {fullpolytest(z);}
	while (n <= Iterations) {
		
		
		z = vec3(sqrt(z.x*z.x+s),sqrt(z.y*z.y+s),sqrt(z.z*z.z+s));
	
		
		if (HoleSphere) {unSpheric(z);}
		//if (HoleSphere) {unSpheric(z);}
		t=z.x-z.y;  t= .5*(t-sqrt(t*t+s));
		z.x=z.x-t;	z.y=z.y+t;
		
		t=z.x-z.z; t= 0.5*(t-sqrt(t*t+s));
  		z.x=z.x-t;	 z.z= z.z+t;
	
		t=z.y-z.z;  t= 0.5*(t-sqrt(t*t+s));
  		z.y=z.y-t;  z.z= z.z+t;
		
		if (HoleSphere) {Spheric(z);}
	
		z.z = z.z-C.z*sc2;
		z.z=-sqrt(z.z*z.z+s);
		z.z=z.z+C.z*sc2;
		
		z.x=sc*z.x-C.x*sc1;
		z.y=sc*z.y-C.y*sc1;
		z.z=sc*z.z;
		
		w=w*sc;
		if (n<=ColorIterations) orbitTrap = min(orbitTrap, (vec4(abs(z),dot(z,z))));
		n++;
	}
	return abs(length(z)-0.0 ) /w;

	
}

#preset Default
FOV = 0.4
Eye = -2.833204,-2.853798,-2.166122
Target = -2.059039,-2.04626,-1.529816
DepthToAlpha = false
depthMag = 1
ShowDepth = false
AntiAlias = 1
Detail = -3
DetailAO = -0.5
FudgeFactor = 0.46
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
OrbitStrength = 1
X = 0,0.2901961,0.5607843,0.5942029
Y = 0,0.7176471,0.7294118,-0.2173913
Z = 0.1490196,1,0,-0.0434783
R = 0.1254902,0.5254902,0.003921569,0.5
BackgroundColor = 0.01960784,0.01176471,0.05882353
GradientBackground = 0.3
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Scale = 3
s = 0
Sphere = true
HoleSphere = false
unSphere = false
Offset = 1,1,1
TotallyTubular = false
tetralixinatorer = false
test = false
conic = false
polygonate = false
scale = 0
sides = 3
sides2 = 3
test2 = false
RotVector = 1,0,0
RotAngle = 0
Iterations = 8
ColorIterations = 8
funn = true
coneheight = 3
Up = 0.1391576,0.3795226,-0.6509612
#endpreset

#preset TetrahedronImport
FOV = 0.4
Eye = 3.202082,0,3.202082
Target = 0,0,0
DepthToAlpha = false
Detail = -3
DetailAO = -0.63
FudgeFactor = 0.7
Dither = 0.22807
NormalBackStep = 1
AO = 0,0,0,0.96721
SpecularExp = 18.8
SpotLight = 1,1,1,0.17391
SpotLightDir = 0.31428,0.1
CamLight = 1,1,1,1.41936
CamLightMin = 0.15294
Glow = 0.835294,0.0784314,0.0784314,0
GlowMax = 20
Fog = 0
HardShadow = 0.13846
ShadowSoft = 2
Reflection = 0
BaseColor = 1,1,1
OrbitStrength = 0.515
X = 0.6,0.0117647,0.0117647,0.59056
Y = 1,0.6,0,0.44882
Z = 1,1,1,0.49606
R = 0.666667,0.666667,0.498039,0.07936
BackgroundColor = 0.666667,0.666667,0.498039
GradientBackground = 0.3
CycleColors = false
Cycles = 4.27409
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Scale = 2
Offset = 1,1,1
Angle1 = 0
Rot1 = 1,1,1
Angle2 = 0
Rot2 = 1,1,1
Iterations = 12
ColorIterations = 13
Up = 0,1,0
depthMag = 1
ShowDepth = false
AntiAlias = 1
MaxRaySteps = 111
BoundingSphere = 2
Specular = 4
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
Eye = -1.926871,-2.362987,-2.728804
Target = -1.369221,-1.594053,-1.860413
DepthToAlpha = false
Detail = -3
DetailAO = -0.5
FudgeFactor = 0.46
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,0.7
CamLight = 0.7764706,0.7764706,0.7764706,0.875
CamLightMin = 0.0645161
Glow = 1,1,1,0.0144928
GlowMax = 17
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
T2offset = 2
T2Scale = 1
T2Iteration1 = 9
depthMag = 1
ShowDepth = false
AntiAlias = 1
MaxRaySteps = 111
BoundingSphere = 2
Specular = 3.2
SpecularExp = 31.37255
SpotLight = 0.1098039,1,0.8980392,0.5319149
SpotLightDir = -0.2,1
Fog = 0
HardShadow = 0
ShadowSoft = 2
Reflection = 0
Scale = 3
s = 0
Sphere = false
HoleSphere = false
unSphere = false
Octalize = false
unOctalize = false
Offset = 1,1,1
Total_Sphere_Iter = 11
SphereiterationT1 = 9
SphereiterationT2 = 9
SphereiterationT3 = 9
Total_unSphere_Iter = 11
unSphereiterationT1 = 11
unSphereiterationT2 = 11
unSphereiterationT3 = 9
RotVector = 1,0,0
RotAngle = 0
RotIter1 = 3
RotIter2 = 9
RotEverAfter = 9
tetralixinatorer = false
test = false
Iterations = 8
ColorIterations = 8
Up = 0.3343058,0.3954327,-0.5648231
#endpreset



/*void fun2 (inout vec3 z) {
	float yz=sqrt(z.x*z.x+z.y*z.y+z.z*z.z);
	float tanyz=(z.x*scale)/yz;
	tanyz=sr13-sr23*tanyz;
	tanyz*=tanyz;
	yz=sqrt(tanyz+2./3.);
	z.yz*=yz;
	float tanx=(length(z.yz)*scale)/yz;
	tanx=sr13-sr23*tanx;
	tanx*=tanx;
	yz=sqrt(tanx+2./3.);
	z.x*=yz;
} */