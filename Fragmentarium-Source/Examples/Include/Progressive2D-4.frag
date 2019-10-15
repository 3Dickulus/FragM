#donotrun
#buffer RGBA32F
#buffershader "BufferShader-4.frag"

// This is a utlity program for setting
// up anti-aliased 2D rendering.

#vertex

out vec2 coord;
out vec2 aaScale;
out vec2 viewCoord;

#group Camera

// Use this to adjust clipping planes

uniform double Zoom; slider[0.5,1,1e16] NotLockable
uniform dvec2 Center; slider[(-100,-100),(0,0),(100,100)] NotLockable

uniform bool EnableTransform; checkbox[true]
uniform float RotateAngle; slider[-360,0,360]
uniform float StretchAngle; slider[-360,0,360]
uniform float StretchAmount; slider[-100,0,100]

uniform vec2 pixelSize;


void main(void)
{
	mat2 transform = mat2(1.0, 0.0, 0.0, 1.0);
	if (EnableTransform)
	{
		float b = radians(RotateAngle);
		float bc = cos(b);
		float bs = sin(b);
		float a = radians(StretchAngle);
		float ac = cos(a);
		float as = sin(a);
		float s = sqrt(pow(2.0, StretchAmount));
		mat2 m1 = mat2(ac, as, -as, ac);
		mat2 m2 = mat2(s, 0.0, 0.0, 1.0 / s);
		mat2 m3 = mat2(ac, -as, as, ac);
		mat2 m4 = mat2(bc, bs, -bs, bc);
		transform = m1 * m2 * m3 * m4;
	}
	float ar = pixelSize.y/pixelSize.x;
	gl_Position =  gl_Vertex;
	viewCoord = gl_Vertex.xy;
	coord = vec2(transform * ((gl_ProjectionMatrix*gl_Vertex).xy*vec2(ar,1.0))/float(Zoom));
	aaScale = vec2(gl_ProjectionMatrix[0][0],gl_ProjectionMatrix[1][1])*pixelSize/float(Zoom);
}

#endvertex

uniform dvec2 Center;

#group Post

uniform double Gamma; slider[0.0,2.2,5.0]
uniform double Exposure; slider[0.0,1.0,30.0]
uniform double Brightness; slider[0.0,1.0,5.0]
uniform double Contrast; slider[0.0,1.0,5.0]
uniform double Saturation; slider[0.0,1.0,5.0]
// 1: Linear, 2: Exponential, 3: Filmic, 4: Reinhart
uniform int ToneMapping; slider[1,1,4]
uniform double AARange; slider[0.0,2.,15.3]
uniform double AAExp; slider[0.0,1,15.3]
uniform bool GaussianAA; checkbox[true]

in vec2 coord;
in vec2 aaScale;
in vec2 viewCoord;
out vec4 FragColor;

dvec2 aaCoord;
uniform vec2 pixelSize;

#ifdef providesFiltering
	dvec4 color(dvec2 z) ;
#else
	dvec3 color(dvec2 z) ;
#endif

#ifdef providesInit
void init(); // forward declare
#endif

uniform int subframe;

dvec2 rand(dvec2 co){
	// implementation found at: lumina.sourceforge.net/Tutorials/Noise.html
	return
	dvec2(fract(sin( dot(co.xy ,dvec2(12.9898,78.233)) ) * 43758.5453),
		fract(cos( dot(co.xy ,dvec2(4.898,7.23)) ) * 23421.631));
}

uniform sampler2D backbuffer;


dvec2 uniformDisc(dvec2 co) {
	if (co == dvec2(0.0)) return dvec2(0.0);
	dvec2 r = rand(co);
	return sqrt(r.y)*dvec2(cos( r.x*6.28 ),sin( r.x*6.28 ));
}

void main() {
    aaCoord = viewCoord;
#ifdef providesInit
	init();
#endif
    //  dvec2 r = rand(viewCoord*(double(subframe)+1.0))-dvec2(0.5);
#ifdef providesFiltering
	 dvec4 prev = texture(backbuffer,(viewCoord+vec2(1.0))/2.0);
	dvec4 new = color(coord.xy+Center);
#ifdef linearGamma
     FragColor = prev+ new;
#else
     FragColor = prev+dvec4( pow( dvec3(new.xyz),dvec3(Gamma)) , new.w);
#endif
 #else
	dvec2 r = uniformDisc(dvec2( 1.0*(double(subframe+1)) ));
	double w =1.0;
      if (GaussianAA) {
	 	// Gaussian
		w= exp( -dot(r,r)/AAExp )-exp( -1.0/AAExp );

	      // Lancos
	      // w = sin(r.x)*sin(r.x/AARange)*sin(r.y)*sin(r.y/AARange)/(r.x*r.x*r.y*r.y*AARange*AARange);
	      // if (w!=w) w = 1.0;
	} // else {  w =pow( 1.0-length(r),1.0);};

	r*=AARange;
	dvec2 c = coord.xy+Center+aaScale*r;
#ifdef linearGamma
	dvec3 color =  color(c);
#else
      dvec3 color = pow( vec3(color(c)), vec3(Gamma));
#endif

      vec4 prev = texture(backbuffer,vec2(viewCoord+vec2(1.0))/2.0);

	if (! (color==color)) { color =dvec3( 0.0); w = 0.0; } // NAN check
      FragColor = vec4(prev+vec4(color*w, w));

#endif
}

