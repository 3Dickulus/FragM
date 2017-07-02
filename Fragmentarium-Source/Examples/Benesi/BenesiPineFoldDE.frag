#info Benesi fold + pine Distance Estimator

#define providesInside
#define providesInit
#include "DE-Raytracer.frag"
//#define providesColor
#include "MathUtils.frag"

const	float sr23=sqrt(2./3.);
const	float sr13=sqrt(1./3.);
const	float sr12=sqrt(.5);


void init() {
	// rot = rotationMatrix3(normalize(RotVector), RotAngle);
}

#group Fractal
// Number of fractal iterations.
uniform int Iterations;  slider[0,5,100]

// Number of color iterations.
uniform int ColorIterations;  slider[0,3,100]
uniform int ColorIterationsMin;  slider[0,1,100]

// Mandelbulb exponent (2 is standard)
uniform float manPower; slider[-10,2,10]

uniform bool Julia; checkbox[false]
uniform vec3 JuliaC; slider[(-5,-5,-5),(0,0,0),(5,5,5)]
//  Multiply pixel parts by this
uniform vec3 PixelMult; slider[(-2,-2,-2),(1,0,0),(2,2,2)]

// Bailout   
uniform float Bailout; slider[0,2,4]
#group Transforms
// magseed is calculated
uniform float T1offset;slider[-5,2.,5]
uniform float T1Scale;slider[-5,2.,5]
uniform vec3 RotVector1; slider[(0,0,0),(1,1,1),(1,1,1)]
uniform float RotAngle1; slider[0.00,0,360]
uniform int Rot1Iteration; slider[1,1,22]
mat3 rot1= rotationMatrix3(normalize(RotVector1), RotAngle1);
uniform float T4offset;slider[-5,2.,5]
uniform float T4Scale;slider[-5,1.,5]
uniform int T4Iteration1;slider[1,5,20]
//uniform int T4Iteration2;slider[1,10,20]
uniform float T3offset;slider[-5,2.,5]
uniform float T3Scale;slider[-5,2.,5]
uniform int T3Iteration1;slider[1,10,20]
//uniform int T3Iteration2;slider[1,10,20]
uniform float T5boffset;slider[-5,2.,5]
uniform float T5bScale;slider[-5,1.,5]
uniform int T5bIteration1;slider[1,5,20]
//uniform vec3 RotVector2; slider[(0,0,0),(1,1,1),(1,1,1)]
//uniform float RotAngle2; slider[0.00,0,360]
//mat3 rot2= rotationMatrix3(normalize(RotVector2), RotAngle2);
//uniform vec3 RotVector3; slider[(0,0,0),(1,1,1),(1,1,1)]
//uniform float RotAngle3; slider[0.00,0,360]
//mat3 rot3= rotationMatrix3(normalize(RotVector3), RotAngle3);

void Transform1(inout vec3 z,inout float dr,in int i) {
	float tx=z.x*sr23-z.z*sr13;
	z.z=z.x*sr13 + z.z*sr23;   
	z.x=tx*sr12-z.y*sr12;               
	z.y=tx*sr12+z.y*sr12; 
	
	z=abs(z)*T1Scale;
	dr*=T1Scale;
	if (i>=Rot1Iteration) {z*=rot1;}     //rotation is here!!!!

	tx=z.x*sr12+z.y*sr12;
	z.y=-z.x*sr12+z.y*sr12; 
	z.x=tx*sr23+z.z*sr13-T1offset;
	z.z=-tx*sr13+z.z*sr23;		
}

void Transform3(inout vec3 z, inout float dr,in int i) {
	float tx=z.x*sr23-z.z*sr13;
	z.z=z.x*sr13 + z.z*sr23;   
	z.x=tx*sr12-z.y*sr12;               
	z.y=tx*sr12+z.y*sr12; 
	z=abs(z);
	z=vec3(abs(z.y+z.z-T3offset),abs(z.x+z.z-T3offset),abs(z.x+z.y-T3offset));
	z*=T3Scale;
	
	//if (i>=Rot1Iteration) {z*=rot1;}     //rotation is here!!!!

	tx=z.x*sr12+z.y*sr12;
	z.y=-z.x*sr12+z.y*sr12; 
	z.x=tx*sr23+z.z*sr13;
	z.z=-tx*sr13+z.z*sr23;		
}

void Transform4(inout vec3 z, inout float dr ,in int i) {
	float tx=z.x*sr23-z.z*sr13;
	z.z=z.x*sr13 + z.z*sr23;   
	z.x=tx*sr12-z.y*sr12;               
	z.y=tx*sr12+z.y*sr12; 
	
	z=vec3(abs(z.y*z.y+z.z*z.z-T4offset),abs(z.x*z.x+z.z*z.z-T4offset),abs(z.x*z.x+z.y*z.y-T4offset));
	z*=T4Scale;
	dr*=T4Scale;
	//if (i>=Rot1Iteration) {z*=rot1;}     //rotation is here!!!!

	tx=z.x*sr12+z.y*sr12;
	z.y=-z.x*sr12+z.y*sr12; 
	z.x=tx*sr23+z.z*sr13;
	z.z=-tx*sr13+z.z*sr23;		
}
void Transform5b(inout vec3 z,inout float dr,in int i) {
	float tx=z.x*sr23-z.z*sr13;
	z.z=z.x*sr13 + z.z*sr23;   
	z.x=tx*sr12-z.y*sr12;               
	z.y=tx*sr12+z.y*sr12; 
	
	z=vec3(abs(pow(pow(z.y,8.)+pow(z.z,8.),.25)-T5boffset),abs(pow(pow(z.x,8.)+pow(z.z,8.),.25)-T5boffset),abs(pow(pow(z.x,8.)+pow(z.y,8.),.25)-T5boffset));
	z*=T5bScale;
	dr*=T5bScale;
	//if (i>=Rot1Iteration) {z*=rot1;}     //rotation is here!!!!

	tx=z.x*sr12+z.y*sr12;
	z.y=-z.x*sr12+z.y*sr12; 
	z.x=tx*sr23+z.z*sr13;
	z.z=-tx*sr13+z.z*sr23;		
}


void PinePart (inout vec3 z) {	
	
	if ((manPower==2.)) {
		float sx2=z.x*z.x;
		float sy2=z.y*z.y;				
		float sz2=z.z*z.z;
		float r1=2.*(z.x)/sqrt(sy2+sz2); 
		z.x=sx2-sy2-sz2;   
 		z.y=r1*2.*z.y*z.z;
		z.z=r1*(sy2-sz2);   
	} else {
		float rxyz=pow((z.x*z.x+z.y*z.y+z.z*z.z),abs(manPower)/2.);
		float r1=sqrt(z.y*z.y+z.z*z.z);
		float victor=atan(r1,z.x)*manPower;
		float phi=atan(z.z,z.y)*manPower;
		z.x=rxyz*cos(victor);
		r1=rxyz*sin(victor);
		z.z=r1*cos(phi);
		z.y=r1*sin(phi);
		
	}
}


float DE(vec3 pos) {

	vec3 z=pos;
	vec3 pixelparts=vec3(pos.x*PixelMult.x,pos.y*PixelMult.y,pos.z*PixelMult.z);
	int i=1;
	float r=length(z);
	float dr=1.0;
	while(r<Bailout && (i<Iterations)) {
			
		//if (i==T4Iteration1 || i==T4Iteration2) {Transform4(z,i);}
		//if (i==T3Iteration1 || i==T3Iteration2) {Transform3(z,i);}
			if (i==T3Iteration1) {Transform3(z,dr,i);}
			if (i==T4Iteration1) {Transform4(z,dr,i);}
			if (i==T5bIteration1) {Transform5b(z,dr,i);}
			Transform1(z,dr,i);
			PinePart(z);

		if (i<ColorIterations) {
			orbitTrap=min(orbitTrap,abs(vec4(z.x*z.x,z.y*z.y,z.z*z.z,dot(z,z))));
		}

		if (Julia) {
			z+=JuliaC;	
		} else {
			z+=pixelparts;
		}	
		r=length(z);
		dr = r*dr*2.0+ 1.0; //pow(i,-i);
		i++;
	}
	return 0.5*log(r)*r/dr;
}

#preset Default
FOV = 0.62536
Eye = -2,0,-1
Target = -1.4,0,0
Up = 0,-1,0
EquiRectangular = false
FocalPlane = 1.1
Aperture = 0
Gamma = 1
ToneMapping = 3
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 6
AntiAliasScale = 1.5
Detail = -3.5
DetailAO = 0
FudgeFactor = 1
MaxRaySteps = 444
Dither = 1
NormalBackStep = 2
AO = 0,0,0,0.85185
Specular = 1.6456
SpecularExp = 1
SpecularMax = 8
SpotLight = 1,1,1,1
SpotLightDir = -1,-0.2
CamLight = 1,1,1,1.5
CamLightMin = 0.12121
Glow = 1,1,1,0.03
GlowMax = 52
Fog = 0
HardShadow = 0.35385
ShadowSoft = 12.5806
Reflection = 0
DebugSun = false
BaseColor = 0.0980392,0.960784,0.717647
OrbitStrength = 0
X = 0.0823529,1,0.941176,0
Y = 0.411765,0.666667,0.462745,0
Z = 0,0.984314,0.772549,0.3
R = 0.509804,0.764706,1,0
BackgroundColor = 0,0,0
GradientBackground = 0
CycleColors = false
Cycles = 4.04901
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 5
ColorIterations = 2
ColorIterationsMin = 1
manPower = 2
Julia = false
JuliaC = 0,0,0
PixelMult = 1,0,0
Bailout = 2
T1offset = 2
T1Scale = 2
RotVector1 = 1,1,1
RotAngle1 = 0
Rot1Iteration = 2
T4offset = 2
T4Scale = 2
T4Iteration1 = 6
T3offset = 2
T3Scale = 1.4
T3Iteration1 = 4
T5boffset = 2
T5bScale = 2
T5bIteration1 = 8
#endpreset

#preset test1
FOV = 0.62536
Eye = 0.879486,0,1.34087
Target = -0.52051,0,0
Up = 0,-1,0
EquiRectangular = false
FocalPlane = 1.1
Aperture = 0
Gamma = 1
ToneMapping = 3
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 0
AntiAliasScale = 0
Detail = -3.5
DetailAO = 0
FudgeFactor = 1
MaxRaySteps = 444
Dither = 1
NormalBackStep = 2
AO = 0,0,0,0.85185
Specular = 1.6456
SpecularExp = 1
SpecularMax = 8
SpotLight = 1,1,1,1
SpotLightDir = 0,-0.2
CamLight = 1,1,1,1.5
CamLightMin = 0.46269
Glow = 1,1,1,0.03
GlowMax = 52
Fog = 0
HardShadow = 0.35385
ShadowSoft = 12.5806
Reflection = 0
DebugSun = false
BaseColor = 0.0980392,0.960784,0.717647
OrbitStrength = 0.3
X = 0.0823529,1,0.941176,0
Y = 0.411765,0.666667,0.462745,0
Z = 0,0.984314,0.772549,0.3
R = 0.509804,0.764706,1,0
BackgroundColor = 0,0,0
GradientBackground = 0
CycleColors = false
Cycles = 4.04901
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 5
ColorIterations = 2
ColorIterationsMin = 1
manPower = 2
Julia = false
JuliaC = 0,0,0
PixelMult = 1,0,0
T1offset = 2
T1Scale = 2
RotVector1 = 1,1,1
RotAngle1 = 0
Rot1Iteration = 2
T4offset = 2
T4Scale = 2
T4Iteration1 = 6
T3offset = 2
T3Scale = 1.4
T3Iteration1 = 4
T5boffset = 2
T5bScale = 2
T5bIteration1 = 8
Bailout = 2
#endpreset

#preset test2
FOV = 0.62536
Eye = -0.505,1.515,1.515
Target = 0,0,0
Up = 0,1,0
EquiRectangular = false
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
Detail = -4
DetailAO = 0
FudgeFactor = 1
MaxRaySteps = 444
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,0.85185
Specular = 1.6456
SpecularExp = 16.364
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = 0.63626,0.5
CamLight = 1,1,1,1.53846
CamLightMin = 0.12121
Glow = 1,1,1,0.03
GlowMax = 52
Fog = 0
HardShadow = 0.35385
ShadowSoft = 12.5806
Reflection = 0
DebugSun = false
BaseColor = 0.490196,0.670588,0.486275
OrbitStrength = 0.5
X = 1,1,1,0
Y = 0.345098,0.666667,0,0.02912
Z = 0.494118,0.988235,0.678431,0
R = 0.0784314,1,0.941176,0.4
BackgroundColor = 0,0,0
GradientBackground = 0
CycleColors = false
Cycles = 4.04901
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 5
ColorIterations = 2
ColorIterationsMin = 1
manPower = 2
Julia = false
JuliaC = 0,0,0
PixelMult = 1,0,0
Bailout = 2
T1offset = 2
T1Scale = 2
RotVector1 = 1,1,1
RotAngle1 = 0
Rot1Iteration = 2
T4offset = 2
T4Scale = 2
T4Iteration1 = 6
T3offset = 2
T3Scale = 1.4
T3Iteration1 = 4
T5boffset = 2
T5bScale = 2
T5bIteration1 = 8
#endpreset

#preset MyTest
FOV = 0.62536
Eye = -1.659139,-0.6174882,-0.5888647
Target = -0.8343153,-0.0149389,-0.0261916
EquiRectangular = false
FocalPlane = 0.5
Aperture = 0.001
Gamma = 1
ToneMapping = 3
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 5
AntiAliasScale = 1.5
Detail = -4
DetailAO = -1.012048
FudgeFactor = 1
MaxDistance = 10
MaxRaySteps = 90
Dither = 1
NormalBackStep = 10 NotLocked
AO = 0,0,0,0.85185
Specular = 1
SpecularExp = 1
SpecularMax = 8
SpotLight = 1,1,1,1
SpotLightDir = -0.1290323,-0.1290323
CamLight = 1,1,1,1
CamLightMin = 0
Glow = 1,1,1,0.1298701
GlowMax = 61
Fog = 0
HardShadow = 0.35385 NotLocked
ShadowSoft = 12.5806
QualityShadows = false
Reflection = 0 NotLocked
DebugSun = false NotLocked
BaseColor = 0.0980392,0.960784,0.717647
OrbitStrength = 1
X = 0.0823529,1,0.941176,-0.3404255
Y = 0.411765,0.666667,0.462745,1
Z = 0,0.984314,0.772549,-1
R = 0.509804,0.764706,1,1
BackgroundColor = 0,0,0
GradientBackground = 1.842105
CycleColors = false
Cycles = 5.761539
EnableFloor = false NotLocked
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 10
ColorIterations = 5
ColorIterationsMin = 1
manPower = 3
Julia = false
JuliaC = 0,0,0
PixelMult = 1,0,0
Bailout = 4
T1offset = 1.190476
T1Scale = 1.162791
RotVector1 = 1,0,0
RotAngle1 = 0
Rot1Iteration = 1
T4offset = 2.857143
T4Scale = 1.162791
T4Iteration1 = 3
T3offset = 2.142857
T3Scale = 1.27907
T3Iteration1 = 5
T5boffset = 5
T5bScale = 0.99
T5bIteration1 = 3
Up = -0.7068628,0.5072215,0.4930237
#endpreset
