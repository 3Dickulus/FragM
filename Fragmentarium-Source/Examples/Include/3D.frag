#donotrun

/*
Simple 3D setup with anti-alias and DOF.

Implement this function in fragments that include this file:
vec3 color(vec3 cameraPos, vec3 direction);

*/

#buffer RGBA32F

#buffershader "BufferShader.frag"

#info Simple 3D Setup
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
// Sets focal plane to Target location
uniform bool AutoFocus; checkbox[false]

uniform float FocalPlane; slider[0,1,50]
uniform float Aperture; slider[0,0.00,0.2]


#group Raytracer

#define PI  3.14159265358979323846264

// Camera position and target.
varying vec3 from;
varying vec3 dir;
varying vec3 dirDx;
varying vec3 dirDy;
varying vec2 coord;
varying float zoom;

uniform int subframe;
uniform sampler2D backbuffer;
varying vec2 viewCoord;
varying vec2 viewCoord2;
varying vec3 Dir;
varying vec3 UpOrtho;
varying vec3 Right;

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
uniform float Gamma; slider[0.0,2.0,5.0];
// 1: Linear, 2: Expontial, 3: Filmic, 4: Reinhart; 5: Syntopia
uniform int ToneMapping; slider[1,4,5];
uniform float Exposure; slider[0.0,1.0,3.0];
uniform float Brightness; slider[0.0,1.0,5.0];
uniform float Contrast; slider[0.0,1.0,5.0];
// Affects Contrast
uniform vec3 AvgLumin; color[0.5,0.5,0.5];
uniform float Saturation; slider[0.0,1.0,5.0];
// Affects Saturation
uniform vec3 LumCoeff; color[0.2125,0.7154,0.0721];
uniform float Hue; slider[0,0,1.0];
uniform float GaussianWeight; slider[0.0,1.0,10.0];
uniform float AntiAliasScale; slider[0,2,10];

varying vec2 PixelScale;
uniform float FOV;

// implement this;
vec3 color(vec3 cameraPos, vec3 direction);

// Given a camera pointing in 'dir' with an orthogonal 'up' and 'right' vector
// and a point, coord, in screen coordinates from (-1,-1) to (1,1),
// a ray tracer direction is returned
vec3 equiRectangularDirection(vec2 coord, vec3 dir, vec3 up, vec3 right)  {
	vec2 r = vec2(coord.x,(1.0-coord.y)*0.5)*PI;
	return cos(r.x)*sin(r.y)*dir+
		 sin(r.x)*sin(r.y)*right+
		cos(r.y)*up;
}

void main() {
	init();

	// A Thin Lens camera model with Depth-Of-Field
	// We want to sample a circular diaphragm
	// Notice: this is not sampled with uniform density
	vec2 r = Aperture*uniformDisc(viewCoord*(float(subframe+1)));
	vec2 disc = uniformDisc(coord*float(subframe+1)); // subsample jitter
	vec2 jitteredCoord;
	// Direction from Lens positon to point on FocalPlane
	vec3 lensOffset = r.x*Right + r.y*UpOrtho;
        vec3 rayDir;

	if (EquiRectangular) {
	  jitteredCoord = AntiAliasScale*PixelScale*disc;
	}
	else {
	  jitteredCoord = coord + AntiAliasScale*PixelScale*FOV*disc;
	}

	//vec3 rayDir = normalize (Dir+ jitteredCoord.x*Right+jitteredCoord.y*UpOrtho)*FocalPlane-(lensOffset);
	rayDir = (Dir+ jitteredCoord.x*Right+jitteredCoord.y*UpOrtho)*FocalPlane-(lensOffset);
	rayDir = normalize(rayDir);

	if (EquiRectangular) {
	  rayDir = equiRectangularDirection(viewCoord2, rayDir, UpOrtho, Right);
	}

	vec3 c =  color(from+lensOffset,rayDir);

	// Accumulate
	vec4 prev = texture2D(backbuffer,(viewCoord+vec2(1.0))*0.5);
	float w =1.0-length(disc);
	if (GaussianWeight>0.) {
		w = exp(-dot(disc,disc)/GaussianWeight)-exp(-1.0/GaussianWeight);
	}

   gl_FragColor = (prev+ vec4(c*w, w));
}
