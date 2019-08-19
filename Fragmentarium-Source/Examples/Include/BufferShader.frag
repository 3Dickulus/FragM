#donotrun
// This is a simple shader for rendering images
// from an accumulated buffer.

#vertex

varying vec2 coord;

void main(void)
{
	gl_Position =  gl_Vertex;
	coord = (gl_ProjectionMatrix*gl_Vertex).xy;
}

#endvertex

#group Post

uniform float Gamma; // widget in Main frag (eg Progressive2D)

// 1: Linear, 2: Exponential, 3: Filmic, 4: Reinhart
uniform int ToneMapping; slider[1,1,4]
uniform float Exposure; slider[0.0,1.0,30.0]
uniform float Brightness; slider[0.0,1.0,5.0];
uniform float Contrast; slider[0.0,1.0,5.0];
uniform float Saturation; slider[0.0,1.0,5.0];

uniform float Hue; slider[0,0,1]
uniform vec3 LumCoeff; color[1,1,1]
uniform vec3 AvgLumin; color[0,0,0]

// RGB <-> HSV conversion, thanks to http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

// HSV <-> RGB conversion, thanks to http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

/*
** Based on: http://mouaif.wordpress.com/2009/01/22/photoshop-gamma-correction-shader/
**
** Contrast, saturation, brightness
** Code of this function is from TGM's shader pack
** http://irrlicht.sourceforge.net/phpBB2/viewtopic.php?t=21057
*/
// For all settings: 1.0 = 100% 0.5=50% 1.5 = 150%
vec3 ContrastSaturationBrightness(vec3  color, float brt, float sat, float con)
{
	vec3 brtColor = color * brt;
	float intensityf = dot(brtColor, LumCoeff);
	vec3 intensity = vec3(intensityf, intensityf, intensityf);
	vec3 satColor = mix(intensity, brtColor, sat);
	vec3 conColor = mix(AvgLumin, satColor, con);
	// https://fractalforums.org/fragmentarium/17/i-cant-get-the-newest-version-to-work/2629/msg13086#msg13086
	return clamp(conColor,0.0,1.0);
}

float sigmoid(float t) {
	float K = 1.0-1./(1.0+exp(-0.5*Contrast*5.));
	t -= 0.5;
	float  x = 1./(1.0+exp(-t*Contrast*5.))-K;
	return x/((1.0-2.0*K));
}

vec3 sigmoid3(vec3 t) {
	return vec3(sigmoid(t.x),sigmoid(t.y),sigmoid(t.z));
}

varying vec2 coord;
uniform sampler2D frontbuffer;

void main() {
	vec2 pos = (coord+1.0) * 0.5;
	vec4 tex = texture2D(frontbuffer, pos);
// 	vec3 c = tex.xyz/tex.a;
    vec3 colorHSV = rgb2hsv(tex.rgb);  //based on ased on VB_overflows answer on https://stackoverflow.com/questions/32080747/gpuimage-add-hue-color-adjustments-per-rgb-channel-adjust-reds-to-be-more-pink
    colorHSV.x += Hue;
    vec3 c = hsv2rgb(vec3(colorHSV));   
    c=c/tex.a;
    
	if (ToneMapping==1) {
		// Linear
		c = c*Exposure;
		c = ContrastSaturationBrightness(c, Brightness, Saturation, Contrast);

	} else if (ToneMapping==2) {
		// ExponentialExposure
		c = vec3(1.0)-exp(-c*Exposure);
		c = ContrastSaturationBrightness(c, Brightness, Saturation, Contrast);

	} else if (ToneMapping==3) {
		// Filmic: http://filmicgames.com/archives/75
		c*=Exposure;
		vec3 x = max(vec3(0.),c-vec3(0.004));
		c = (x*(6.2*x+.5))/(x*(6.2*x+1.7)+0.06);
		c = pow(c, vec3(2.2)); // It already takes the Gamma into acount
		c = ContrastSaturationBrightness(c, Brightness, Saturation, Contrast);

	} else if (ToneMapping==4) {
		// Reinhart
		c*=Exposure;
		c = c/(1.+c);
		c = ContrastSaturationBrightness(c, Brightness, Saturation, Contrast);
	}  else if (ToneMapping==5) {
		c = sigmoid3(c*Exposure+vec3(Brightness-1.0));

	}
	c = pow(c, vec3(1.0/Gamma));

	gl_FragColor = vec4(c,1.0);
}
