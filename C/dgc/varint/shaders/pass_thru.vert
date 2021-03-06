#version 400

uniform mat4 m_mod;
uniform mat4 m_view;
uniform mat4 m_proj;

layout(location=0) in vec3 in_position;
layout(location=1) in float in_field;
out vec4 ex_color;

/*
void main(void){
    gl_Position=m_proj*m_view*m_mod*vec4(in_position,1.0);
	ex_color=vec4(0.5+in_field,0.5,0.5,0.5);
}
*/

vec3 get_jet_color(float value){
     float four_val=4*value;
     float red=min(four_val-1.5,-four_val+4.5);
     float green=0.6*min(four_val-0.5,-four_val+3.5);
     float blue=min(four_val+0.5,-four_val+2.5);
     return clamp(vec3(red,green,blue),0.0,1.0);
}

void main(void){
	float field=(in_field+1.0)/2.0;
    vec3 out_rgb=get_jet_color(field);
    gl_Position=m_proj*m_view*m_mod*vec4(in_position,1.0);
	ex_color=vec4(out_rgb,1.0);
}
