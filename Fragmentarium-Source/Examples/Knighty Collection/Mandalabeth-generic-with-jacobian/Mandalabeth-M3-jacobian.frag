#version 120
#info Generic M3 Mandalabeth. DE using jacobian. by knighty
#include "DE-Raytracer.frag"
#include "MathUtils.frag"
#group Mandalabeth


// Number of fractal iterations.
uniform int Iterations;  slider[0,9,100]

// Bailout radius
uniform float Bailout; slider[0,5,64]

// parameter
uniform float par; slider[-3,1,3]
//expo
uniform float expo; slider[2,6,20]



//mandala
vec2 mandala(vec2 p, float par, out mat2 j){
	float r=length(p);
	float a=atan(p.y,p.x);
	float r1=pow(r,par), a1=a*par;
	j[0].x=par*r1*cos(a1); j[0].y=par*r1*sin(a1);
	j[1]=j[0].yx;j[1].x*=-1.0;
	return r1*r*vec2(cos(a1+a),sin(a1+a));
}
// Compute the distance from `pos` to the Mandalabeth.
float DE(vec3 p) {
	vec3 v=p;
   	float r2 = dot(v,v),dr = 1.;
	mat3 j=mat3(1.0);  
	 
   	for(int i = 0; i<Iterations && r2<Bailout; i++){
      		vec3 p1=par*p;
		mat2 pj=mat2(1.0);
		mat3 ppj=mat3(0.0);
		mat3 trs=mat3(1.);
			p1+=vec3(mandala((trs*v).xy,expo,pj),0.)*trs;
			ppj=ppj+transpose(trs)*mat3(pj)*trs;
		trs=mat3(vec3(0.,0.,1),vec3(1.,0.,0.),vec3(0.,1.,0.));
			p1+=vec3(mandala((trs*v).xy,expo,pj),0.)*trs;
			ppj=ppj+transpose(trs)*mat3(pj)*trs;
		trs=mat3(vec3(0.,1.,0.),vec3(0.,0.,1),vec3(1.,0.,0.));
			p1+=vec3(mandala((trs*v).xy,expo,pj),0.)*trs;
			ppj=ppj+transpose(trs)*mat3(pj)*trs;

		j=ppj*j+mat3(par);
		v=p1;
		 r2 = dot(v,v);
		orbitTrap = min(orbitTrap, abs(vec4(v,r2)));
   	}
	orbitTrap.w=sqrt(orbitTrap.w);
	float r = sqrt(r2);
	j[0]=abs(j[0]);j[1]=abs(j[1]);j[2]=abs(j[2]);v=j*vec3(1.,1.,1.);
	return abs(0.5*log(r)*r)/max(v.x,max(v.y,v.z));//dr;//
}

#preset default
FOV = 0.4
Eye = 0.179573,0.0192,-0.0859354
Target = -0.718292,-0.0768,0.3437416
Up = -0.0748119,0.995012,0.0659802
EquiRectangular = false
AutoFocus = false
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
DepthToAlpha = false
ShowDepth = false
DepthMagnitude = 1
Detail = -3.53094
DetailAO = -1.57143
FudgeFactor = 0.6544118
MaxDistance = 1000
MaxRaySteps = 1620
Dither = 0.50877
NormalBackStep = 1
AO = 0,0,0,1
Specular = 1
SpecularExp = 44.643
SpecularMax = 10
SpotLight = 1,1,1,0.42308
SpotLightDir = 0.1,-0.9077
CamLight = 1,1,1,2
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 20
Fog = 0.2935455
HardShadow = 0
ShadowSoft = 2
QualityShadows = false
Reflection = 0
DebugSun = false
BaseColor = 0.694118,0.694118,0.694118
OrbitStrength = 0.42857
X = 0.5,0.6,0.6,0.7
Y = 1,0.6,0,0.4
Z = 0.8,0.78,1,0.5
R = 0.4,0.7,1,0.12
BackgroundColor = 0.5843137,0.5843137,0.5843137
GradientBackground = 0.3
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 9
Bailout = 5
par = 1
expo = 6
#endpreset
