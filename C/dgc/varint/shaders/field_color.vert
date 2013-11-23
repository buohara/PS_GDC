#version 400

uniform mat4 m_mod;
uniform mat4 m_view;
uniform mat4 m_proj;

layout (location=0) in vec3 in_position;
layout (location=1) in float field;
out vec4 ex_color;

void main(void){
    gl_Position=m_proj*m_view*m_mod*vec4(in_position,1.0);
    gl_PointSize=20.f;
	ex_color=vec4(field,0.3,1.0,1.0);
}
