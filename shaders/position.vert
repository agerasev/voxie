attribute vec4 a_vertex;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

varying vec3 v_tex_coord;

void main() {
	v_tex_coord = a_vertex.xyz + vec3(0.5, 0.5, 0.5);
	vec4 pos = u_proj*(u_view*(u_model*a_vertex));
	gl_Position = vec4(pos.xy, pos.z, pos.w);
}
