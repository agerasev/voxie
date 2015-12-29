#version 120

attribute vec4 a_vertex;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

uniform mat4 u_tex_map;
uniform mat3 u_tex_basis;

varying vec3 v_tex_coord;
varying vec3 v_tex_dir;
varying float v_z_value;

void main() {
	vec4 view_pos = u_view*(u_model*a_vertex);
	v_tex_coord = (u_tex_map*a_vertex).xyz;
	v_tex_dir = u_tex_basis*view_pos.xyz;
	v_z_value = view_pos.z;
	gl_Position = u_proj*(view_pos);
}
