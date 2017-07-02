#donotrun

#info
#info NODE-Raytracer 0.9.1
#info
#info By Pablo Roman Andrioli (Kali)
#info (Some parts based on Syntopia's raytracers)
#info
#info Run in "continuous" mode and set enough frames
#info

#camera 3D

#vertex
#group Camera

uniform float FOV; slider[0,0.4,2.0] NotLockable
uniform vec3 Eye; slider[(-50,-50,-50),(0,0,-10),(50,50,50)] NotLockable
uniform vec3 Target; slider[(-50,-50,-50),(0,0,0),(50,50,50)] NotLockable
uniform vec3 Up; slider[(0,0,0),(0,1,0),(0,0,0)] NotLockable
uniform vec2 pixelSize;

varying vec3 from; // out
varying vec2 coord; // out
varying vec2 viewcoord; // out
varying vec2 globalcoord; // out
varying float zoom; // out
varying vec3 dir; // out
varying vec3 Dir; // out
varying vec3 up; // out
varying vec3 right; // out
void main(void)
{
	gl_Position =  gl_Vertex;
	coord = (gl_ProjectionMatrix*gl_Vertex).xy;
	coord.x*= pixelSize.y/pixelSize.x;
	vec2 ps =vec2(pixelSize.x*gl_ProjectionMatrix[0][0], pixelSize.y*gl_ProjectionMatrix[1][1]);
	zoom = length(ps);
	from = Eye;
	Dir = normalize(Target-Eye);
	up = Up-dot(Dir,Up)*Dir;
	up = normalize(up);
	right = normalize( cross(Dir,up));
	dir = (coord.x*right + coord.y*up )*FOV+Dir;
	viewcoord=(gl_Vertex.xy+vec2(1.0))*0.5;
	globalcoord=coord;
}
#endvertex

mat3  rotation(vec3 v, float angle)
{
	float c = cos(angle);
	float s = sin(angle);

	return mat3(c + (1.0 - c) * v.x * v.x, (1.0 - c) * v.x * v.y - s * v.z, (1.0 - c) * v.x * v.z + s * v.y,
		(1.0 - c) * v.x * v.y + s * v.z, c + (1.0 - c) * v.y * v.y, (1.0 - c) * v.y * v.z - s * v.x,
		(1.0 - c) * v.x * v.z - s * v.y, (1.0 - c) * v.y * v.z + s * v.x, c + (1.0 - c) * v.z * v.z
		);
}

uniform vec3 Eye;
uniform vec3 Target;
uniform vec3 Up;
uniform float FOV;
in vec3 from,dir,right,up,Dir;
in vec2 viewcoord, globalcoord;
in float zoom;
uniform vec2 pixelSize;
uniform vec2 globalPixelSize;
vec3 hitcolor;
vec3 hitnormal;
vec3 lightdir1, lightdir2;

#buffer RGBA32F
uniform sampler2D backbuffer;
uniform int subframe;

// #group Post
// // Available when using exr image filename extention
// uniform bool DepthToAlpha; checkbox[false];
// // for rendering depth to alpha channel in EXR images, set in DE-Raytracer.frag
// // see http://www.fractalforums.com/index.php?topic=21759.msg87160#msg87160
// bool depthFlag = true; // do depth on the first hit not on reflections

#group Raytracer
uniform float MinDistance; slider[0,0,5]
uniform float MaxDistance; slider[0,5,20]
uniform float FixedStep; slider[0.001,.05,.2]
uniform int SearchSteps; slider[0,20,50]
uniform float NoiseReduction; slider[0,0.5,.95]
uniform float DEAdjust; slider[0,.01,.1]


#group Lights
uniform float Ambient; slider[0,.2,1]
uniform vec4 SpotLight1; color[0,.5,1,1,1,1]
uniform vec4 SpotLight2; color[0,0,1,1,1,1]
uniform vec2 LightDir1; slider[(0,0),(0,0),(360,360)]
uniform vec2 LightDir2; slider[(0,0),(0,0),(360,360)]
uniform bool ShowLightPos; checkbox[false]
uniform float Diffuse; slider[0,1,1]
uniform float Specular; slider[0,.5,1]
uniform float SpecularExp; slider[0,20,100]
uniform float DarkLevel; slider[0,0,1]

#group AO
uniform bool SSAO; checkbox[true]
uniform float SSAOTreshold; slider[0,0,.05]
uniform bool AODither; checkbox[false]
uniform float AOStrength; slider[0,0,1]
uniform float AORadius; slider[1,5,50]
uniform int AOSamples; slider[1,200,1000]


#group Color
uniform vec3 BaseColor; color[1.0,1.0,1.0]
uniform vec3 BackgroundUp; color[1.0,1.0,1.0]
uniform vec3 BackgroundDown; color[0.0,0.0,0.0]
uniform vec3 AmbientUp; color[1.0,1.0,1.0]
uniform vec3 AmbientDown; color[0.5,0.5,0.5]
uniform bool BackgroundAmbient; checkbox[false]
uniform float ColoringMix; slider[0,.5,1]
uniform float ColorDensity; slider[0,.5,1]
uniform float ColorOffset; slider[0,0,1]
uniform vec3 Color1; color[1.0,0.0,0.0]
uniform vec3 Color2; color[0.0,1.0,0.0]
uniform vec3 Color3; color[0.0,0.0,1.0]
uniform vec3 Color4; color[1.0,0.5,0.5]
uniform vec3 Color5; color[0.5,0.1,0.5]
uniform vec3 Color6; color[0.5,0.5,1.0]


#group Volumetric
uniform vec4 Volumetric; color[0,0,1,1,1,1]
uniform float VolumetricExp; slider[0,1,5]
uniform bool VolumetricOnly; checkbox[false]
uniform float VolumetricMix; slider[0,0,1]
uniform float VolumetricAdd; slider[0,0,1]
uniform float VolAO; slider[0,0,1]
uniform bool InverseVolAO; checkbox[false]
uniform float VolBackgroundMix; slider[0,0,1]
uniform float VolDistanceFade; slider[0,0,1]
uniform float VolDistanceFadeExp; slider[0,1,5]
uniform float VolInsideFade; slider[0,0,1]
uniform float VolInsideFadeExp; slider[0,1,5]
uniform float VolColorMix; slider[0,0,1]
uniform float VolColorDensity; slider[0,.5,1]
uniform float VolColorOffset; slider[0,0,1]
uniform vec3 VolColor1; color[1.0,0.0,0.0]
uniform vec3 VolColor2; color[0.0,1.0,0.0]
uniform vec3 VolColor3; color[0.0,0.0,1.0]


#group Effects
uniform vec4 Fog; color[0,0,10,1,1,1]
uniform float FogExp; slider[0,2,10]
uniform float FogBackMix; slider[0,.5,1]
uniform float FogGlowMix; slider[0,.5,1]
uniform float FogVolumetricMix; slider[0,0,1]
uniform vec4 Glow; color[0,0,1,1,1,1]
uniform int GlowSamples; slider[1,100,200]
uniform float GlowRadius; slider[0,.5,1]
uniform float InnerGlowTreshold; slider[0,.1,.5]
uniform float GlowContextBright; slider[0,.5,1]
uniform float GlowContextColor; slider[0,.5,1]
uniform float Blur; slider[0,0,1]
uniform float LightBlur; slider[0,0,1]
uniform float LightBlurTreshold; slider[0.5,.75,1]
uniform float DOF; slider[0,0,1]
uniform float DOFexp; slider[0,1,2]
uniform float Focus; slider[0,0,1]
uniform bool ShowFocus; checkbox[false]
uniform int BlurSamples; slider[1,20,100]



#define PI  3.14159265358979323846264


bool formula(vec3 p);
bool inside(vec3 p);
float DE(vec3 p);

mat3 rotmat(vec3 v, float angle)
{
	float c = cos(radians(angle));
	float s = sin(radians(angle));

	return mat3(c + (1.0 - c) * v.x * v.x, (1.0 - c) * v.x * v.y - s * v.z, (1.0 - c) * v.x * v.z + s * v.y,
		(1.0 - c) * v.x * v.y + s * v.z, c + (1.0 - c) * v.y * v.y, (1.0 - c) * v.y * v.z - s * v.x,
		(1.0 - c) * v.x * v.z - s * v.y, (1.0 - c) * v.y * v.z + s * v.x, c + (1.0 - c) * v.z * v.z
		);
}

float rand(vec2 co){
	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

mat3 align(vec3 fw, vec3 up2) {
	vec3 rt=normalize(cross(fw,up2));
	up2=cross(rt,fw);
	return mat3(-rt,up2,fw);
}


vec3 getcolor(float index) {
	float cx=index*ColorDensity+ColorOffset*PI*6;
	vec3 col;
	float ps=PI/3;
	float f1=max(0,sin(cx)-.5)/.5;
	float f2=max(0,sin(cx+ps)-.5)/.5;
	float f3=max(0,sin(cx+ps*2)-.5)/.5;
	float f4=max(0,sin(cx+ps*3)-.5)/.5;
	float f5=max(0,sin(cx+ps*4)-.5)/.5;
	float f6=max(0,sin(cx+ps*5)-.5)/.5;
	col=mix(Color1,Color2,f1);
	col=mix(col,Color3,f2);
	col=mix(col,Color4,f3);
	col=mix(col,Color5,f4);
	col=mix(col,mix(Color6,Color1,f6),f5);
	col=mix(BaseColor,col,ColoringMix);
	return col;
}

vec3 getvcolor(float index) {
	float cx=index*VolColorDensity+VolColorOffset*PI*3;
	vec3 col;
	float ps=PI/1.5;
	float f1=max(0,sin(cx));
	float f2=max(0,sin(cx+ps));
	float f3=max(0,sin(cx+ps*2));
	col=mix(VolColor1,VolColor2,f1);
	col=mix(col,mix(VolColor3,VolColor1,f3),f2);
	col=mix(Volumetric.xyz,col,VolColorMix);
	return col;
}

float focus(float depth) {
	float dist=abs(depth-MaxDistance*Focus)+.005;
	return max(0,1-exp(-DOF*pow(dist,DOFexp)));
}

vec3 blur(vec3 col, vec2 pix, float depth) {
	float dof=focus(depth)*10+max(Blur*5,LightBlur*5);
	float islblur=sign(LightBlur)*BlurSamples/20*(1.5-sign(DOF+Blur));
	vec3 av=vec3(0.)+col*islblur;
	vec2 rv=pix*subframe;
	float samples=islblur;
	for (float df=1; df<=BlurSamples; df++) {
		vec2 rr=vec2(rand(rv*df*.10687956)-.5,rand(rv*df*.2598765216)-.5)*2;
		vec2 rp=rr*dof*globalPixelSize;
		vec4 sp=(abs(rp.x)<globalPixelSize.x/2.1&&abs(rp.y)<globalPixelSize.y/2.1)?vec4(col,depth):texture2D(backbuffer,pix+rp);
		float sblur=LightBlur*max(0,length(sp.xyz)-LightBlurTreshold)*length(rr);
		float f=focus(sp.w)+Blur+sblur;
		av+=sp.xyz*f;
		samples+=f;
	}
	av/=samples;
	return av;
}


vec3 glow(vec3 col, vec2 pix, float depth) {
	vec3 av=vec3(0.1);
	vec2 rv=pix*subframe;
	float samples=1;
	float d=2;
	float glorad=GlowRadius*globalPixelSize*200;
	for (float df=1; df<=GlowSamples; df++) {
		vec2 rr=normalize(vec2(rand(rv*df*.10687956)-.5,rand(rv*df*.2598765216)-.5)*2);
		float rl=rand(rv*df*.32158658);
		vec2 rp=rr*rl*glorad;
		vec4 sp=(abs(rp.x)<globalPixelSize.x/2&&abs(rp.y)<globalPixelSize.y/2)?vec4(col,MaxDistance):texture2D(backbuffer,pix+rp);
		if (depth-sp.w>InnerGlowTreshold || (depth==MaxDistance && sp.w<MaxDistance)) {
			d=min(d,rl);
			av+=sp.xyz;
			samples++;
		}
	}
	av/=samples;
	float conglowb=mix(1,length(av),GlowContextBright*.99);
	vec3 conglowc=mix(Glow.xyz,normalize(av),GlowContextColor*.99);
	return min(vec3(2,2,2),conglowb*conglowc*smoothstep(1,0,pow(d,.5))*Glow.w*2);
}


vec3 Normal(vec2 pix) {
	vec3 n;
	vec3 e=vec3(pixelSize,0);
	float w=texture2D(backbuffer,pix).w;
	float dx1=texture2D(backbuffer,pix-e.xz).w-texture2D(backbuffer,pix).w;
	float dx2=texture2D(backbuffer,pix).w-texture2D(backbuffer,pix+e.xz).w;
	float dy1=texture2D(backbuffer,pix-e.zy).w-texture2D(backbuffer,pix).w;
	float dy2=texture2D(backbuffer,pix).w-texture2D(backbuffer,pix+e.zy).w;
	vec3 n1=normalize(zoom*right+max(dx1,dx2)*-Dir/w);
	vec3 n2=normalize(zoom*up+max(dy1,dy2)*-Dir/w);
	return normalize(cross(n1,n2));

}



float SAO(vec2 pix, float depth) {
	float aofactor=1;
	if (AOStrength>0 && depth<MaxDistance) {
		float AOthold=SSAOTreshold*depth+.00001;
		float aoc=0;
		vec2 pr=vec2(1.23254,1.854321)*(AODither?viewcoord*subframe*.25321856:vec2(1.126586));
		for (float aos=1; aos<=AOSamples; aos++) {
			vec2 aor=aos*pr;
			vec2 aorad=vec2(rand(aor*1.354856)-.5,rand(aor*1.232548)-.5)*AORadius*globalPixelSize*2;
			float nd=texture2D(backbuffer,pix+aorad).w;
			float dif=depth-nd;
			aoc+=clamp(dif/AOthold,0,1);
		}
		aofactor=max(0,1-aoc/AOSamples*AOStrength*2);
	}
	return aofactor;
}


float AO(vec3 p) {
	float aofactor=1;
	if (AOStrength>0) {
		vec2 pbr=vec2(subframe*.125685)*(AODither?viewcoord:vec2(1.));
		float aoc=0;
		float aorad=AORadius*.005;
		for (float s=1; s<AOSamples; s++) {
			vec2 sr=pbr*s;
			vec3 rv=vec3(rand(sr*vec2(1.2568575,2.2548565)),rand(sr*vec2(1.4567545,1.7246545)),rand(sr*vec2(1.355565,3.254261)));
			rv=normalize(rv-vec3(.5));
			float l=rand(sr*vec2(1.112354834,0.9843223));
			if (formula(p+rv*aorad*l)) aoc+=1;
		}
		aofactor=max(0,1-max(0,aoc-AOSamples/2.1)/AOSamples*10*AOStrength);
	}
	return aofactor;
}


vec3 Light(vec2 pix, vec3 ldir, float ao) {
	vec3 cdir=normalize(Eye-Target);
	vec3 n=hitnormal;
	float diff=max(0.0,dot(-n,ldir))*Diffuse;
	vec3 r = reflect(cdir,n);
	float s = max(0.0,dot(ldir,r));
	float spec = (SpecularExp<=0.0) ? 0.0 : pow(s,SpecularExp)*max(0,Specular)*2;
	return vec3(diff)*hitcolor*ao+vec3(spec)*(.5+hitcolor/2);
}

vec4 orbitTrap;

float colorindex=0;
float vcolorindex=0;
float volumetric=-1;
bool side;

vec4 trace (vec3 f,vec3 d) {
	orbitTrap=vec4(10000.);
	vec3 prevc=texture2D(backbuffer,viewcoord).xyz;
	float prevr=texture2D(backbuffer,viewcoord).w;
	if (subframe==1) prevr=MaxDistance;
	vec3 newc;
	vec3 c;
	float rt=MinDistance+FixedStep*rand(viewcoord*vec2(subframe*1.1654985,subframe*1.25486548));
	float r=MaxDistance;
	float rit=0;
	bool i=formula(f+rt*d);
	side=i;
	vec3 vol=vec3(0.);
	float vs=1;
	while (rt<MaxDistance) {
		volumetric=-1;
		bool it=formula(f+rt*d);
		if (Volumetric.w>0) {
			if (rt<prevr && it && r==MaxDistance) {
				r=rt;
				i=true;
			}
			if (volumetric>-1) {
				vec3 vc;
				vc=pow(volumetric,VolumetricExp)/VolumetricExp*(VolColorMix>0?getvcolor(vcolorindex):Volumetric.xyz);
				vc*=exp(-VolDistanceFade*pow(rt,VolDistanceFadeExp));
				if (it) {
					vc*=exp(-VolInsideFade*pow(rit,VolInsideFadeExp)*10);
					rit+=FixedStep;
				}
				vol+=vc;
				vs++;
			}
		} else {
			if (rt<prevr && it) {
				r=rt;
				i=true;
				break;
			}
		}
		rt+=FixedStep;
	}
	vol*=Volumetric.w/vs;
	if (r<prevr) {
		int ss=0;
		float s=FixedStep;
		float bs=r;
		while (ss<SearchSteps) {
			ss+=1;
			s=s/2;
			if (i) r-=s; else r+=s;
			i=formula(f+r*d);
			if (i) bs=r;
		}
		r=bs;
	} else {r=prevr;}
	i=formula(f+r*d);

	if (orbitTrap==vec4(10000)) {
		hitcolor=ColoringMix>0?getcolor(colorindex):BaseColor;
	} else {hitcolor=BaseColor;	}

	hitnormal=Normal(viewcoord);
	float aofactor=1;
	if (subframe>1) aofactor=SSAO?SAO(viewcoord,r):AO(f+r*d);
	vec3 backg=mix(BackgroundDown,BackgroundUp,(globalcoord.y+1)/2);
	if (Volumetric.w>0) backg=mix(backg,vol,VolBackgroundMix);
	if (r<MaxDistance) {
		newc=Light(viewcoord,normalize(lightdir1),aofactor)*SpotLight1.w*SpotLight1.xyz*2;
		newc+=Light(viewcoord,normalize(lightdir2),aofactor)*SpotLight2.w*SpotLight2.xyz*2;
		vec3 AmbientD=BackgroundAmbient?BackgroundDown:AmbientDown;
		vec3 AmbientU=BackgroundAmbient?BackgroundUp:AmbientUp;
		newc+=Ambient*hitcolor*aofactor*mix(AmbientD,AmbientU,max(0,dot(hitnormal,up)));
		newc=max(vec3(DarkLevel*hitcolor),newc);
	} else {
		newc=backg;
	}

	if (Volumetric.w>0 && volumetric>-1) {
		vol=mix(vol,vol*(InverseVolAO?1/aofactor:aofactor),VolAO);
		vol=min(vec3(2,2,2),vol);
		newc=VolumetricOnly?vol:mix(newc,vol, VolumetricMix);
		newc+=vol*VolumetricAdd*2;
	}

	if (subframe>2) {
		vec3 gl=vec3(0.);
		if (Glow.w>0) {gl=glow(newc,viewcoord, r); newc+=gl;}
		vec3 fogc=Fog.xyz;
		fogc=mix(fogc,backg,FogBackMix);
		if (Volumetric.w>0) fogc=mix(fogc,vol,FogVolumetricMix);
		fogc+=gl*FogGlowMix;
		newc=mix(newc,fogc,1-exp(-pow(Fog.w/FogExp,4)*pow(r,FogExp)));
		if (ShowFocus) newc=(abs(r-Focus*MaxDistance)<.01?vec3(1,1,.5):newc);
	}
	if (subframe>3) {
		if (DOF+Blur+LightBlur>0) newc=blur(newc,viewcoord,r);
		newc=mix(newc,prevc,SSAO?NoiseReduction:.9);
	}

	newc=subframe>1?newc:vec3(pow(1.1-r/MaxDistance,4)*2)*(abs(r-Focus*MaxDistance)<.01?vec3(.5,.5,1):vec3(1.));
	return vec4(newc*2.0,r);
}

float torusDE( vec3 p, vec2 t )
{
  vec2 q = vec2(length(p.xz)-t.x,p.y);
  return length(q)-t.y;
}


float lightpDE(vec3 pos,float l) {
	pos=pos.x*right+pos.y*up+pos.z*Dir;
	vec3 p=pos;
	vec3 ldir=l==1?normalize(lightdir1):normalize(lightdir2);
	if (abs(ldir)!=vec3(0,0,1)) {
		p*=align(normalize(cross(ldir,vec3(0,1,0))),normalize(ldir));
	} else {
		p*=align(normalize(cross(ldir,vec3(0,1,0))),normalize(ldir));
	}
	float d= max(length(p+vec3(0,.5,0))-.5,length(p.xz)-.02);
	d=min(d,length(p+vec3(0,1,0))-.1);
	d=min(d,torusDE(p.xyz,vec2(.5*(.5+l/10),.01)));
	return d;
}

float lightposDE(vec3 pos, inout float l) {
	float lde1=lightpDE(pos,1);
	float lde2=lightpDE(pos,2);
	float globe=length(pos)-.25;
	if (SpotLight1.w==0) lde1=globe;
	if (SpotLight2.w==0) lde2=globe;
	float d1=min(lde1,lde2);
	float d2=min(d1,globe);
	l=lde1<lde2?1.:2.;
	l=d2==globe?0.:l;
	return d2;
}


vec3 DEnormal(vec3 pos, float normalDistance) {
	float l;
	normalDistance = max(normalDistance*0.5, 1.0e-7);
	vec3 e = vec3(0.0,normalDistance,0.0);
	vec3 n = vec3(lightposDE(pos+e.yxx,l)-lightposDE(pos-e.yxx,l),
		lightposDE(pos+e.xyx,l)-lightposDE(pos-e.xyx,l),
		lightposDE(pos+e.xxy,l)-lightposDE(pos-e.xxy,l));
	n =  normalize(n);
	return n;
}

vec3 DEtrace() {
	vec3 fr=vec3(0,0,-2);
	vec3 hit = vec3(0.0);
	vec3 direction = normalize(vec3(globalcoord*.7,1));

	float dist = 0.0;
	float totalDist = 0.;

	int steps;
	float l=0;

	for (steps=0; steps<200; steps++) {
		vec3 p = fr + totalDist * direction;
		dist=lightposDE(p,l);
		totalDist += dist;
		if (dist < .01) break;
		if (totalDist > 3) break;
	}

	vec3 hitColor;
	vec3 backColor = vec3(0.);

// 	if(depthFlag) {
// 		// do depth on the first hit not on reflections
// 		depthFlag=false;
// 		// for rendering depth to alpha channel in EXR images
// 		// see http://www.fractalforums.com/index.php?topic=21759.msg87160#msg87160
// 		if(DepthToAlpha==true) gl_FragDepth = depth;
// 		else
// 		// sets depth for spline path occlusion
// 		// see http://www.fractalforums.com/index.php?topic=16405.0
// 		gl_FragDepth = ((1000.0 / (1000.0 - 0.00001)) +
// 		(1000.0 * 0.00001 / (0.00001 - 1000.0)) /
// 		clamp(totalDist, 0.00001, 1000.0));
// 	}

	if ( dist < .01) {
		hit = fr + totalDist * direction;
		vec3 n= DEnormal(hit-.01*direction, .01);
		vec3 spotDir = vec3(0,-.5,1);
		spotDir = normalize(spotDir);
		vec3 r = spotDir - 2.0 * dot(n, spotDir) * n;
		float s = max(0.0,dot(direction,-r));
		float diffuse = max(0.0,dot(-n,spotDir));
		float specular = pow(s,10);
		vec3 col;
		if (l==0) col=vec3(1.);
		if (l==1) col=vec3(.75,.75,1);
		if (l==2) col=vec3(1,.75,.75);
		return col*(diffuse*.6+specular*.7+.1);
	}
	else {
		return vec3(0.);
	}
}


#ifdef providesInit
void init();
#else
void init() {}
#endif
void main() {
	init();
	lightdir1 = normalize(vec3(1,0,0)*rotmat(vec3(0,0,1),LightDir1.x)*rotmat(vec3(0,1,0),LightDir1.y));
	lightdir2 = normalize(vec3(1,0,0)*rotmat(vec3(0,0,1),LightDir2.x)*rotmat(vec3(0,1,0),LightDir2.y));
	vec4 t = trace(from,dir);
	vec4 t2=vec4(DEtrace(),MaxDistance*Focus);
	gl_FragColor = length(t2.xyz)>0&&ShowLightPos?t2:t;
//         depthFlag=true; // do depth on the first hit not on reflections
}

bool formula(vec3 p) {
	bool i=false;
	#ifdef providesInside
	i=inside(p);
	#else
	i=DE(p)<pow(DEAdjust,2);
	#endif
	return i!=side;
}


