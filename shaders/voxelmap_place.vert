#version 130

in vec4 a_vertex;
in vec4 a_normal;

uniform mat4 u_tex;
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

uniform mat4 u_inv_tex;
uniform mat4 u_inv_model;
uniform mat4 u_inv_view;

out vec4 v_tex_pos;
out vec4 v_tex_norm;
out vec4 v_tex_dir;
out float v_z_value;

void main() {
	vec4 view_pos = u_view*(u_model*a_vertex);
	v_tex_pos  = u_tex*a_vertex;
	v_tex_norm = u_tex*a_normal;
	vec4 view_dir = vec4(view_pos.xyz, 0.0);
	v_tex_dir  = u_tex*(u_inv_model*(u_inv_view*view_dir));
	v_z_value = view_pos.z;
	gl_Position = u_proj*(view_pos);
}
