// blaws
// November 7, 2014
// Tree

uniform mat4 u_mv_Matrix;

attribute vec3 a_vPosition;
attribute vec3 a_vNormal;
attribute vec2 a_vTexCoord;

varying vec3 v_vNormal;
varying vec2 v_vTexCoord;

//==========================================================

void main()
{
    gl_Position = u_mv_Matrix * vec4( a_vPosition, 1.0 );
    v_vNormal = normalize( a_vNormal );
    v_vTexCoord = a_vTexCoord;
}
