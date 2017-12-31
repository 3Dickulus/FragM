#donotrun
#info 3DKn-0.9.12.frag: Original shader by syntopia. Modifications by knighty:
#info  - Assumes square shaped pixel -> no over-blurring along x axis if the rendering window width is small wrt its height
#info  - Added polygon shaped diaphragm.
#info  - Added Control for the width of the in-focus region.

/*
Simple 3D setup with anti-alias and DOF.
Implement this function in fragments that include this file:
vec3 color(vec3 cameraPos, vec3 direction);
*/

#buffer RGBA32F
#buffershader "BufferShader-1.0.1.frag"
#camera 3D

#vertex

#group Camera
// Field-of-view
uniform float FOV; slider[0,0.4,2.0] NotLockable
uniform vec3 Eye; slider[(-50,-50,-50),(0,0,-10),(50,50,50)] NotLockable
uniform vec3 Target; slider[(-50,-50,-50),(0,0,0),(50,50,50)] NotLockable
uniform vec3 Up; slider[(0,0,0),(0,1,0),(0,0,0)] NotLockable

varying vec3 from;
uniform vec2 pixelSize;
varying vec2 coord;
varying vec2 viewCoord;
varying vec2 viewCoord2;
varying vec3 dir;
varying vec3 Dir;
varying vec3 UpOrtho;
varying vec3 Right;
uniform int subframe;
varying vec2 PixelScale;

#ifdef providesInit
void init(); // forward declare
#else
void init() {}
#endif

void main(void)
{
	gl_Position =  gl_Vertex;
	coord = (gl_ProjectionMatrix*gl_Vertex).xy;
	coord.x*= pixelSize.y/pixelSize.x;

	// we will only use gl_ProjectionMatrix to scale and translate, so the following should be OK.
	PixelScale =vec2(pixelSize.x*gl_ProjectionMatrix[0][0], pixelSize.y*gl_ProjectionMatrix[1][1]);
	viewCoord = gl_Vertex.xy;
	viewCoord2= (gl_ProjectionMatrix*gl_Vertex).xy;

	from = Eye;
	Dir = normalize(Target-Eye);
	UpOrtho = normalize( Up-dot(Dir,Up)*Dir );
	Right = normalize( cross(Dir,UpOrtho));
	coord*=FOV;
	init();
}
#endvertex

#group Camera
uniform bool EquiRectangular; checkbox[false]

#group Raytracer

#define PI  3.14159265358979323846264

// Camera position and target.
varying vec3 from;
varying vec3 dir;
varying vec3 dirDx;
varying vec3 dirDy;
varying vec2 coord;
varying float zoom;

//#if 0
//uniform int backbufferCounter;//  subframe; //
//#define subframe backbufferCounter
//#else
uniform int subframe;
//#endif
uniform sampler2D backbuffer;
varying vec2 viewCoord;
varying vec2 viewCoord2;
varying vec3 Dir;
varying vec3 UpOrtho;
varying vec3 Right;


//Distance from camera to in focus zone
uniform float FocalPlane; slider[0.01,1,50]
uniform float Aperture; slider[0,0.00,0.5]
//Width of the in focus zone. this value is relative to FocalPlane
uniform float InFocusAWidth; slider[0,0,1]
uniform bool DofCorrect; checkbox[true]
//Number of sides for the diaphragm. 2->circular
uniform int ApertureNbrSides; slider[2,7,10]
//Rotation of the diaphragm
uniform float ApertureRot; slider[0,0,360]
//For star shaped diphragms. Very limited
uniform bool ApStarShaped; checkbox[false]

// vec2 rand2(vec2 co){
// #ifdef WANG_HASH
//         return vec2(fract(sin(dot(wang_hash_fp(co) ,wang_hash_fp(vec2(12.9898,78.233)))) * 3758.5453),
//                 fract(cos(dot(wang_hash_fp(co) ,wang_hash_fp(vec2(4.898,7.23)))) * 23421.631));
// #else
//         // implementation found at: lumina.sourceforge.net/Tutorials/Noise.html
//         return vec2(fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453),
//                 fract(cos(dot(co.xy ,vec2(4.898,7.23))) * 23421.631));
// #endif
// }

vec2 NGonDisc(vec2 co) {
	vec2 r = rand2(co);
	float d=1./tan(PI/float(ApertureNbrSides));
	float lh=1./sqrt(1.+d*d);
	vec2 p=vec2(r.x+r.y, r.x-r.y);
	if(!ApStarShaped) p.x=1.-abs(p.x-1.);
	p.x*=d;
	p*=lh;
	float a=dot(rand2(11.*co+vec2(1.,1.)),vec2(1.,1.));
	a=floor(a*float(ApertureNbrSides))*2.*PI/float(ApertureNbrSides) + ApertureRot*PI/360.;
	vec2 cs= vec2(cos(a), sin(a));
	p= vec2(cs.x*p.x-cs.y*p.y, cs.x*p.y+cs.y*p.x);
	/*a=ApertureRot*PI/360.;
	cs= vec2(cos(a), sin(a));
	p= vec2(cs.x*p.x-cs.y*p.y, cs.x*p.y+cs.y*p.x);*/
	return p;
}

vec2 uniformDisc(vec2 co) {
	vec2 r = rand2(co);
	return sqrt(r.y)*vec2(cos(r.x*6.28),sin(r.x*6.28));
}

#ifdef providesInit
void init(); // forward declare
#else
void init() {}
#endif
//out vec4 gl_FragColor;

#group Post
uniform float Gamma; slider[0.0,1.0,5.0]
// 1: Linear, 2: Expontial, 3: Filmic, 4: Reinhart; 5: Syntopia
uniform int ToneMapping; slider[1,1,5]
uniform float Exposure; slider[0.0,1.0,3.0]
uniform float Brightness; slider[0.0,1.0,5.0];
uniform float Contrast; slider[0.0,1.0,5.0];
uniform float Saturation; slider[0.0,1.0,5.0];
uniform float GaussianWeight; slider[0.0,1.0,10.0];
uniform float AntiAliasScale; slider[0,2.5,10]
//Apply Bloom. Note: specularity parameters (Light tab) have a huge impact.
uniform bool Bloom; checkbox[false]
uniform float BloomIntensity; slider[0,0,2]
uniform float BloomPow; slider[0,2,10]
uniform int   BloomTaps; slider[1,4,100]
uniform float BloomStrong; slider[1,1,20]

varying vec2 PixelScale;
uniform float FOV;

// Given a camera pointing in 'dir' with an orthogonal 'up' and 'right' vector
// and a point, coord, in screen coordinates from (-1,-1) to (1,1),
// a ray tracer direction is returned
vec3 equiRectangularDirection(vec2 coord, vec3 dir, vec3 up, vec3 right)  {
	vec2 r = vec2(coord.x,(1.0-coord.y)*0.5)*PI;
	return cos(r.x)*sin(r.y)*dir+
		 sin(r.x)*sin(r.y)*right+
		cos(r.y)*up;
}

struct SRay{
	vec3 Origin; // origin of the ray. Intially camera center
	vec3 Direction; // main ray direction
	vec3 Offset; // For DoF
	float Pos; // current position along the ray
	float fudge;// Lazy way to avoid overshooting
	float iFP;// =1/FocalPlane
};

vec3 SRCurrentPt(SRay Ray){
if(DofCorrect){
	float t = Ray.Pos;
	vec3 p= Ray.Origin+Ray.Direction*t ;
	//t = 1.-t*Ray.iFP;
	t = 1.-(t+Ray.fudge)*Ray.iFP;
	float d=t;
	d = d - InFocusAWidth*(2.*smoothstep(-1.5*InFocusAWidth, 1.5*InFocusAWidth,t)-1.);
	vec3 o= Ray.Offset*d;
	return p+ o;
	}
else {
	float t = Ray.Pos;
	return Ray.Origin+Ray.Direction*t + Ray.Offset*(1.-t*Ray.iFP + clamp(t*Ray.iFP-1., -InFocusAWidth, InFocusAWidth));
	}
}


void SRAdvance(inout SRay Ray, float dist){ Ray.Pos+=dist*Ray.fudge;}

SRay SRReflect(SRay Ray, vec3 normal, float eps){
	vec3 hit=SRCurrentPt(Ray);
	Ray.Direction = reflect(Ray.Direction, normal);
	Ray.Offset = reflect(Ray.Offset, normal);
	Ray.Origin= hit + reflect(Ray.Origin - hit, normal);//hit-Ray.Direction*Ray.Pos;
	//Ray.Origin= hit-Ray.Direction*Ray.Pos;
	Ray.Pos+=eps;
	return Ray;
}

// implement this;
vec3 color(SRay Ray);

void main() {
	init();
	vec3 hitNormal = vec3(0.0);
	vec3 hit;

	// A Thin Lens camera model with Depth-Of-Field

	vec2 r = vec2(0.);
	if(ApertureNbrSides == 2)
		// We want to sample a circular diaphragm
		r = Aperture*uniformDisc(viewCoord*(float(subframe)+1.0));
	else
		// We want to sample a polygonal diaphragm
		r = Aperture*NGonDisc(viewCoord*(float(subframe)+1.0));

	//r*=1.-InFocusAWidth;//an error while modifying this script. It gives interresting result though.

	//jitter for multisampling
	vec2 disc = uniformDisc(coord*float(1+subframe)); // subsample jitter
	vec2 jitteredCoord;// = coord + AntiAliasScale*PixelScale.y*FOV*disc;

	// Offset for DoF
	vec3 lensOffset =  r.x*Right + r.y*UpOrtho;
        if (EquiRectangular) {
          jitteredCoord = AntiAliasScale*PixelScale*disc;
        }
        else {
          jitteredCoord = coord + AntiAliasScale*PixelScale*FOV*disc;
        }
	//"principal" ray direction
	vec3 rayDir = (Dir+ jitteredCoord.x * Right + jitteredCoord.y * UpOrtho);
        if (EquiRectangular) {
          rayDir = equiRectangularDirection(viewCoord2, rayDir, UpOrtho, Right);
        }
	float rayDirLength=length(rayDir);
	float FPO1= length( rayDir+lensOffset*( 1. - 1./FocalPlane + clamp( 1./FocalPlane-1., -InFocusAWidth, InFocusAWidth)));

	//Construct the ray
	SRay Ray = SRay(from, rayDir/rayDirLength, lensOffset, 0., 1./max(1.,FPO1), 1./FocalPlane);

	//grab incoming light
	vec3 c =  color(Ray);

	// Accumulate
	vec4 prev = texture2D(backbuffer,(viewCoord+vec2(1.0))/2.0);

	float w =1.0-length(disc);
	if (GaussianWeight>0.) {
		w = exp(-dot(disc,disc)/GaussianWeight)-exp(-1./GaussianWeight);
	}
	gl_FragColor =(prev+ vec4(c*w, w));
}
