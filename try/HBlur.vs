#version 330 core


layout(location = 0) in vec3 vertexPosition_modelspace;
 
uniform float blurFactor;

out vec2 UV;
out vec2 v_blurTexCoords[14];
 
void main()
{
    gl_Position = vec4(vertexPosition_modelspace,1);
    UV = (vertexPosition_modelspace.xy+vec2(1,1))/2.0;
    v_blurTexCoords[ 0] = UV + vec2(-0.028*blurFactor, 0.0);
    v_blurTexCoords[ 1] = UV + vec2(-0.024*blurFactor, 0.0);
    v_blurTexCoords[ 2] = UV + vec2(-0.020*blurFactor, 0.0);
    v_blurTexCoords[ 3] = UV + vec2(-0.016*blurFactor, 0.0);
    v_blurTexCoords[ 4] = UV + vec2(-0.012*blurFactor, 0.0);
    v_blurTexCoords[ 5] = UV + vec2(-0.008*blurFactor, 0.0);
    v_blurTexCoords[ 6] = UV + vec2(-0.004*blurFactor, 0.0);
    v_blurTexCoords[ 7] = UV + vec2( 0.004*blurFactor, 0.0);
    v_blurTexCoords[ 8] = UV + vec2( 0.008*blurFactor, 0.0);
    v_blurTexCoords[ 9] = UV + vec2( 0.012*blurFactor, 0.0);
    v_blurTexCoords[10] = UV + vec2( 0.016*blurFactor, 0.0);
    v_blurTexCoords[11] = UV + vec2( 0.020*blurFactor, 0.0);
    v_blurTexCoords[12] = UV + vec2( 0.024*blurFactor, 0.0);
    v_blurTexCoords[13] = UV + vec2( 0.028*blurFactor, 0.0);
}