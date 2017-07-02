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

uniform float Gamma;
uniform float Exposure;
uniform float Brightness;
uniform float Contrast;
uniform float Saturation;
uniform int ToneMapping;
uniform bool Bloom;
uniform float BloomIntensity; 
uniform float BloomPow;
uniform int   BloomTaps; 
uniform float BloomStrong;
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
	const vec3 LumCoeff = vec3(0.2125, 0.7154, 0.0721);
	vec3 AvgLumin = vec3(0.5);
	vec3 brtColor = color * brt;
	float intensityf = dot(brtColor, LumCoeff);
	vec3 intensity = vec3(intensityf, intensityf, intensityf);
	vec3 satColor = mix(intensity, brtColor, sat);
	vec3 conColor = mix(AvgLumin, satColor, con);
	return conColor;
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
vec4 bloom(vec2 pos, vec2 quality){//see: https://gist.github.com/BlackBulletIV/4218802
	int samples=2*BloomTaps+1;
	vec4 sum = vec4(0);
	int diff = (samples - 1) / 2;
	vec2 sizeFactor = quality;
	for (int x = -diff; x <= diff; x++)
	{
		float wx=float(x)/float(diff); wx=1.-wx*wx; wx=wx*wx;
		for (int y = -diff; y <= diff; y++)
		{
			float wy=float(y)/float(diff); wy=1.-wy*wy; wy=wy*wy;
			vec2 offset = vec2(x, y) * sizeFactor;
			sum += texture2D(frontbuffer, (pos+offset))*wx*wy;
		}
	}
	return (sum / float(samples * samples)); 
}
void main() {
	vec2 pos = (coord+vec2(1.0))/2.0;
	vec2 pixelsiz=vec2(dFdx(pos.x),dFdy(pos.y))*BloomStrong;
	vec4 tex = texture2D(frontbuffer, pos);
	vec3 c = tex.xyz/tex.a;
	if(Bloom){
		vec4 b=bloom(pos,pixelsiz);
		b=b/b.w;
		c+=BloomIntensity*pow(b.xyz,vec3(BloomPow));
	}
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
