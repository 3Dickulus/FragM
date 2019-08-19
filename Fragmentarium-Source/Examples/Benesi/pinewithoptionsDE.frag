
#define providesInside
#define providesInit
#include "MathUtils.frag"
//#include "DE-Raytracer.frag"
#include "Fast-Raytracer.frag"
//#include "DE-PK-1.1.9.frag"
//#include "DE-Raytracer-v0.9.10.frag"
//#include "DE-Kn2.frag"
//#include "DE-Kn9.frag"  doesn't work

//#define providesColor
/*const*/	float sr32=sqrt(3./2.);
/*const*/	float sr23=sqrt(2./3.);
/*const*/	float sr13=sqrt(1./3.);
/*const*/	float sr12=sqrt(.5);


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

uniform int SphereIteration;slider[0,10,20]

uniform float T2offset;slider[-5,2.,5]
uniform float T2Scale;slider[-5,1.,5]
uniform int T2Iteration1;slider[1,5,20]
//uniform int T2Iteration2;slider[1,10,20]
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
void Transform2(inout vec3 z,inout float dr,in int i) {
	float tx=z.x*sr23-z.z*sr13;
	z.z=z.x*sr13 + z.z*sr23;   
	z.x=tx*sr12-z.y*sr12;               
	z.y=tx*sr12+z.y*sr12; 
	
	z=vec3(abs(sqrt(z.y*z.y+z.z*z.z)-T2offset),abs(sqrt(z.x*z.x+z.z*z.z)-T2offset),abs(sqrt(z.x*z.x+z.y*z.y)-T2offset));
	z*=T2Scale;
	dr*=T2Scale;

	tx=z.x*sr12+z.y*sr12;
	z.y=-z.x*sr12+z.y*sr12; 
	z.x=tx*sr23+z.z*sr13;
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
	orbitTrap= vec4(10.0);
	vec3 z=pos;
	vec3 pixelparts=vec3(pos.x*PixelMult.x,pos.y*PixelMult.y,pos.z*PixelMult.z);
	int i=1;
	float r=length(z);
	float dr=1.0;
	
	while(r<Bailout && (i<Iterations)) {
			
		//if (i==T4Iteration1 || i==T4Iteration2) {Transform4(z,i);}
		//if (i==T3Iteration1 || i==T3Iteration2) {Transform3(z,i);}
			if (i==T2Iteration1) {Transform2(z,dr,i);}
			if (i==T3Iteration1) {Transform3(z,dr,i);}
			if (i==T4Iteration1) {Transform4(z,dr,i);}
			if (i==T5bIteration1) {Transform5b(z,dr,i);}
			//if (i==2) convert3(z);
			
			Transform1(z,dr,i);
			if (i==SphereIteration) unSpheric(z);
			PinePart(z);
			if (i==SphereIteration) Spheric(z);
		if (i>ColorIterationsMin && i<ColorIterations) {
			orbitTrap=min(orbitTrap,vec4(z.x*z.x,z.y*z.y,z.z*z.z,dot(z,z)));
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




#preset default
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

#preset DE-Raytracer-v0.9.10
FOV = 0.62536
Eye = -3.4,0.0149438,0.0988771
Target = -0.4,0.0149438,0.0988771
Detail = -3.5
FudgeFactor = 1
BoundingSphere = 2
Dither = 1
NormalBackStep = 2
AO = 0,0,0,0.85185
Specular = 1.6456
SpecularExp = 1
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
Iterations = 6
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
T3Iteration1 = 6
T5boffset = 2
T5bScale = 2
T5bIteration1 = 8
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
DetailAO = 0
MaxRaySteps = 444
SpecularMax = 8
Up = 0,-0.9773935,0.1477186
#endpreset

#preset Knighty2DE
FOV = 0.4
Eye = -2.293522,-0.004899,-1.009766
Target = 5.494852,-0.3418392,5.253436
DepthToAlpha = false
Detail = -3.8
DetailAO = 0
FudgeFactor = 1
Dither = 1
NormalBackStep = 1
AO = 0,0,0,0.6
CamLight = 1,1,1,1
CamLightMin = 0.5
Glow = 1,1,1,0
GlowMax = 20
BaseColor = 0.0745098,0.7647059,0
OrbitStrength = 1
X = 0.2980392,0.9607843,0.5411765,1
Y = 0.4039216,1,0.9137255,1
Z = 0.1372549,0.4666667,1,1
R = 0.4,0.7,1,1
BackgroundColor = 0,0,0.05098039
GradientBackground = 1
CycleColors = false
Cycles = 5.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 6
ColorIterations = 4
ColorIterationsMin = 0
manPower = 2
Julia = false
JuliaC = 0,0,0
PixelMult = 1,0,0
Bailout = 2
T1offset = 2
T1Scale = 2
RotVector1 = 1,1,1
RotAngle1 = 225
Rot1Iteration = 3
T4offset = 1.4
T4Scale = 1.4
T4Iteration1 = 4
T3offset = 2
T3Scale = 2
T3Iteration1 = 10
T5boffset = 2
T5bScale = 1
T5bIteration1 = 6
FocalPlane = 1
Aperture = 0
InFocusAWidth = 0
ApertureNbrSides = 2
ApertureRot = 0
ApStarShaped = false
Gamma = 1
ToneMapping = 1
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 0
AntiAliasScale = 0
Bloom = true
BloomIntensity = 0.5
BloomPow = 1
BloomTaps = 4
MaxDistance = 3
AoCorrect = 0
Specular = 0.4
SpecularExp = 16
Reflection = 1,1,1
SpotGlow = true
SpotLight = 1,1,1,1
LightPos = 3.333333,0,0
LightSize = 0.4
LightFallOff = 0.1
LightGlowRad = 1.3
LightGlowExp = 1
HardShadow = 0
HF_Fallof = 1.1
HF_Const = 0
HF_Intensity = 0
HF_Dir = 0,0,1
HF_Offset = 0
HF_Color = 1,1,1,1
HF_Scatter = 0
HF_Anisotropy = 0,0,0
HF_FogIter = 1
HF_CastShadow = false
CloudScale = 1
CloudFlatness = 0
CloudTops = 1
CloudBase = -1
CloudDensity = 1
CloudRoughness = 1
CloudContrast = 1
CloudColor = 0.65,0.68,0.7
SunLightColor = 0.7,0.5,0.3
MaxRaySteps = 116
ReflectionsNumber = 0
ShadowSoft = 0
WindDir = 0,0,1
WindSpeed = 1
Up = 0.0231671,0.9994201,0.024957
#endpreset

#preset FastandFunny
FOV = 0.4
Eye = -3.512018,0.224707,2.70213
Target = 4.185728,-0.2592076,-3.662658
DepthToAlpha = false
depthMag = 1
ShowDepth = false
AntiAlias = 1
Detail = -3.5
DetailAO = 0
FudgeFactor = 0.5
MaxRaySteps = 336
BoundingSphere = 2
Dither = 0.5
NormalBackStep = 2
AO = 0,0,0,0.7
Specular = 2
SpecularExp = 26
SpotLight = 0.772549,1,1,0.4
SpotLightDir = 1,0.1
CamLight = 1,1,1,1.5
CamLightMin = 0.2
Glow = 0.745098,0.9764706,1,0.2
GlowMax = 4
Fog = 0
HardShadow = 0
ShadowSoft = 2
Reflection = 0
BaseColor = 0.8235294,0.9764706,0.9647059
OrbitStrength = 0.4545455
X = 0.3333333,0.3333333,0.4980392,0.3
Y = 0.5647059,0.6117647,0.6313725,0.4
Z = 0.6156863,0.1490196,1,0.5
R = 0.6,1,0.7803922,0.12
BackgroundColor = 0.003921569,0.007843137,0.01176471
GradientBackground = 0.3
CycleColors = false
Cycles = 1.1
EnableFloor = true
FloorNormal = 0,0,0
FloorHeight = 2.5
FloorColor = 0,0.04705882,1
Iterations = 6
ColorIterations = 3
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
Rot1Iteration = 1
T2offset = 2
T2Scale = 1
T2Iteration1 = 8
T4offset = 2
T4Scale = 1
T4Iteration1 = 10
T3offset = 2
T3Scale = 2
T3Iteration1 = 10
T5boffset = 2
T5bScale = 1
T5bIteration1 = 10
Up = 0.6374076,0.0051081,0.7705096
#endpreset