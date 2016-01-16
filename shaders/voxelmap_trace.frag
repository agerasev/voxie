#version 130

uniform sampler3D u_texture;
// uniform sampler3D u_light_texture;
uniform ivec3 u_size;

uniform mat4 u_tex;
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

uniform mat4 u_inv_tex;
uniform mat4 u_inv_model;
uniform mat4 u_inv_view;

uniform ivec2 u_lod;

uniform vec4 u_ambient;
uniform vec4 u_light_pos;
uniform vec4 u_light_color;

in vec4 v_tex_pos;
in vec4 v_tex_norm;
in vec4 v_tex_dir;
in float v_z_value;

out vec4 out_FragColor;

ivec3 get_size_lod(ivec3 size, int lod) {
	return (size - ivec3(1))/(1<<lod) + ivec3(1);
}

vec3 uni_to_cell(vec3 tc, int lod) {
	return tc*vec3(get_size_lod(u_size, lod));
}

vec3 cell_to_uni(vec3 gc, int lod) {
	return gc/vec3(get_size_lod(u_size, lod));
}

const float EPSILON = 1e-3;

bool is_outside_float(vec3 p, vec3 b, vec3 e) {
	vec3 eps = vec3(EPSILON);
	bvec3 lb = greaterThan(b - eps, p);
	bvec3 hb = greaterThanEqual(p, e + eps);
	return lb.x || lb.y || lb.z || hb.x || hb.y || hb.z;
}

bool is_outside_int(ivec3 p, ivec3 b, ivec3 e) {
	bvec3 lb = greaterThan(ivec3(0), p);
	bvec3 hb = greaterThanEqual(p, e);
	return lb.x || lb.y || lb.z || hb.x || hb.y || hb.z;
}

bool is_behind_int(ivec3 p, ivec3 d, ivec3 b, ivec3 e) {
	bvec3 lb = lessThan(ivec3(lessThan(p, ivec3(0)))*d, ivec3(0));
	bvec3 hb = greaterThan(ivec3(greaterThanEqual(p, e))*d, ivec3(0));
	return lb.x || lb.y || lb.z || hb.x || hb.y || hb.z;
}

float get_depth(float z) {
	return 0.5 - 0.5*(u_proj[2][2] + u_proj[3][2]/z);
}

vec4 sample_int(ivec3 p, int lod) {
	return texelFetch(u_texture, p, lod);
}
vec4 sample_float(vec3 p, int lod) {
	 return textureLod(u_texture, cell_to_uni(p, lod), lod);
}

bool is_solid(ivec3 p, int lod) {
	return sample_int(p, lod).a > 0.1;
}

float get_occlusion(vec3 p, ivec3 ip, ivec3 n, int lod) {
	ivec3 bx = ivec3(n.x == 0, n.x != 0, 0);
	ivec3 by = ivec3(0, n.z != 0, (n.x != 0) || (n.y != 0));
	ivec3 bp = ip + n;
	vec3 c = fract(p*get_size_lod(u_size, lod)) - vec3(0.5);
	float x = dot(c, vec3(bx));
	float y = dot(c, vec3(by));
	float s = 1.0;
	if(!is_solid(bp, lod)) {
		s += (0.5 + x)*float(!is_solid(bp + bx, lod));
		s += (0.5 + y)*float(!is_solid(bp + by, lod));
		s += (0.5 - x)*float(!is_solid(bp - bx, lod));
		s += (0.5 - y)*float(!is_solid(bp - by, lod));
		s += (0.5 + x)*(0.5 + y)*float(!is_solid(bp + bx + by, lod));
		s += (0.5 - x)*(0.5 + y)*float(!is_solid(bp - bx + by, lod));
		s += (0.5 - x)*(0.5 - y)*float(!is_solid(bp - bx - by, lod));
		s += (0.5 + x)*(0.5 - y)*float(!is_solid(bp + bx - by, lod));
	}
	return s/4.0;
}

bool project_on_box(inout vec3 pos, in vec3 dir, inout vec3 norm, inout float dist) {
	if(!is_outside_float(pos, vec3(0), vec3(1)))
		return true;
	
	bvec3 bs = lessThan(dir, vec3(0));
	vec3 ts = (vec3(bs) - pos)/dir;
	ivec3 id = 2*ivec3(bs) - ivec3(1);
	ivec3 dip;
	float t = -1.0;
	
	if(ts.x > 0 && (t < 0 || ts.x < t) && !is_outside_float(pos + dir*ts.x, vec3(0), vec3(1))) {
		dip = ivec3(id.x, 0, 0);
		t = ts.x;
	}
	if(ts.y > 0 && (t < 0 || ts.y < t) && !is_outside_float(pos + dir*ts.y, vec3(0), vec3(1))) {
		dip = ivec3(0, id.y, 0);
		t = ts.y;
	}
	if(ts.z > 0 && (t < 0 || ts.z < t) && !is_outside_float(pos + dir*ts.z, vec3(0), vec3(1))) {
		dip = ivec3(0, 0, id.z);
		t = ts.z;
	}
	if(t < 0)
		return false;
	
	pos += dir*t;
	if(is_outside_float(pos, vec3(0), vec3(1)))
		return false;
	norm = vec3(dip);
	dist += t;
	return true;
}

bool trace(inout vec3 pos, out ivec3 ipos, in vec3 dir, inout vec3 norm, inout ivec3 inorm, inout float dist, in int lod) {
	ivec3 size = get_size_lod(u_size, lod);
	vec3 d = uni_to_cell(dir, lod);
	vec3 p = uni_to_cell(pos, lod);
	ivec3 id = ivec3(sign(d));
	ivec3 idn = ivec3(greaterThan(abs(norm), vec3(1.0 - EPSILON)));
	ivec3 ip = idn*ivec3(round(p)) + 
	  (ivec3(1) - idn)*(ivec3(ceil(p))*ivec3(greaterThan(id,ivec3(0,0,0))) + ivec3(floor(p))*ivec3(greaterThan(ivec3(0,0,0),id)));
	
	vec3 sp = p;
	ivec3 dip, cip;
	
	float t = 0.0;
	
	bool found = false;
	int i;
	for(i = 0; i < size[0] + size[1] + size[2] + 3; ++i) {
		vec3 ts;
		
		// choose next intersection plane
		ts = (vec3(ip) - p)/d;
		if(ts.x < ts.y) {
			if(ts.x < ts.z) {
				dip = ivec3(id.x, 0, 0);
				t = ts.x;
			} else {
				dip = ivec3(0, 0, id.z);
				t = ts.z;
			}
		} else {
			if(ts.y < ts.z) {
				dip = ivec3(0, id.y, 0);
				t = ts.y;
			} else {
				dip = ivec3(0, 0, id.z);
				t = ts.z;
			}
		}
		
		// compute intersection parameters
		sp = p + d*t;
		cip = ivec3(floor(sp + 0.5*vec3(dip)));
		
		// increment intersection iterator
		ip += dip;
		
		// not found if point is outside
		if(is_behind_int(ip, id, ivec3(0), size + ivec3(1))) {
			found = false;
			break;
		}
		
		// found if opaque enough
		if(is_solid(cip, lod)) {
			found = true;
			break;
		}
	}
	
	if(found) {
		pos = sp/size;
		ipos = cip;
		inorm = -dip;
		norm = vec3(inorm);
		dist += t;
		return true;
	}
	
	return false;
}


void main() {
	vec4 color = vec4(0.0);
	float shadow = 1.0;
	float depth = 1.0;
	vec3 norm = v_tex_norm.xyz;
	vec3 pos = v_tex_pos.xyz;
	vec3 dir = v_tex_dir.xyz;
	
	int lod = u_lod[0];
	float dist = 0.0;
	
	//if(is_outside_float(pos, vec3(0), vec3(1)))
	if(!project_on_box(pos, dir, norm, dist))
		discard;
	
	depth = gl_FragCoord.z;
	//depth = get_depth(v_z_value);
	
	ivec3 ipos, inorm;
	
	if(!trace(pos, ipos, dir, norm, inorm, dist, lod))
		discard;
	
	color = vec4(sample_int(ipos, lod).rgb, 1.0);
	shadow = get_occlusion(pos, ipos, inorm, lod);
	
	// depth and normal
	depth = get_depth(v_z_value*(1.0 + dist));
	vec3 n = norm;
	
	// light
	vec3 w_pos = (u_model*u_inv_tex*vec4(pos, 1.0)).xyz;
	vec3 w_norm = normalize((u_model*u_inv_tex*vec4(n, 0.0)).xyz);
	vec4 light_pos = u_light_pos;
	vec3 light_dir = normalize(light_pos.xyz - w_pos*light_pos.w);
	vec3 dif = max(dot(w_norm, light_dir), 0.0)*u_light_color.rgb;
	vec3 amb = u_ambient.rgb;
	vec3 new_color = amb*color.rgb*shadow + dif*color.rgb*shadow; 
	color.rgb = new_color.rgb;
	
	// specular
	vec3 v_pos = (u_inv_view*vec4(0,0,0,1)).xyz;
	vec3 v_dir = normalize(w_pos - v_pos);
	vec3 r_dir = v_dir - 2.0*dot(v_dir, w_norm)*w_norm;
	color.rgb += vec3(pow(max(dot(r_dir, light_dir), 0.0), 64.0))*shadow;
	
	gl_FragDepth = depth;
	out_FragColor = vec4(color);
}
