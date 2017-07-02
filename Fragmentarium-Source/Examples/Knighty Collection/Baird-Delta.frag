#info Baird Delta
#define providesInit
#define USE_EIFFIE_SHADOW
#define MULTI_SAMPLE_AO
#include "DE-kn2.frag"
#include "MathUtils.frag"

#group Baird_Delta
// Number of fractal iterations.
uniform int Iterations;  slider[0,9,100]

uniform float angle; slider[60.0001,72,90]

//Rotation
uniform vec3 RotVector; slider[(0,0,0),(1,1,1),(1,1,1)]

uniform float RotAngle; slider[0.00,0,360]

//#define PI 3.14159
#define ACONV PI/360.

mat3 rot;

const vec3 fl0=vec3(cos(PI/6.),-sin(PI/6.),0.);
vec3 fl1=vec3(0.);
vec3 fp1=vec3(0.);
float scl=1.;
void init() {
	float beta=ACONV*angle;
	float t=tan(beta);
	fp1=vec3(0.5,0.,sqrt(3.*t*t-1.)*0.25);
	fl1=normalize(vec3(1.,0.,-0.5*sqrt(3.*t*t-1.)));
	t=cos(beta);
	scl=4.*t*t;

	rot = rotationMatrix3(normalize(RotVector), RotAngle);
}

float DE(vec3 p) {
	float r2=dot(p,p), dd=1.; 
	for(int i=0; i<Iterations && r2<10000.; i++){
		//Sierpinski triangle symmetry + fold about xy plane
		p.yz=abs(p.yz);
		float t=2.*min(0.,dot(p,fl0)); p-=t*fl0;
		p.y=abs(p.y);
		
		//Koch curve fold
		p-=fp1;
		t=2.*min(0.,dot(p,fl1)); p-=t*fl1;
		p+=fp1;

		//p*=rot;
		//scale
		p.x-=1.;p*=rot; p*=scl; p.x+=1.;dd*=scl;
		r2=dot(p,p);
		orbitTrap = min(orbitTrap, abs(vec4(p.x,p.y,p.z,r2)));
	}
	return (sqrt(r2)-3.)/dd;
}
#preset Default
FOV = 0.4
Eye = 1.93719,1.284552,0.9206986
Target = -5.895554,-3.751212,-2.72476
FocalPlane = 1
Aperture = 0
InFocusAWidth = 0
ApertureNbrSides = 5 NotLocked
ApertureRot = 0
ApStarShaped = false NotLocked
Gamma = 2.2
ToneMapping = 3
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 1
Bloom = false Locked
BloomIntensity = 0.25
BloomPow = 2
BloomTaps = 4
Detail = -3
RefineSteps = 4
FudgeFactor = 1
MaxRaySteps = 128
MaxDistance = 20
Dither = 0.537037
NormalBackStep = 1
DetailAO = -1.5
coneApertureAO = 0.7894737
maxIterAO = 20
AO_ambient = 0.9756098
AO_camlight = 0
AO_pointlight = 0.2857143
AoCorrect = 0
Specular = 0.4
SpecularExp = 8.929
CamLight = 1,1,1,0
AmbiantLight = 1,1,1,0.7058824
Glow = 1,1,1,0
GlowMax = 20
Reflection = 0,0,0
ReflectionsNumber = 0 Locked
SpotGlow = true
SpotLight = 1,1,1,4
LightPos = 0.4,4.4,1.948
LightSize = 0.1
LightFallOff = 0
LightGlowRad = 0
LightGlowExp = 1
HardShadow = 1 Locked
ShadowSoft = 20
BaseColor = 0.807843,0.807843,0.807843
OrbitStrength = 0.5116279
X = 0.105882,0.423529,0.835294,0.5534
Y = 1,0.752941,0.00784314,1
Z = 0.329412,0.831373,0.215686,-1
R = 0.945098,0.545098,0.494118,0.09804
BackgroundColor = 0,0,0
GradientBackground = 0.3
CycleColors = true
Cycles = 4.04901
EnableFloor = true
FloorNormal = 0,0,0.90244
FloorHeight = -0.6
FloorColor = 0.678431,0.678431,0.678431
HF_Fallof = 0.0005
HF_Const = 0
HF_Intensity = 0
HF_Dir = 0,0,1
HF_Offset = 0
HF_Color = 1,1,1,1
HF_Scatter = 0.838
HF_Anisotropy = 0.176471,0.12549,0.0941176
HF_FogIter = 8
HF_CastShadow = true
CloudScale = 1
CloudFlatness = 0
CloudTops = 1
CloudBase = -1
CloudDensity = 1
CloudRoughness = 1
CloudContrast = 1
CloudColor = 0.65,0.68,0.7
SunLightColor = 0.7,0.5,0.3
FudgeAO = 0.5090909
Up = -0.2876681,-0.1832309,0.8712043
Iterations = 12
angle = 80
RotVector = 0,0,1
RotAngle = 0
#endpreset

#preset AO_Alone
FOV = 0.4
Eye = 1.85828,1.23429,0.885389
Target = -6.03281,-3.79196,-2.64557
FocalPlane = 1
Aperture = 0
InFocusAWidth = 0
ApertureNbrSides = 5 NotLocked
ApertureRot = 0
ApStarShaped = false NotLocked
Gamma = 2
ToneMapping = 5
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 1
Bloom = false Locked
BloomIntensity = 0.25
BloomPow = 2
BloomTaps = 4
Detail = -3
RefineSteps = 4
FudgeFactor = 1
MaxRaySteps = 128
MaxDistance = 20
Dither = 0.5102
NormalBackStep = 1
DetailAO = -1
coneApertureAO = 0.7368421
maxIterAO = 20
AO_ambient = 1
AO_camlight = 0
AO_pointlight = 0
AoCorrect = 0
Specular = 0.4
SpecularExp = 8.929
CamLight = 1,1,1,0
AmbiantLight = 1,1,1,1
Glow = 1,1,1,0
GlowMax = 20
Reflection = 0,0,0
ReflectionsNumber = 0 Locked
SpotGlow = true
SpotLight = 1,1,1,0
LightPos = 1.1688,3.2468,1.948
LightSize = 0.1
LightFallOff = 0
LightGlowRad = 0
LightGlowExp = 1
HardShadow = 1 Locked
ShadowSoft = 20
BaseColor = 1,1,1
OrbitStrength = 0
X = 0.105882,0.423529,0.835294,0.5534
Y = 1,0.752941,0.00784314,1
Z = 0.329412,0.831373,0.215686,-1
R = 0.945098,0.545098,0.494118,0.09804
BackgroundColor = 0,0,0
GradientBackground = 0.3
CycleColors = true
Cycles = 4.04901
EnableFloor = true
FloorNormal = 0,0,0.90244
FloorHeight = -0.6
FloorColor = 1,1,1
HF_Fallof = 0.0005
HF_Const = 0
HF_Intensity = 0
HF_Dir = 0,0,1
HF_Offset = 0
HF_Color = 1,1,1,1
HF_Scatter = 0.838
HF_Anisotropy = 0.176471,0.12549,0.0941176
HF_FogIter = 8
HF_CastShadow = true
CloudScale = 1
CloudFlatness = 0
CloudTops = 1
CloudBase = -1
CloudDensity = 1
CloudRoughness = 1
CloudContrast = 1
CloudColor = 0.65,0.68,0.7
SunLightColor = 0.7,0.5,0.3
FudgeAO = 1
Iterations = 12
angle = 80
RotVector = 0,0,1
RotAngle = 0
Up = 0.155105,0.0729892,0.985198
#endpreset
