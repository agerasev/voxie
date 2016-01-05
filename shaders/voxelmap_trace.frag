#version 130

uniform sampler3D u_texture;
uniform sampler3D u_shadow;
uniform ivec3 u_size;

uniform mat4 u_tex;
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

uniform mat4 u_inv_tex;
uniform mat4 u_inv_model;
uniform mat4 u_inv_view;

in vec4 v_tex_pos;
in vec4 v_tex_norm;
in vec4 v_tex_dir;
in float v_z_value;

out vec4 out_FragColor;

bool is_outside(vec3 p, vec3 d, vec3 size) {
	vec3 eps = 1e-2*size;
	bvec3 lb = greaterThan(-eps, p);
	bvec3 hb = greaterThan(p, size + eps);
	return lb.x || lb.y || lb.z || hb.x || hb.y || hb.z;
}

void main() {
	vec4 color = vec4(0.0);
	float shadow = 1.0;
	float depth = 1.0;
	vec4 pos = v_tex_pos;
	vec4 dir = v_tex_dir;
	vec4 norm = v_tex_norm;
	
	vec3 size = vec3(u_size);
	vec3 d = dir.xyz*size;
	vec3 p = pos.xyz*size;
	vec3 n = norm.xyz;
	ivec3 id = ivec3(sign(d));
	ivec3 ip = ivec3(ceil(p))*ivec3(greaterThan(id,ivec3(0,0,0))) + ivec3(floor(p))*ivec3(greaterThan(ivec3(0,0,0),id));
	
	vec3 sp = p, cp = p;
	depth = -u_proj[2][2] - u_proj[3][2]/v_z_value;
	
	int i;
	for(i = 0; i < 0x1000; ++i) {
		// break if point is outside
		if(is_outside(sp,d,size)) {
			depth = 1.0;
			break;
		}
		
		// get color and break if opaque enough
		color.a = texelFetch(u_texture, ivec3(floor(cp)), 0).a;
		if(color.a > 0.1) {
			color.rgb = texture(u_texture, cp/size).rgb;
			shadow = dot(texture(u_shadow, (sp + vec3(0.5))/(size + vec3(1))).rgb, abs(n));
			// light
			float diff = max(0.0, -dot(normalize(u_view*u_model*u_inv_tex*vec4(n, 0.0)), normalize(vec4((u_view*u_model*u_inv_tex*vec4(sp/size, 1.0)).xyz, 0.0))));
			color.rgb *= diff;
			if(diff > 0.0)
				color.rgb += vec3(pow(diff, 64.0));
			break;
		}
		
		// choose next intersection plane
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
		
		// compute next intersection parameters
		sp = p + d*t;
		cp = sp + 0.5*vec3(dip);
		depth = -u_proj[2][2] - u_proj[3][2]/(v_z_value*(1.0 + t));
		n = vec3(-dip);
		
		// increment intersection iterator
		ip += dip;
	}
	
	gl_FragDepth = depth;
	out_FragColor = vec4(color.rgb*shadow, color.a);
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
