#version 120
#extension GL_EXT_gpu_shader4 : enable
#extension GL_ARB_texture_rectangle : enable

////varying float color;
//varying vec4 pos;
uniform sampler2DRect rands;
uniform sampler2DRect tweenTo;

// to-do: depth should be displacement texture too!

uniform float width;
uniform float height;
uniform float depth;

void main() {
    gl_PointSize = 10.0;
	vec4 pos = gl_Vertex;
    vec2 coord = vec2(pos.xy);
    float color = texture2DRect(rands, pos.xy).x;
    vec4 disp = texture2DRect(tweenTo, pos.xy);
    disp.x *= width;
    disp.y *= height;
    disp.z *= depth;
    disp.w = 1.0;
    
    // look up color based on mapped coordinate
    vec4 posMutated = pos;
    posMutated = mix(pos, disp, color);
    
    gl_Position = gl_ModelViewProjectionMatrix * posMutated;
    gl_FrontColor = gl_Color;
}