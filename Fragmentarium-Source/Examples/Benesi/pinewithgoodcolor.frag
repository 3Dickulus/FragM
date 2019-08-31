
#define providesInside
#define providesInit
//#include "DE-Raytracer.frag"
#include "Fast-Raytracer-with-Palette.frag"
//#include "DE-PK-1.1.9.frag"
//#include "DE-Raytracer-v0.9.10.frag"
//#include "DE-Kn2exp.frag"
//#include "DE-Kn9.frag"  doesn't work

//#define providesColor
#include "MathUtils.frag"
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
//if color iteration is 0, takes the normal minimum orbit of all iterations.  If it is set to a specific number, takes the orbit of that iteration only.
uniform int ColorIteration;  slider[0,0,100]

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
	dr*=T3Scale;
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
	
	
		orbitTrap=min(vec4(pos,length(pos)),
									vec4(10.,10.,10.,10.));
		

	//orbitTrap= vec4(10.0);
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
			if (ColorIteration==0  || ColorIteration>=Iterations) {
					orbitTrap=min(orbitTrap,vec4(z.x*z.x,z.y*z.y,z.z*z.z,dot(z,z)));
				} else if (i==ColorIteration) {
					orbitTrap=vec4(z,length(z));
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



#preset lolcolors
FOV = 0.4
Eye = 2.562139,-0.0670966,0.4123994
Target = -4.418752,0.34414,0.0992925
DepthToAlpha = false
depthMag = 1
ShowDepth = false
AntiAlias = 1
Detail = -3.7
DetailAO = -0.5
FudgeFactor = 0.8
MaxRaySteps = 181
BoundingSphere = 2
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,0.7
Specular = 4
SpecularExp = 16
SpotLight = 0.7529412,1,0.8980392,1
SpotLightDir = -0.4693878,0.4285714
CamLight = 1,1,1,1
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 20
Fog = 0
HardShadow = 0
ShadowSoft = 2
Reflection = 0
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
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
paletteColoring = true
pBaseColor = 0,0,0
BaseStrength = 0
cSpeed = 6
pOffset = 0
color0 = 1,1,1
Dist0to1 = 1
color1 = 1,0,0.06666667
Dist1to2 = 1
color2 = 0,0.04705882,0.9960784
Dist2to3 = 1
color3 = 0.2313725,1,0
Dist3to0 = 1
orbitStrengthXYZR = 0,0,0,1
Iterations = 6
ColorIteration = 3
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
SphereIteration = 10
T2offset = 2
T2Scale = 1
T2Iteration1 = 5
T4offset = 2
T4Scale = 1
T4Iteration1 = 5
T3offset = 2
T3Scale = 2
T3Iteration1 = 10
T5boffset = 2
T5bScale = 1
T5bIteration1 = 5
Sharp0to1 = false
Sharp1to2 = true
Sharp2to3 = false
Sharp3to0 = false
Up = 0.0589955,0.9982494,-0.0042291
#endpreset

#preset colorstest
FOV = 0.4
Eye = -0.2074053,0.0261624,-0.1871428
Target = 0.6009195,-0.5656118,6.740802
DepthToAlpha = false
depthMag = 1
ShowDepth = false
AntiAlias = 1
Detail = -5
DetailAO = -0.5
FudgeFactor = 0.95
MaxRaySteps = 411
BoundingSphere = 2
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,0.7
Specular = 2
SpecularExp = 16
SpotLight = 1,0.6470588,0.07843137,1
SpotLightDir = 0.0204082,0.877551
CamLight = 1,1,1,1.405405
CamLightMin = 0.2352941
Glow = 1,1,1,0
GlowMax = 20
Fog = 0
HardShadow = 0
ShadowSoft = 2
Reflection = 0
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
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 5
ColorIterationsMin = 0
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
SphereIteration = 10
T2offset = 2
T2Scale = 1
T2Iteration1 = 5
T4offset = 2
T4Scale = 1
T4Iteration1 = 5
T3offset = 2
T3Scale = 2
T3Iteration1 = 10
T5boffset = 2
T5bScale = 1
T5bIteration1 = 5
paletteColoring = true
pBaseColor = 0,0,0
BaseStrength = 0.3
cSpeed = 4
color0 = 0.9490196,0,0.9019608
Dist0to1 = 1
color1 = 0.03137255,0,1
Dist1to2 = 1
color2 = 0,0.6039216,0.6980392
Dist2to3 = 1
color3 = 0.01568627,1,0
Dist3to0 = 1
orbitStrengthXYZR = 0,0,0,1
ColorBackFromMaxIter = 2
Up = -0.0428541,0.99502,0.0899931
OrbitTrapMode = 1
#endpreset