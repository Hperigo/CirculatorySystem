#version 150

out vec4 oColor;

uniform sampler2D uTexMap;
uniform sampler2D uTexPlanes;
uniform sampler2D uTexTerminator;

uniform float uTime;

// Map
uniform vec4 uMapDayColor;
uniform vec4 uMapNightColor;

uniform vec4 uPlanesDayColor;
uniform vec4 uPlanesNightColor;


in vec2	TexCoord;
void main( void )
{

	vec4 tMap           = texture( uTexMap, TexCoord.st 	+ vec2(uTime, 0.0) );
	vec4 tPlanes 		= texture( uTexPlanes, TexCoord.st  + vec2(uTime, 0.0));
	vec4 tTerminator    = texture( uTexTerminator, TexCoord.st);


	vec4 mapDay = tMap * uMapDayColor;
	vec4 mapNight = tMap * uMapNightColor;
	vec4 map  = mix(mapDay , mapNight, tTerminator.r );

	vec4 planesDay = tPlanes * uPlanesDayColor;
	vec4 planesNight = tPlanes * uPlanesNightColor;

	vec4 planes = tPlanes; // mix(planesDay, planesNight, tTerminator.r);

	vec4 compose =  mix(map, planes, tPlanes.a);

	vec3 inverted = compose.rgb;
	oColor =  vec4(inverted, 1.0);
}
