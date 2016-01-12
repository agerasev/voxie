#version 130

uniform sampler3D u_texture;
uniform sampler3D u_light;
uniform ivec3 u_size;
uniform ivec3 u_offset;
uniform ivec3 u_true_size;

uniform mat4 u_tex;
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

uniform mat4 u_inv_tex;
uniform mat4 u_inv_model;
uniform mat4 u_inv_view;

uniform ivec2 u_lod;

in vec4 v_tex_pos;
in vec4 v_tex_norm;
in vec4 v_tex_dir;
in float v_z_value;

out vec4 out_FragColor;

bool is_outside(vec3 p, vec3 d, vec3 size) {
	vec3 eps = 1e-3*size;
	bvec3 lb = greaterThan(-eps, p);
	bvec3 hb = greaterThan(p, size + eps);
	return lb.x || lb.y || lb.z || hb.x || hb.y || hb.z;
}

float get_depth(float z) {
	return 0.5 - 0.5*(u_proj[2][2] + u_proj[3][2]/z);
}

bool is_solid(ivec3 p) {
	return texelFetch(u_texture, p + u_offset, u_lod[0]).a > 0.1;
}

float get_occlusion(vec3 p, ivec3 n) {
	ivec3 bx = ivec3(n.x == 0, n.x != 0, 0);
	ivec3 by = ivec3(0, n.z != 0, (n.x != 0) || (n.y != 0));
	ivec3 bp = ivec3(floor(p)) + n;
	vec3 c = fract(p) - vec3(0.5);
	float x = dot(c, vec3(bx));
	float y = dot(c, vec3(by));
	float s = 0.0;
	if(!is_solid(bp)) {
		s += 1.0;
		s += (0.5 + x)*float(!is_solid(bp + bx));
		s += (0.5 + y)*float(!is_solid(bp + by));
		s += (0.5 - x)*float(!is_solid(bp - bx));
		s += (0.5 - y)*float(!is_solid(bp - by));
		s += (0.5 + x)*(0.5 + y)*float(!is_solid(bp + bx + by));
		s += (0.5 - x)*(0.5 + y)*float(!is_solid(bp - bx + by));
		s += (0.5 - x)*(0.5 - y)*float(!is_solid(bp - bx - by));
		s += (0.5 + x)*(0.5 - y)*float(!is_solid(bp + bx - by));
	}
	return s/4.0;
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
	
	depth = gl_FragCoord.z;
	//depth = get_depth(v_z_value);
	
	ivec3 dip = ivec3(0);
	vec3 dp = -n;
	float t = 0.0;
	bool found = false;
	
	int i;
	for(i = 0; i < u_size[0] + u_size[1] + u_size[2] + 3; ++i) {
		// break if point is outside
		if(is_outside(sp,d,size)) {
			found = false;
			break;
		}
		
		// break if opaque enough
		if(is_solid(ivec3(floor(cp)))) {
			found = true;
			break;
		}
		
		vec3 ts;
		// choose next intersection plane
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
		
		// compute intersection parameters
		dp = vec3(dip);
		sp = p + d*t;
		cp = sp + 0.5*dp;
		
		// increment intersection iterator
		ip += dip;
	}
	
	if(found) {
		color = vec4(textureLod(u_texture, (cp + vec3(u_offset))/vec3(u_true_size), u_lod[0]).rgb, 1.0);
		shadow = get_occlusion(cp, -dip);
		
		// depth and normal
		depth = get_depth(v_z_value*(1.0 + t));
		n = -dp;
		
		// light
		vec3 w_pos = (u_model*u_inv_tex*vec4(sp/size, 1.0)).xyz;
		vec3 w_norm = normalize((u_model*u_inv_tex*vec4(n, 0.0)).xyz);
		vec4 light_pos = gl_LightSource[0].position;
		vec3 light_dir = normalize(light_pos.xyz - w_pos*light_pos.w);
		vec3 dif = max(dot(w_norm, light_dir), 0.0)*gl_LightSource[0].diffuse.rgb;
		vec3 amb = gl_LightSource[0].ambient.rgb;
		color.rgb *= dif;
		// specular
		vec3 v_pos = (u_inv_view*vec4(0,0,0,1)).xyz;
		vec3 v_dir = normalize(w_pos - v_pos);
		vec3 r_dir = v_dir - 2.0*dot(v_dir, w_norm)*w_norm;
		color.rgb += vec3(pow(max(dot(r_dir, light_dir), 0.0), 64.0));
	} else {
		depth = 1.0;
	}
	
	gl_FragDepth = depth;
	out_FragColor = vec4(color.rgb*shadow, color.a);
}
