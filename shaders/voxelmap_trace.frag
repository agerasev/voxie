#version 120

uniform sampler3D u_texture;
uniform ivec3 u_tex_size;

uniform mat4 u_proj;

varying vec3 v_tex_coord;
varying vec3 v_tex_dir;
varying float v_z_value;

bool is_outside(vec3 sp, vec3 size) {
	vec3 eps = 1e-3*size;
	bvec3 lb = greaterThan(-eps, sp);
	bvec3 hb = greaterThan(sp, size + eps);
	return lb.x || lb.y || lb.z || hb.x || hb.y || hb.z;
}

void main() {
	vec4 color = vec4(0.0, 0.0, 0.0, 0.0);
	float depth = 1.0;
	vec3 pos = v_tex_coord;
	vec3 dir = v_tex_dir;
	
	vec3 size = vec3(u_tex_size);
	vec3 d = dir*size;
	vec3 p = pos*size;
	ivec3 id = ivec3(sign(d));
	ivec3 ip = ivec3(ceil(p))*ivec3(greaterThan(id,ivec3(0,0,0))) + ivec3(floor(p))*ivec3(greaterThan(ivec3(0,0,0),id));
	
	vec3 sp = p;
	depth = -u_proj[2][2] - u_proj[3][2]/v_z_value;
	
	int i;
	for(i = 0; i < 0x1000; ++i) {
		if(is_outside(sp,size)) {
			depth = 1.0;
			break;
		}
		color = texture3D(u_texture, sp/size);
		if(color.a > 0.9) {
			break;
		}
		
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
		
		sp = p + d*t + 0.5*vec3(dip);
		depth = -u_proj[2][2] - u_proj[3][2]/(v_z_value*(1.0 + t));
		
		ip += dip;
	}
	
	gl_FragDepth = depth;
	gl_FragColor = color;
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
