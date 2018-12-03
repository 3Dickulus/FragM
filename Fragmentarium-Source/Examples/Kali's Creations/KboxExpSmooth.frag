#info Mandelbox Distance Estimator (Rrrola's version).
#define providesInit
#include "MathUtils.frag"
#include "ColorPalette.frag"
#include "DE-Kn2cr11.frag"
#group KaliBox

uniform int Iterations;  slider[0,17,300]
uniform int ColorIterations;  slider[0,3,300]
uniform float MinRad2;  slider[0,0.25,2.0]

uniform float Scale;  slider[-3.0,1.3,3.0]
uniform vec3 Trans; slider[(-5,-5,-5),(0.5,0.5,0.5),(5,5,5)]
uniform vec3 Julia; slider[(-5,-5,-5),(-1,-1,-1),(0,0,0)]
uniform bool DoJulia; checkbox[true]

uniform vec3 RotVector; slider[(0,0,0),(1,1,1),(1,1,1)]
uniform float RotAngle; slider[0.00,0,180]

mat3 rot;

void init() {
	 rot = rotationMatrix3(normalize(RotVector), RotAngle);
}

float absScalem1 = abs(Scale - 1.0);
float AbsScaleRaisedTo1mIters = pow(abs(Scale), float(1-Iterations));
vec4 scale = vec4(Scale, Scale, Scale, abs(Scale)) / MinRad2;


float DE(vec3 pos) {
	vec4 p = vec4(pos,1);  
	for (int i=0; i<Iterations; i++) {
		p.xyz*=rot;
		p.xyz=abs(p.xyz)+Trans;
		float r2 = dot(p.xyz, p.xyz);
		p *= clamp(max(MinRad2/r2, MinRad2), 0.0, 1.0);  
		p = p*scale;
		p += (DoJulia ? vec4(Julia,0) : vec4(0.0));
	
	}
	return (length(p.xyz) - absScalem1) / p.w - AbsScaleRaisedTo1mIters;
}

float Coloring(vec3 p) {
	float expsmooth=0.0;
	float r1 = 0.0, r2 = 0.0;
	for (int i=0; i<ColorIterations; i++) {
		p*=rot;
		p=abs(p)+Trans;
		r1 = r2;
		r2 = dot(p, p);
		p *= clamp(max(MinRad2/r2, MinRad2), 0.0, 1.0);  
		p = p* Scale/MinRad2;
		p += (DoJulia ? vec4(Julia,0) : vec4(0.0));
		expsmooth+=exp(-1.0/abs(r1-r2));	
	}
	return expsmooth;
}


#preset Default
Color1 = 0.666667,0.278431,0.0156863
Color2 = 1,1,1
Color3 = 0.380392,0.356863,0.498039
ColorDensity = 1
ColorOffset = 1.8519
FOV = 0.76422
Eye = -0.653263,-1.75106,0.356387
Target = 3.03364,1.45537,-2.01029
Up = -0.530146,0.795753,0.252224
EquiRectangular = false
FocalPlane = 0.7065
Aperture = 0.00364
Gamma = 1
ToneMapping = 1
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
Detail = -3.40711
DetailAO = -1.7857
FudgeFactor = 0.68224
MaxRaySteps = 522
Dither = 0.51754
NormalBackStep = 1
AO = 0,0,0,0.81481
Specular = 0.02353
SpecularExp = 20
SpecularMax = 10
SpotLight = 1,1,1,0.58824
SpotLightDir = -0.46874,-0.1875
CamLight = 1,1,1,0.26924
CamLightMin = 0
Glow = 1,1,1,0.26027
GlowMax = 227
Fog = 0.85186
HardShadow = 0
ShadowSoft = 2
Reflection = 0
DebugSun = false
BaseColor = 1,1,1
OrbitStrength = 1
X = 0.5,0.6,0.6,0.16536
Y = 1,0.6,0,0.4
Z = 0.8,0.78,1,0.07084
R = 0.4,0.7,1,0.03174
BackgroundColor = 0.258824,0.266667,0.301961
GradientBackground = 0
CycleColors = false
Cycles = 9.51206
EnableFloor = false
FloorNormal = 0,0,1
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 15
ColorIterations = 10
MinRad2 = 0.43138
Scale = 1.94736
Trans = 0,-2.6991,0
Julia = -0.5,0,-0.5
RotVector = 1,1,0
RotAngle = 0
#endpreset