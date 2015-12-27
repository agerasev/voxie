uniform sampler3D u_texture;
uniform ivec3 u_tex_size;

varying vec3 v_tex_coord;
varying vec3 v_tex_dir;

void main() {
	vec4 color = vec4(0.0, 0.0, 0.0, 0.0);
	vec3 pos = v_tex_coord;
	vec3 dir = v_tex_dir;
	
	vec3 size = vec3(u_tex_size);
	vec3 p = pos*size;
	vec3 d = dir*size;
	ivec3 id = ivec3(sign(d));
	ivec3 ip = ivec3(ceil(p))*ivec3(greaterThan(id,ivec3(0,0,0))) + ivec3(floor(p))*ivec3(greaterThan(ivec3(0,0,0),id));
	int i;
	/*
	color = texture3D(u_texture, pos);
	if(color.a > 0.5) {
		gl_FragColor = vec4(color);
		return;
	}
	*/
	for(i = 0; i < 0x100; ++i) {
		ivec3 dip;
		vec3 ts;
		float t;
		ts = (vec3(ip) - p)/d;
		if(ts.x < ts.y) {
			if(ts.x < ts.z) {
				dip = ivec3(id.x,0,0);
				t = ts.x;
			} else {
				dip = ivec3(0,0,id.z);
				t = ts.z;
			}
		} else {
			if(ts.y < ts.z) {
				dip = ivec3(0,id.y,0);
				t = ts.y;
			} else {
				dip = ivec3(0,0,id.z);
				t = ts.z;
			}
		}
		vec3 sp = p + d*t + 0.5*vec3(dip);
		bvec3 lb = greaterThan(vec3(0.0,0.0,0.0),sp);
		bvec3 hb = greaterThan(sp,size);
		if(lb.x || lb.y || lb.z || hb.x || hb.y || hb.z)
			break;
		ip += dip;
		color = texture3D(u_texture, sp/size);
		if(color.a > 0.5)
			break;
	}
	
	gl_FragColor = vec4(color);
}


/*
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
*/
