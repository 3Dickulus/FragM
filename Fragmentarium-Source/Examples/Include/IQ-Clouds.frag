#donotrun
// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
// adapted for Fragmentarium by user 3Dickulus FractalForums.com
#group Clouds
uniform int CloudSteps; slider[1,64,128]
uniform float CloudHeight; slider[-5,-1,5]
uniform vec3 WindDir; slider[(-1,-1,-1),(0,0,1),(1,1,1)]
uniform vec3 DiffuseLinear1; color[0.65,0.68,0.7]
uniform vec3 DiffuseLinear2; color[0.7,0.5,0.3]
uniform vec3 SunRefract1; color[1.0,0.5,1.0]
uniform vec3 SunRefract2; color[1.0,.6,0.1]
uniform vec3 SunRefract3; color[1.0,0.4,0.2]
uniform float Attenuation; slider[-5,-1,5]

// hash based 3d value noise
float hash( float n )
{
    return fract(sin(n)*43758.5453);
}
float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);

    f = f*f*(3.0-2.0*f);
    float n = p.x + p.y*57.0 + 113.0*p.z;
    return mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                   mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y),
               mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                   mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
}

vec4 map( in vec3 p )
{
	float d = CloudHeight - p.y;

	vec3 q = p - WindDir*time;
	float f;
    f  = 0.5000*noise( q ); q = q*2.02;
    f += 0.2500*noise( q ); q = q*2.03;
    f += 0.1250*noise( q ); q = q*2.01;
    f += 0.0625*noise( q );

	d += 3.0*f;

	d = clamp( d, 0.0, 1.0 );

	vec4 res = vec4( d );

	res.xyz = mix( 1.15*vec3(1.0,0.95,0.8), vec3(0.7,0.7,0.7), res.x );

	return res;
}


vec3 sundir = vec3(-1.0,0.0,0.0);


vec4 raymarch( in vec3 ro, in vec3 rd )
{
	vec4 sum = vec4(0, 0, 0, 0);

	float t = 0.0;
	for(int i=0; i<CloudSteps; i++)
	{
		if( sum.w > 0.99 ) continue;

		vec3 pos = ro + t*rd;
		vec4 col = map( pos );

		float dif =  clamp((col.w - map(pos+0.3*sundir).w), 0.0, 1.0 );

		col.xyz *= DiffuseLinear1*1.35 + 0.45*DiffuseLinear2*dif;

		col.a *= 0.35;
		col.rgb *= col.a;

		sum = sum + col*(1.0 - sum.w);

		t += 0.1;
	}

	sum.xyz /= (0.001+sum.w);

	return clamp( sum, 0.0, 1.0 );
}

vec4 clouds(vec3 from, vec3 dir, vec3 blend)
{
    vec3 ro = from;
    vec3 rd = dir;

    sundir = vec3(sin(SpotLightDir.x*3.1415)*cos(SpotLightDir.y*3.1415*0.5), sin(SpotLightDir.y*3.1415*0.5)*sin(SpotLightDir.x*3.1415), cos(SpotLightDir.x*3.1415));

    vec4 res = raymarch( ro, rd );

	float sun = clamp( dot(sundir,rd), 0.0, 1.0 );
	vec3 col = blend - rd.y*0.2*SunRefract1 + 0.15*0.5;
	col += 0.2*SunRefract2*pow( sun, 8.0 );
	col *= 0.95;
	col = mix( col, res.xyz, res.w );
	col += 0.1*SunRefract3*pow( sun, 3.0 );

    return vec4( col, res.w );

}

