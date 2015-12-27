attribute vec4 a_vertex;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

uniform mat3 u_tex_basis;

varying vec3 v_tex_coord;
varying vec3 v_tex_dir;

void main() {
	vec4 view_pos = u_view*(u_model*a_vertex);
	v_tex_coord = a_vertex.xyz + vec3(0.5, 0.5, 0.5);
	v_tex_dir = u_tex_basis*view_pos.xyz;
	gl_Position = u_proj*(view_pos);
}
