#info Menger Distance Estimator.  Benesi.
#define providesInit
#include "Fast-Raytracer-with-Palette.frag"
#include "MathUtils.frag"
#group Menger
// Based on Knighty's Kaleidoscopic IFS 3D Fractals, described here:
// http://www.fractalforums.com/3d-fractal-generation/kaleidoscopic-%28escape-time-ifs%29/
float sr32=1.2247448713915890491;   
const float pi=2.*1.570796326794897;
const float pi2=2.*pi;
const float sr13=sqrt(1./3.);
const float sr23=sqrt (2./3.);
// Scale parameter. A perfect Menger is 3.0
uniform float Scale; slider[0.00,3.0,4.00]
uniform float s; slider[0.000,0.005,0.100]
uniform bool HoleSphere; checkbox[false]
// Number of fractal iterations.
uniform int Iterations;  slider[0,8,100]
uniform int ColorIterations;  slider[0,8,100]
#group Transforms
uniform bool Polyhedronate;checkbox[false]
uniform bool Stellate; checkbox[true]
uniform bool StellateHedron; checkbox[true]
uniform float starangle;slider[-3.1416,1.0,3.1416]
uniform float starangle2;slider[-3.1416,1.10,3.1416]
uniform bool test1; checkbox[false]
uniform bool test2; checkbox[false]

uniform bool bothWithSphericAndUnspheric;checkbox[true]

//set both sides to 3 for a tetra hedron, 4 for a cube, and that's it for the Platonics from this set
uniform float sides;slider[2.0,6.0,15.0]
uniform float sides2;slider[2.0,8.0,15.0]

uniform vec3 RotVector; slider[(0,0,0),(1,1,1),(1,1,1)]
uniform float RotAngle; slider[0.00,0,180]
uniform int RotIter1;slider[0,3,20]
uniform int RotEverAfter;slider[0,9,20]


mat3 rot;

void init() {
	rot = rotationMatrix3(normalize(RotVector), RotAngle);
}

void unPolyhedronator (inout vec3 z) {  
	float rCyz;
	float rCxyz;
if (bothWithSphericAndUnspheric) {	rCyz= (z.y*z.y)/(z.z*z.z);
		if (rCyz<1.) {rCyz=1./sqrt(rCyz+1.);} else {rCyz=1./sqrt(1./rCyz+1.);}
		z.yz*=rCyz;
 		rCxyz= (z.y*z.y+z.z*z.z)/(z.x*z.x);
		if (rCxyz<1.) {rCxyz=1./sqrt(rCxyz+1.);} else {rCxyz=1./sqrt(1./rCxyz+1.);}
		z.xyz*=rCxyz;
	} 
	 rCyz=abs(atan(z.z,z.y));
	float i=1.;
 rCxyz= abs(atan(sqrt(z.y*z.y+z.z*z.z),z.x));
		while (rCxyz>pi/sides2 && i<sides2) {rCxyz-=pi2/sides2; i++;}
		z/=cos(rCxyz);
 i=1.;
	while (rCyz>pi/sides && i<sides) {rCyz-=pi2/sides; i++;}
	z.yz/=cos(rCyz);
}

void Polyhedronator (inout vec3 z) {  
	
	float rCyz=abs(atan(z.z,z.y));
	 float i=1.;
	while (rCyz>pi/sides && i<sides) {rCyz-=pi2/sides; i++;}
		z.yz*=cos(rCyz);
	i=1.;
	 float rCxyz= abs(atan(sqrt(z.y*z.y+z.z*z.z),z.x));
		i=1.;
		while (rCxyz>pi/sides2 && i<sides2) {rCxyz-=pi2/sides2; i++;}
		z*=cos(rCxyz);

 rCyz= (z.y*z.y)/(z.z*z.z);
 rCxyz= (z.y*z.y+z.z*z.z)/(z.x*z.x);
if (rCyz<1.) {rCyz=sqrt(rCyz+1.);} else {rCyz=sqrt(1./rCyz+1.);}
if (rCxyz<1.) {rCxyz=sqrt(rCxyz+1.);} else {rCxyz=sqrt(1./rCxyz+1.);}

z.yz*=rCyz;  
z*=rCxyz;  
}



void Stellation (inout vec3 z) {  
	
	float rCyz=abs(atan(z.z,z.y));  
	 float i=1.;
	while (rCyz>pi/sides && i<sides) {rCyz-=pi2/sides; i++;}
	rCyz=abs(rCyz);
		z.yz*=(cos(pi/sides*.5)*cos(rCyz-starangle))/cos(pi/(sides)*.5-starangle);
		float rCxyz;
		if (StellateHedron) {
	  rCxyz= abs(atan(sqrt(z.y*z.y+z.z*z.z),z.x));
		i=1.;
		while (rCxyz>pi/sides2 && i<sides2) {rCxyz-=pi2/sides2; i++;}
		rCxyz=abs(rCxyz);
		 z*=(cos(pi/sides2*.5)*cos(rCxyz-starangle2))/cos(pi/(sides2)*.5-starangle2);
		}
 rCyz= (z.y*z.y)/(z.z*z.z);

	if (rCyz<1.) {rCyz=sqrt(rCyz+1.);} else {rCyz=sqrt(1./rCyz+1.);}
 if (StellateHedron) {
		 rCxyz= (z.y*z.y+z.z*z.z)/(z.x*z.x);
		if (rCxyz<1.) {rCxyz=sqrt(rCxyz+1.);} else {rCxyz=sqrt(1./rCxyz+1.);}
		z*=rCxyz;  
	}
z.yz*=rCyz;  
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
	 if (Stellate) Stellation(z);
	if (test1 && !Stellate) Polyhedronator(z);
	if (Polyhedronate) Polyhedronator(z);	
		
	while (n <= Iterations) {
		if (n==RotIter1 || n>=RotEverAfter) { 
					z*=rot; }
	
	
			z = vec3(sqrt(z.x*z.x+s),sqrt(z.y*z.y+s),sqrt(z.z*z.z+s));
	
			t=z.x-z.y;  t= .5*(t-sqrt(t*t+s));
			z.x=z.x-t;	z.y=z.y+t;
		
			t=z.x-z.z; t= 0.5*(t-sqrt(t*t+s));
  			z.x=z.x-t;	 z.z= z.z+t;
	
			t=z.y-z.z;  t= 0.5*(t-sqrt(t*t+s));
  			z.y=z.y-t;  z.z= z.z+t;
		
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
Eye = -1.580232,-1.885012,-2.189006
Target = -1.022582,-1.116078,-1.320615
DepthToAlpha = false
depthMag = 1
ShowDepth = false
AntiAlias = 1
Detail = -3
DetailAO = -0.5
FudgeFactor = 0.4
MaxRaySteps = 111
BoundingSphere = 2
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,0.6
Specular = 2.2
SpecularExp = 31.37255
SpotLight = 0.8941176,1,0.8235294,0.5319149
SpotLightDir = -0.2,1
CamLight = 0.8980392,0.8980392,0.8980392,0.9189189
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
paletteColoring = true
pBaseColor = 0.9568627,0.9568627,0.9568627
BaseStrength = 0.6956522
cSpeed = 10
pOffset = 0
color0 = 0,0.9490196,0.4745098
Sharp0to1 = false
Dist0to1 = 1
color1 = 0.02745098,0.9529412,1
Sharp1to2 = false
Dist1to2 = 1
color2 = 0.9058824,0.9098039,0.5098039
Sharp2to3 = false
Dist2to3 = 1
color3 = 0.6470588,1,0.7372549
Sharp3to0 = false
Dist3to0 = 1
orbitStrengthXYZR = 0,0,0,1
Scale = 3
s = 0
HoleSphere = false
Iterations = 8
ColorIterations = 8
bothWithSphericAndUnspheric = true
Total_Polygonate_Iter = 0
PolygonateiterationT1 = 0
PolygonateiterationT2 = 0
PolygonateiterationT3 = 0
Total_unPolygonate_Iter = 0
unPolygonateiterationT1 = 0
unPolygonateiterationT2 = 0
unPolygonateiterationT3 = 0
sides = 3
sides2 = 3
RotVector = 1,1,1
RotAngle = 0
RotIter1 = 3
RotEverAfter = 9
Up = 0.3343059,0.3954328,-0.564823
#endpreset
