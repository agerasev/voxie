uniform sampler3D u_texture;
uniform ivec3 u_tex_size;

varying vec3 v_tex_coord;
varying vec3 v_tex_dir;

void main() {
	vec4 color = vec4(0.0, 0.0, 0.0, 0.0);
	vec3 pos = v_tex_coord;
	vec3 dir = normalize(v_tex_dir);
	float step = 0.01;
	float factor = step*dot(abs(dir), vec3(u_tex_size));
	while(pos.x >= 0.0 && pos.x <= 1.0 && pos.y >= 0.0 && pos.y <= 1.0 && pos.z >= 0.0 && pos.z <= 1.0) {
		vec4 new_color = texture3D(u_texture, pos);
		new_color.a *= factor;
		color += vec4(new_color.rgb*(1.0 - color.a)*new_color.a, (1.0 - color.a)*new_color.a);
		if(color.a > 1.0 - 1e-4)
			break;
		pos += step*dir;
	}
	gl_FragColor = vec4(color.rgb/color.a, color.a);
}
