#version 400
layout(location=0) in vec4 in_Position;
//layout(location=1) in vec4 in_Color;
out vec4 ex_Color;
uniform mat4 m_mod;
uniform mat4 m_view;
uniform mat4 m_proj;
void main(void){
   gl_Position=m_proj*m_view*m_mod*in_Position;
   ex_Color=vec4(1.0,1.0,1.0,1.0);
}
