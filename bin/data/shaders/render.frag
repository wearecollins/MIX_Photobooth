#version 120
#extension GL_ARB_texture_rectangle : enable
#define KERNEL_SIZE 9

uniform sampler2DRect posData;
uniform vec2  screen;

void main(void){
    vec2 st = gl_TexCoord[0].st;    // gets the position of the pixel that it´s dealing with...
    
    vec2 pos = texture2DRect( posData, st).xy;
    
    gl_FragColor = vec4(vel.x,vel.y,0.0,1.0);   // Then save the vel data into the velocity FBO
}