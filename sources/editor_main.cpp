#include "vox/engine.hpp"

class VoxelEditor : public Input::MouseListener {
private:
	Engine &engine;
	Graphics *gfx;
	Camera &cam;
	VoxelMap &vox;
	
public:
	VoxelEditor(Engine &e) : engine(e), gfx(e.getGraphics()), cam(e.getStorage()->cam), vox(e.getStorage()->vox) {
		
	}

	virtual void down(int b, int x, int y) override {
		if(b == 1) {
			add(x, y);
		} else if(b == 0) {
			remove(x, y);
		}
		highlight(x, y);
	}
	virtual void up(int b, int x, int y) override {
		
	}
	virtual void move(int x, int y) override {
		highlight(x, y);
	}
	
	void add(int mx, int my) {
		ivec3 under, over;
		if(intersect(mx, my, under, over)) {
			ivec3 bs = vox.size;
			if(over.x() >= 0 && over.x() < bs[0] && over.y() >= 0 && over.y() < bs[1] && over.z() >= 0 && over.z() < bs[2]) {
				int i = 4*((over.z()*bs[1] + over.y())*bs[0] + over.x());
				vox.data[i + 3] = 0xFF;
				vox.write(vox.data.data() + i, over, ivec3(1,1,1));
			}
		}
	}
	
	void remove(int mx, int my) {
		ivec3 under, over;
		if(intersect(mx, my, under, over)) {
			ivec3 bs = vox.size;
			if(under.x() >= 0 && under.x() < bs[0] && under.y() >= 0 && under.y() < bs[1] && under.z() >= 0 && under.z() < bs[2]) {
				int i = 4*((under.z()*bs[1] + under.y())*bs[0] + under.x());
				vox.data[i + 3] = 0x00;
				vox.write(vox.data.data() + i, under, ivec3(1,1,1));
			}
		}
	}
	
	ivec3 prev = fvec3(-1,-1,-1);
	void highlight(int mx, int my) {
		ivec3 next = fvec3(-1,-1,-1);
		ivec3 under, over;
		if(intersect(mx, my, under, over)) {
			next = under;
		}
		if(next.x() != prev.x() || next.y() != prev.y() || next.z() != prev.z()) {
			ivec3 bs = vox.size;
			if(prev.x() >= 0 && prev.x() < bs[0] && prev.y() >= 0 && prev.y() < bs[1] && prev.z() >= 0 && prev.z() < bs[2]) {
				int i = 4*((prev.z()*bs[1] + prev.y())*bs[0] + prev.x());
				vox.data[i + 0] = 0xa0;
				vox.data[i + 1] = 0xa0;
				vox.data[i + 2] = 0xa0;
				vox.write(vox.data.data() + i, prev, ivec3(1,1,1));
			}
			if(next.x() >= 0 && next.x() < bs[0] && next.y() >= 0 && next.y() < bs[1] && next.z() >= 0 && next.z() < bs[2]) {
				int i = 4*((next.z()*bs[1] + next.y())*bs[0] + next.x());
				vox.data[i + 0] = 0xff;
				vox.data[i + 1] = 0xff;
				vox.data[i + 2] = 0x00;
				vox.write(vox.data.data() + i, next, ivec3(1,1,1));
			}
			prev = next;
		}
	}
	
	bool intersect(int mx, int my, ivec3 &under, ivec3 &over) {
		int width = gfx->width, height = gfx->height;
		Projector proj = gfx->proj;
		fmat4 view =cam.view;
		fmat4 model = gfx->model;
		
		fvec2 mv(2*(float)mx/width - 1, 1 - 2*(float)my/height);
		fvec4 view_pos(mv[0]*proj.w, mv[1]*proj.h, -proj.n, 1.0);
		fvec4 view_dir(view_pos.sub<3>(), 0.0);
		fmat4 imv = invert(model*view);
		fvec3 pos = (imv*view_pos).sub<3>() + vec3(0.5,0.5,0.5);
		fvec3 dir = (imv*view_dir).sub<3>();
		
		fvec3 size = vox.size;
		fvec3 d = dir*size;
		fvec3 p = pos*size;
		ivec3 id((d.x() > 0) - (d.x() < 0), (d.y() > 0) - (d.y() < 0), (d.z() > 0) - (d.z() < 0));
		ivec3 ip(
		  int(ceil(p.x()))*(id.x() > 0) + int(floor(p.x()))*(id.x() < 0),
		  int(ceil(p.y()))*(id.y() > 0) + int(floor(p.y()))*(id.y() < 0),
		  int(ceil(p.z()))*(id.z() > 0) + int(floor(p.z()))*(id.z() < 0)
		);
		ivec3 dip(0,0,0);
		
		fvec3 sp = p, cp = p;
		float color = 0;
		
		bool found = false;
		for(int i = 0; i < 0x100; ++i) {
			// break if point is outside
			//if(is_outside(sp,d,size)) {
			//	break;
			//}
			
			// get color and break if opaque enough
			ivec3 icp(floor(cp.x()), floor(cp.y()), floor(cp.z()));
			ivec3 bs = vox.size;
			if(icp.x() >= 0 && icp.x() < bs[0] && icp.y() >= 0 && icp.y() < bs[1] && icp.z() >= 0 && icp.z() < bs[2])
				color = vox.data[4*((icp.z()*bs[1] + icp.y())*bs[0] + icp.x()) + 3];
				if(color > 0.9) {
					under = icp;
					over = icp - dip;
					found = true;
					break;
				}
			
			// choose next intersection plane
			fvec3 ts;
			float t;
			ts = (fvec3(ip) - p)/d;
			if(ts.x() < ts.y()) {
				if(ts.x() < ts.z()) {
					dip = ivec3(id.x(),0,0);
					t = ts.x();
				} else {
					dip = ivec3(0,0,id.z());
					t = ts.z();
				}
			} else {
				if(ts.y() < ts.z()) {
					dip = ivec3(0,id.y(),0);
					t = ts.y();
				} else {
					dip = ivec3(0,0,id.z());
					t = ts.z();
				}
			}
			
			// compute next intersection parameters
			sp = p + d*t;
			cp = sp + 0.5f*fvec3(dip);
			
			// increment intersection iterator
			ip += dip;
		}
		
		if(found) {
			//printf("%d %d %d\n", under(0), under(1), under(2));
			//fflush(stdout);
			return true;
		}
		
		return false;
	}
};

int main(int argc, char *argv[]) {
	
	Engine engine;
	
	VoxelMap &vox = engine.getStorage()->vox;
	
	if(argc >= 2 && !vox.fileLoad(argv[1], true)) {
		
	} else {
		ivec3 size(16,16,16);
		if(argc >= 5) {
			size = ivec3(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
		}
		std::vector<unsigned char> data;
		data.resize(4*size[0]*size[1]*size[2]);
		for(int i = 0; i < size[0]*size[1]*size[2]; ++i) {
			data[4*i + 0] = 0xA0;
			data[4*i + 1] = 0xA0;
			data[4*i + 2] = 0xA0;
			data[4*i + 3] = 0xFF;
		}
		vox.init(size, data.data(), true);
	}
	
	VoxelEditor editor(engine);
	engine.getInput()->setMouseListener(&editor);
	engine.loop();
	
	if(argc >= 2) {
		vox.fileSave(argv[1]);
	}
	
	return 0;
}
