uniform sampler3D u_texture;

varying vec3 v_tex_coord;

void main() {
	gl_FragColor = texture3D(u_texture, v_tex_coord);
}
