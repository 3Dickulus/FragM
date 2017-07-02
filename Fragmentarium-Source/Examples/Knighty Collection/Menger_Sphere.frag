// http://www.fractalforums.com/programming/project-a-cube-into-a-sphere-%28not-riemann%27s-way%29/
#info Menger Distance Estimator.
#define providesInit
#include "DE-Raytracer.frag"
#include "MathUtils.frag"
#group Menger

// Number of iterations.
uniform int Iterations;  slider[0,10,20]

// Scale parameter. A perfect Menger is 3.0
uniform float Scale; slider[0.00,3,4.00]

uniform vec3 RotVector; slider[(0,0,0),(1,0,0),(1,1,1)]

// Scale parameter. A perfect Menger is 3.0
uniform float RotAngle; slider[0.00,00,360]

// Scaling center
uniform vec3 Offset; slider[(-2,-2,-2),(1,1,1),(2,2,2)]

mat3 rot;
float sc,sr;
void init() {
	rot = rotationMatrix3(normalize(RotVector), RotAngle);
	vec3 o=abs(Offset);
	sc = max(o.x,max(o.y,o.z));
	sr=sqrt(dot(o,o)+1.);
}

float DE(vec3 p)
{
#if 1
	vec3 ap=abs(p);
	float Linf=max(max(ap.x,ap.y),ap.z);//infinity norm
	float L2=length(p);//euclidean norm
	float multiplier=L2/Linf;
	p*=multiplier;//Spherify transform.
	float dd=multiplier*1.6;//to correct the DE. Found by try and error. there should be better formula.
#else
	float dd=1.;
#endif
	float r2=dot(p,p);
	for(int i = 0; i<Iterations && r2<100.; i++){
		p=abs(p);
		if(p.y>p.x) p.xy=p.yx;
      		if(p.z>p.y) p.yz=p.zy;
      		if(p.y>p.x) p.xy=p.yx;
		p.z=abs(p.z-1./3.*Offset.z)+1./3.*Offset.z;
		p=p*Scale-Offset*(Scale-1.); dd*=Scale;
		p=rot*p;
		r2=dot(p,p);	
	}
#if 1
	return (sqrt(r2)-sr)/dd;//bounding volume is a sphere
#else
	p=abs(p); return (max(p.x,max(p.y,p.z))-sc)/dd;//bounding volume is a cube
#endif
}

#preset Default
FOV = 0.62536
Eye = 0.990786,-0.983864,0.939562
Target = -0.294355,0.292298,-0.279136
EquiRectangular = false
FocalPlane = 1
Aperture = 0
Gamma = 2
ToneMapping = 4
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
Detail = -2.84956
DetailAO = -0.5
FudgeFactor = 1
MaxRaySteps = 56
Dither = 0.51754
NormalBackStep = 1
AO = 0,0,0,0.7
Specular = 0.4
SpecularExp = 16
SpecularMax = 10
SpotLight = 1,1,1,0.4
SpotLightDir = 0.1,0.1
CamLight = 1,1,1,1
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 20
Fog = 0
HardShadow = 0
ShadowSoft = 2
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
Iterations = 10
Scale = 3
RotVector = 1,0,0
RotAngle = 0
Offset = 1,1,1
Up = 0.435461,0.286384,0.853439
#endpreset
