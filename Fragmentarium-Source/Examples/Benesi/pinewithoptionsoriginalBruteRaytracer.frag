
#info Mandelbulb without Distance Estimator

#define providesInside

//#define providesColor
#include "MathUtils.frag"
//#include "Brute-RaytracerExp.frag"
#include "Brute-Raytracer.frag"
const	float sr23=sqrt(2./3.);
const	float sr13=sqrt(1./3.);
const	float sr12=sqrt(.5);

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
uniform float Bailout; slider[0,12,30]
#group Transforms
// magseed is calculated
uniform float T1offset;slider[-5,2.,5]
uniform float T1Scale;slider[-5,2.,5]
uniform vec3 RotVector1; slider[(0,0,0),(1,1,1),(1,1,1)]
uniform float RotAngle1; slider[0.00,0,360]
uniform int Rot1Iteration; slider[1,1,22]
mat3 rot1= rotationMatrix3(normalize(RotVector1), RotAngle1);
uniform float T4offset;slider[-5,2.,5]
uniform float T4Scale;slider[-5,2.,5]
uniform int T4Iteration1;slider[1,5,20]
//uniform int T4Iteration2;slider[1,10,20]
uniform float T3offset;slider[-5,2.,5]
uniform float T3Scale;slider[-5,2.,5]
uniform int T3Iteration1;slider[1,10,20]
//uniform int T3Iteration2;slider[1,10,20]
uniform float T5boffset;slider[-5,2.,5]
uniform float T5bScale;slider[-5,2.,5]
uniform int T5bIteration1;slider[1,5,20]
//uniform vec3 RotVector2; slider[(0,0,0),(1,1,1),(1,1,1)]
//uniform float RotAngle2; slider[0.00,0,360]
//mat3 rot2= rotationMatrix3(normalize(RotVector2), RotAngle2);
//uniform vec3 RotVector3; slider[(0,0,0),(1,1,1),(1,1,1)]
//uniform float RotAngle3; slider[0.00,0,360]
//mat3 rot3= rotationMatrix3(normalize(RotVector3), RotAngle3);

void Transform1(inout vec3 z,in int i) {
	float tx=z.x*sr23-z.z*sr13;
	z.z=z.x*sr13 + z.z*sr23;   
	z.x=tx*sr12-z.y*sr12;               
	z.y=tx*sr12+z.y*sr12; 
	
	z=abs(z)*T1Scale;

	if (i>=Rot1Iteration) {z*=rot1;}     //rotation is here!!!!

	tx=z.x*sr12+z.y*sr12;
	z.y=-z.x*sr12+z.y*sr12; 
	z.x=tx*sr23+z.z*sr13-T1offset;
	z.z=-tx*sr13+z.z*sr23;		
}

void Transform3(inout vec3 z,in int i) {
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

void Transform4(inout vec3 z,in int i) {
	float tx=z.x*sr23-z.z*sr13;
	z.z=z.x*sr13 + z.z*sr23;   
	z.x=tx*sr12-z.y*sr12;               
	z.y=tx*sr12+z.y*sr12; 
	
	z=vec3(abs(z.y*z.y+z.z*z.z-T4offset),abs(z.x*z.x+z.z*z.z-T4offset),abs(z.x*z.x+z.y*z.y-T4offset));
	z*=T4Scale;

	//if (i>=Rot1Iteration) {z*=rot1;}     //rotation is here!!!!

	tx=z.x*sr12+z.y*sr12;
	z.y=-z.x*sr12+z.y*sr12; 
	z.x=tx*sr23+z.z*sr13;
	z.z=-tx*sr13+z.z*sr23;		
}
void Transform5b(inout vec3 z,in int i) {
	float tx=z.x*sr23-z.z*sr13;
	z.z=z.x*sr13 + z.z*sr23;   
	z.x=tx*sr12-z.y*sr12;               
	z.y=tx*sr12+z.y*sr12; 
	
	z=vec3(abs(pow(pow(z.y,8.)+pow(z.z,8.),.25)-T5boffset),abs(pow(pow(z.x,8.)+pow(z.z,8.),.25)-T5boffset),abs(pow(pow(z.x,8.)+pow(z.y,8.),.25)-T5boffset));
	z*=T5bScale;

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


bool inside(vec3 pos) {
	orbitTrap= vec4(10.0);
	vec3 z=pos;
	vec3 pixelparts=vec3(pos.x*PixelMult.x,pos.y*PixelMult.y,pos.z*PixelMult.z);
	int i=1;
	float r=length(z);

	while(r<Bailout && (i<Iterations)) {
			
		//if (i==T4Iteration1 || i==T4Iteration2) {Transform4(z,i);}
		//if (i==T3Iteration1 || i==T3Iteration2) {Transform3(z,i);}
			if (i==T3Iteration1) {Transform3(z,i);}
			if (i==T4Iteration1) {Transform4(z,i);}
			if (i==T5bIteration1) {Transform5b(z,i);}
			Transform1(z,i);
			PinePart(z);

		if (i>ColorIterationsMin && i<ColorIterations) {
			orbitTrap=min(orbitTrap,vec4(z.x*z.x,z.y*z.y,z.z*z.z,dot(z,z)));
		}
		if (Julia) {
			z+=JuliaC;	
		} else {
			z+=pixelparts;
		}	
		r=length(z);
		i++;
	}
	return (r<Bailout);
}




#preset default
FOV = 0.5
Eye = 0,0,-3.40137
Target = 0,0,0
Up = 0,-1,0
Specular = 0
SpecularExp = 5.455
SpotLight = 0.0627451,0.0352941,0,0.89706
SpotLightDir = 0.80246,0.78126
CamLight = 0.976471,0.972549,0.968627,1.47826
CamLightMin = 0.53012
Fog = 1.84
BaseColor = 0.0156863,0.0156863,0.0156863
OrbitStrength = 0.76623
X = 1,0.482353,0.184314,0.35922
Y = 0.980392,0.87451,0.337255,1
Z = 0.945098,0.733333,0.188235,0.1068
R = 0.952941,0.203922,0,0.2549
BackgroundColor = 0.0117647,0.00392157,0.176471
GradientBackground = 1.33335
CycleColors = false
Cycles = 23.1868
AroundTarget = 0,0
AroundEye = 0,0
UpRotate = 0
EquiRectangular = false
Gamma = 0.5
ToneMapping = 2
Exposure = 1.34694
Brightness = 1
Contrast = 0.9901
Saturation = 1
NormalScale = 0.00024
AOScale = 0.003
Glow = 0.75962
AOStrength = 0.97143
Samples = 222
Stratify = true
DebugInside = false
CentralDifferences = true
SampleNeighbors = true
Near = 0.4956
Far = 5.71704
ShowDepth = false
DebugNormals = false
Iterations = 5
ColorIterations = 3
ColorIterationsMin = 0
manPower = 2
Seed = 2
Julia = false
JuliaC = 0,0,0
Bailout = 22
RotVector1 = 1,1,1
RotAngle1 = 138.751
PixelMult = 1,0,0
#endpreset

#preset test
FOV = 0.5
Eye = 0,0,0
Target = 0,0,1
Up = 0,-1,0
Specular = 0
SpecularExp = 5.455
SpotLight = 0.0627451,0.0352941,0,0.89706
SpotLightDir = 0.80246,0.78126
CamLight = 0.976471,0.972549,0.968627,1.47826
CamLightMin = 0.53012
Fog = 1.84
BaseColor = 0.294118,0.223529,0.141176
OrbitStrength = 0.7
X = 1,0.352941,0.0313725,0.6
Y = 0.968627,0.917647,0.729412,0.5
Z = 1,0.454902,0.0117647,1
R = 0.870588,0.772549,0.568627,0.39
BackgroundColor = 0.0117647,0.00392157,0.176471
GradientBackground = 1.33335
CycleColors = false
Cycles = 23.1868
AroundTarget = 0,0
AroundEye = 0,0
UpRotate = 0
EquiRectangular = false
Gamma = 0.5
ToneMapping = 2
Exposure = 1.34694
Brightness = 1
Contrast = 0.9901
Saturation = 1
NormalScale = 0.00024
AOScale = 0.003
Glow = 0.75962
AOStrength = 0.97143
Samples = 222
Stratify = true
DebugInside = false
CentralDifferences = true
SampleNeighbors = true
Near = 0.4956
Far = 5.71704
ShowDepth = false
DebugNormals = false
Iterations = 5
ColorIterations = 3
ColorIterationsMin = 0
manPower = 2
Seed = 2
Julia = false
JuliaC = 0,0,0
Bailout = 22
RotVector1 = 1,1,1
RotAngle1 = 89.286
PixelMult = 1,0,0
Rot1Iteration = 3
#endpreset