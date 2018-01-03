#version 120
//Aexion's Quadray formula
//Script by Knighty. Added a 'Sign' parameter to the formula
//Looks like there is still some work to do on the DE formula

#info Quadray sets Distance Estimator
#include "MathUtils.frag"
#include "DE-Raytracer.frag"
#group Quadray

// Number of fractal iterations.
uniform int Iterations;  slider[0,11,100]

// Bailout radius
uniform float Bailout; slider[2,3.5,64]

//Offset int the 4th dimension
uniform float Offset; slider[-2,0,2]

//sign. Actually a factor of the c value
uniform float Sign; slider[-2,-1,2]

const mat3x4 mc=mat3x4(vec4(.5,-.5,-.5,.5),
						     vec4(.5,-.5,.5,-.5),
						     vec4(.5,.5,-.5,-.5));
float DE(vec3 pos) {
	vec4 cp=Sign*(abs(mc*pos)+vec4(Offset));
	vec4 z=cp;
	float r=length(z);
	float dr=1.;
	for(int i=0; i<Iterations && r<Bailout;i++){
		dr=2.*r*dr+abs(Sign);
		vec4 tmp0=z*z;
		vec2 tmp1=2.*z.wx*z.zy;
		z=tmp0-tmp0.yxwz+tmp1.xxyy+cp;
		r=length(z);
		orbitTrap = min(orbitTrap, abs(vec4(z.x,z.y,z.z,r*r)));
	}
	return 0.5*r*log(r)/dr;
	//return 0.5*(r-2.)*log(r+1.)/dr;
}
#preset Default
FOV = 0.62536
Eye = 0.8667741,-2.021315,-1.704485
Target = -1.610366,4.413903,3.861513
Up = 0.1159535,0.6737829,-0.7273999
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
Detail = -2.84956
DetailAO = -1.35716
FudgeFactor = 1
MaxDistance = 1000
MaxRaySteps = 164
Dither = 0.51754
NormalBackStep = 1
AO = 0,0,0,0.85185
Specular = 1
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
QualityShadows = false
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
Bailout = 6.279
Offset = 0
Sign = -1
#endpreset
