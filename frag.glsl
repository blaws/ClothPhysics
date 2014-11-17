// blaws
// November 7, 2014
// Tree

uniform sampler2D u_sTexture;

varying vec3 v_vNormal;
varying vec2 v_vTexCoord;

const vec3 color = vec3( 1.0, 1.0, 1.0 );
const vec3 lightDir = vec3( 1.0, 1.0, 0.0 );

//==========================================================

void main()
{
    vec3 L = normalize( lightDir );
    vec3 N = normalize( v_vNormal );
    float NdotL = dot( N, L );
    
    vec3 baseColor = texture2D( u_sTexture, v_vTexCoord ).xyz;
    
    gl_FragColor = vec4( NdotL * baseColor, 1.0 );
}
