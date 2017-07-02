#donotrun

/*
SoC.frag (Sphere of Confusion march) by eiffie
Fast anti-alias, DoF, softshadows and reflections.
License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

Implement this function in fragments that include this file:
float DE(vec3 pos); //accumulate color samples in the variable mcol when bColoring==true
*/
#buffer RGBA32F

//The vertex setup is taken from Syntopia's examples
#info Simple 3D Setup
#camera 3D

#vertex

#group Camera
// Field-of-view
uniform float FOV; slider[0,0.4,2.0] NotLockable
uniform vec3 Eye; slider[(-50,-50,-50),(0,0,-10),(50,50,50)] NotLockable
uniform vec3 Target; slider[(-50,-50,-50),(0,0,0),(50,50,50)] NotLockable
uniform vec3 Up; slider[(0,0,0),(0,1,0),(0,0,0)] NotLockable
uniform vec2 pixelSize;
varying vec3 from;
varying vec3 dir;
varying vec3 Dir;

#ifdef providesInit
void init(); // forward declare
#else
void init() {}
#endif

void main(void)
{
	gl_Position =  gl_Vertex;
	vec2 coord = (gl_ProjectionMatrix*gl_Vertex).xy;
	coord.x*= pixelSize.y/pixelSize.x;
	from = Eye;
	Dir = normalize(Target-Eye);
	vec3 UpOrtho = normalize( Up-dot(Dir,Up)*Dir );
	vec3 Right = normalize( cross(Dir,UpOrtho));
	dir = (coord.x*Right + coord.y*UpOrtho )*FOV+Dir;
	init();
}
#endvertex

#group Camera

#define PI  3.14159265358979323846264
// Camera position and target.
varying vec3 from;
varying vec3 dir;
varying vec3 Dir;
uniform vec2 pixelSize;
uniform float time;
uniform float FocalPlane; slider[0,1,50]
uniform float Aperture; slider[0,0.01,0.1]

#group Post
// for rendering depth to alpha channel in EXR images
// see http://www.fractalforums.com/index.php?topic=21759.msg87160#msg87160
uniform bool DepthToAlpha; checkbox[false];
bool depthFlag = true; // do depth on the first hit not on reflections

#group Raytracer
// Lower this if the system is missing details
uniform float FudgeFactor;slider[0,1,1]
// Maximum number of  raymarching steps.
uniform int MaxRaySteps;  slider[0,100,200]
// Maximum number of shadowmarching steps.
uniform int MaxShadowSteps;  slider[0,32,200]
// Distance to boundary of scene.
uniform float MaxDistance;slider[1.0,10.0,100.0]
// Forward step as a fraction of CoC radius.
uniform float ForwardStep;slider[0.0,0.25,0.5]

#group Light
// The specular intensity
uniform float Specular; slider[0.0,0.5,1.0]
// The specular exponent
uniform float SpecularExp; slider[0.0,16.0,100.0]
// Diffuse contrast
uniform float DiffuseContrast; slider[0.0,0.7,1.0]
// Shadow Contrast
uniform float ShadowContrast; slider[0.0,0.7,1.0]
// The penumbra
uniform float ShadowCone; slider[0.0,0.05,0.5]
// Specular light color
uniform vec3 LightColor; color[1.0,0.9,0.4]
// Direction of light
uniform vec3 LightDir; slider[(-1.0,-1.0,-1.0),(0.5,0.5,-0.5),(1.0,1.0,1.0)]
// Background color
uniform vec3 BGColor; color[0.5,0.5,0.5]
// Blurs the reflections
uniform float ReflectionBlur;slider[0.0,0.5,2.0]

bool bColoring=false;//accumulate colors in DE when true
vec4 mcol=vec4(0.0);//this is the variable to accumulate colors in

float DE(vec3 pos) ; // Must be implemented in other file

#ifdef providesInit
void init(); // forward declare
#else
void init() {}
#endif

//random seed and generator
float randSeed=fract(cos((gl_FragCoord.x+gl_FragCoord.y*117.0+time*10.0)*473.7192451));
float randStep(){//a simple pseudo random number generator based on iq's hash
	return  (0.8+0.2*fract(sin(++randSeed)*43758.5453123));
}
float CircleOfConfusion(float t){//calculates the radius of the circle of confusion at length t
	return max(abs(FocalPlane-t)*Aperture,pixelSize.x*(1.0+t));
}
float linstep(float a, float b, float t){return clamp((t-a)/(b-a),0.,1.);}// i got this from knighty
float FuzzyShadow(vec3 ro, vec3 rd, float coneGrad, float rCoC){
	float t=rCoC*2.0,s=1.0;
	for(int i=0;i<MaxShadowSteps;i++){
		if(s<0.1 || t>MaxDistance)break;
		float r=rCoC+t*coneGrad;//radius of cone
		float d=DE(ro+rd*t)+r*0.5;
		s*=linstep(-r,r,d);
		t+=abs(d)*randStep();
	}
	return clamp(s*ShadowContrast+(1.0-ShadowContrast),0.0,1.0);
}

void main() {
	init();
	vec3 ro=from;
	vec3 rd=normalize(dir);
	vec3 L=normalize(LightDir);
	vec4 col=vec4(0.0);//color accumulator
	float t=DE(ro)*FudgeFactor*randStep(),cor=1.0/dot(rd,Dir);//distance traveled, focus correction
	ro+=rd*t;
	for(int i=0;i<MaxRaySteps;i++){//march loop
		if(col.w>0.9 || t>MaxDistance)break;//bail if we hit a surface or go out of bounds
		float rCoC=CircleOfConfusion(t*cor);//calc the radius of CoC
		float d=DE(ro)+ForwardStep*rCoC;
		if(d<rCoC){//if we are inside add its contribution
			vec3 p=ro-rd*abs(d-rCoC);//back up to border of CoC
			mcol=vec4(0.0);//clear the color trap, collecting color samples with normal deltas
			bColoring=true;//alerts the user to accumulate colors in mcol
			vec2 v=vec2(rCoC*0.333,0.0);//use normal deltas based on CoC radius
			vec3 N=normalize(vec3(-DE(p-v.xyy)+DE(p+v.xyy),-DE(p-v.yxy)+DE(p+v.yxy),-DE(p-v.yyx)+DE(p+v.yyx)));
			bColoring=false;
			if(N!=N)N=-rd;//if we found no gradient... cheat
			if(dot(rd,N)<0.0){
				mcol*=0.1666;
				float sh=FuzzyShadow(p,L,ShadowCone,rCoC);
				vec3 scol=mcol.rgb*(1.0-DiffuseContrast+sh*0.5*DiffuseContrast*(1.0+dot(N,L)));
				scol+=sh*Specular*pow(max(0.0,dot(reflect(rd,N),L)),SpecularExp)*LightColor;
				if(mcol.a>0.01 && d<rCoC*mcol.a){//quick reflections
					rd=reflect(rd,N);d=rCoC*mcol.a;ro=p;t+=ReflectionBlur;
				}
				float alpha=FudgeFactor*(1.0-col.w)*linstep(-rCoC,rCoC,-d);//calculate the mix like cloud density
				col+=vec4(scol*alpha,alpha);//blend in the new color
			}
		}
		d=FudgeFactor*abs(d)*randStep();//add in noise to reduce banding and create fuzz
		ro+=d*rd;//march
		t+=d;
	}//mix in background color
	vec3 scol=BGColor+rd*0.1;
	col.rgb+=scol*(1.0-clamp(col.w,0.0,1.0));
	gl_FragColor = vec4(vec3(clamp(col.rgb,0.0,1.0)), 1.0);

	if(depthFlag) {
		// do depth on the first hit not on reflections
		depthFlag=false;
		// for rendering depth to alpha channel in EXR images
		// see http://www.fractalforums.com/index.php?topic=21759.msg87160#msg87160
		if(DepthToAlpha==true) gl_FragDepth = 1.0/t;
		else
		// sets depth for spline path occlusion
		// see http://www.fractalforums.com/index.php?topic=16405.0
		gl_FragDepth = ((1000.0 / (1000.0 - 0.00001)) +
		(1000.0 * 0.00001 / (0.00001 - 1000.0)) /
		clamp(t, 0.00001, 1000.0));
	}

}


