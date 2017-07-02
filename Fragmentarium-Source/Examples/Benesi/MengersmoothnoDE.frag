#info Menger Distance Estimator.
#define providesInside

#include "MathUtils.frag"
#include "Brute-Raytracer.frag"
#group Menger
// Based on Knighty's Kaleidoscopic IFS 3D Fractals, described here:
// http://www.fractalforums.com/3d-fractal-generation/kaleidoscopic-%28escape-time-ifs%29/

// Scale parameter. A perfect Menger is 3.0
uniform float Scale; slider[0.00,3.0,6.00]
uniform float s; slider[0.000,0.001,0.100]
uniform bool Sphere;checkbox[false]
uniform int Iterations;  slider[0,8,100]
uniform float RotAngle; slider[0.00,0,180]
uniform vec3 RotVector; slider[(0,0,0),(1,1,1),(1,1,1)]
uniform float Bailout; slider[0.00,3,15]

uniform int ColorIterations;  slider[0,8,100]

mat3 rot = rotationMatrix3(normalize(RotVector), RotAngle);

vec3 convert3 (vec3 z) {
	
	vec3 z2=normalize(abs(z));
	
   	float ang1=abs(atan(z2.y/z2.z));  
	float r2=sqrt(z2.z*z2.z+z2.y*z2.y);
	float ang2=abs(atan(r2/z2.x));
	
	if (ang1<.7854) {ang1=1./cos(ang1);}  else {ang1=1./sin(ang1);}
	if (ang2<.7854) {ang2=1./cos(ang2);}  else {ang2=1./sin(ang2);}
	

	z.yz*=ang1;
	z.xyz*=ang2;
	
	return z;
}

bool inside (vec3 z)
{
	float t=9999.0;
	 float sc=Scale;
	 float sc1=sc-1.0;
	float sc2=sc1/sc;
	vec3 C=vec3(1.0,1.0,.5);
	float r=0.;
	int n = 0;
		if(Sphere) {z=convert3(z);}
	while (n < Iterations  && r<Bailout) {

		z = vec3(sqrt(z.x*z.x+s),sqrt(z.y*z.y+s),sqrt(z.z*z.z+s));

		z = rot *z;

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
		if (n<ColorIterations) orbitTrap = min(orbitTrap, (vec4(abs(z),dot(z,z))));
		r=length(z);
		n++;
	}
	
	return (r<Bailout);
}
#preset newest
FOV = 0.4
Eye = -3,0,0
Target = 0,0,0
Up = 0,1,0
AroundTarget = 0,0
AroundEye = 0,0
UpRotate = 0
EquiRectangular = false
Gamma = 1
ToneMapping = 1
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
NormalScale = 1
AOScale = 1
Glow = 1
AOStrength = 0.6
Samples = 222
Stratify = true
DebugInside = false
CentralDifferences = true
SampleNeighbors = true
Near = 0
Far = 5
ShowDepth = false
DebugNormals = false
Specular = 0.1666
SpecularExp = 16
SpotLight = 1,1,1,0.19608
SpotLightDir = 0.37142,0.1
CamLight = 1,1,1,1.13978
CamLightMin = 0.29412
Fog = 0.4161
BaseColor = 1,1,1
OrbitStrength = 0.2987
X = 0.5,0.6,0.6,0.2126
Y = 1,0.6,0,0.30708
Z = 0.8,0.78,1,0.35434
R = 0.552941,0.666667,0.639216,0.03174
BackgroundColor = 0.0431373,0.0470588,0.0431373
GradientBackground = 0.3
CycleColors = false
Cycles = 6.95699
Scale = 3
OffScale = 3
RotVector = 1,1,1
RotAngle = 0
RotAnglexy = 0
RotAnglexz = 0
RotAngleyz = 0
OffSet = 1,1,1
OffSetMult = 1
addin = 0,0,0
addin2 = 0
Bailout = 3
MengerBailout = 3
Iterations = 8
ColorIterations = 2
#endpreset

#preset Default
FOV = 0.4
Eye = -3.72729,-0.0860174,-1.93389
Target = 5.14721,0.118786,2.6706
Up = -0.00348884,0.999311,-0.0369503

Specular = 0.1666
SpecularExp = 16
SpotLight = 1,1,1,0.19608
SpotLightDir = 0.37142,0.1
CamLight = 1,1,1,1.13978
CamLightMin = 0.29412
Glow = 1,1,1,0.07895
Fog = 0.4161
BaseColor = 1,1,1
OrbitStrength = 0.2987
X = 0.5,0.6,0.6,0.2126
Y = 1,0.6,0,0.30708
Z = 0.8,0.78,1,0.35434
R = 0.666667,0.666667,0.498039,0.03174
BackgroundColor = 0.6,0.6,0.45
GradientBackground = 0.3
CycleColors = false
Cycles = 6.95699
Scale = 3
RotVector = 1,1,1
RotAngle = 0
Offset = 1,1,1
Iterations = 8
ColorIterations = 2
#endpreset

#preset Twisted
FOV = 0.4
Eye = 0.18678,-2.50326,0.726368
Target = -1.17942,6.78003,-2.73109
Up = -0.925816,-0.244735,-0.288044
AntiAlias = 1
Detail = -2.60183
DetailAO = -0.63
FudgeFactor = 1
MaxRaySteps = 156
BoundingSphere = 18.181
Dither = 0.5
AO = 0,0,0,0.81
Specular = 0
SpecularExp = 16
SpotLight = 1,1,1,0.22857
SpotLightDir = -0.03614,0.1
CamLight = 1,1,1,1.32394
CamLightMin = 0.15294
Glow = 1,1,1,0.02174
Fog = 0.15748
HardShadow = 0.13846
Reflection = 0
BaseColor = 1,1,1
OrbitStrength = 0.20253
X = 0.5,0.6,0.6,0.2
Y = 1,0.6,0,0.2762
Z = 0.8,0.78,1,-0.08572
R = 0.666667,0.666667,0.498039,0.21154
BackgroundColor = 0.6,0.6,0.45
GradientBackground = 0.3
CycleColors = true
Cycles = 4.27409
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Scale = 3.1
RotVector = 1,0.40816,0.18367
RotAngle = 2.1366
Offset = 1,0.95614,1
Iterations = 13
ColorIterations = 8
#endpreset

#preset Silver
FOV = 0.4
Eye = -0.16015,1.81406,0.598377
Target = 1.0002,-6.2196,-5.24233
Up = 0.708366,0.417383,-0.569218
AntiAlias = 1
Detail = -2.53981
DetailAO = -0.5
FudgeFactor = 0.80723
MaxRaySteps = 56
BoundingSphere = 2
Dither = 0.21053
AO = 0,0,0,0.45679
Specular = 2.5316
SpecularExp = 16
SpotLight = 1,1,1,0.03261
SpotLightDir = 0.65626,-0.3125
CamLight = 1,1,1,1.13978
CamLightMin = 0
Glow = 1,1,1,0.07895
Fog = 0.4161
HardShadow = 0.4
Reflection = 0.57692
BaseColor = 1,1,1
OrbitStrength = 0
X = 0.5,0.6,0.6,0.2126
Y = 1,0.6,0,0.30708
Z = 0.8,0.78,1,0.35434
R = 0.666667,0.666667,0.498039,0.03174
BackgroundColor = 0.219608,0.219608,0.160784
GradientBackground = 0.3
CycleColors = true
Cycles = 18.1816
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Scale = 3
RotVector = 1,1,1
RotAngle = 0
Offset = 1,1,1
Iterations = 8
ColorIterations = 8
#endpreset
