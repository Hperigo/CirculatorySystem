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

	vec2 texCood = TexCoord.st 	+ vec2(uTime, 0.0);

	vec4 tMap           =  texture( uTexMap, texCood );
	vec4 tPlanes 		= texture( uTexPlanes, texCood );
	vec4 tTerminator    = texture( uTexTerminator, TexCoord.st);

	vec4 mapDayColor = tMap * uMapDayColor.r;
	vec4 mapNightColor = tMap * uMapNightColor.r;
	vec4 map =  mix(mapDayColor, mapNightColor, tTerminator.r); //tMap * uMapDayColor.r;

	vec4 compose =  mix(map, tPlanes, tPlanes.a);
	oColor =  compose; // vec4( compose.rgb , 1.0);


	// vec4 mapNight = tMap * uMapNightColor;
	// vec4 map  = mix(mapDay , mapNight, tTerminator.r );
	//
	// vec4 planesDay = tPlanes * uPlanesDayColor;
	// vec4 planesNight = (1.0 - tPlanes) * uPlanesNightColor;
	//
	// vec4 planes = mix(planesNight, planesDay, tTerminator.r)  * planesNight.a;
	//
	//
	// oColor =   vec4(compose.rgb, 1.0);
}
