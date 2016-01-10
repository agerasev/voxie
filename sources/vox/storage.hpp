#pragma once

#include <map>

#include <vox/objects/object.hpp>

class Storage {
private:
	std::map<ID,Object*> objects;
public:
	bool insertObject(Object *obj) {
		return objects.insert(std::pair<ID,Object*>(obj->id, obj)).second;
	}
	void removeObject(ID id) {
		objects.erase(id);
	}
	Object *getObject(ID id) {
		auto i = objects.find(id);
		if(i == objects.end())
			return nullptr;
		return i->second;
	}
	
	// iteration
	class iterator {
	private:
		std::map<ID,Object*>::iterator map_iterator;
	public:
		iterator(std::map<ID,Object*>::iterator mi) : map_iterator(mi) {}
		Object *operator * () {
			return map_iterator->second;
		}
		Object *operator -> () {
			return **this;
		}
		bool operator != (const iterator &i) {
			return i.map_iterator != map_iterator;
		}
		bool operator == (const iterator &i) {
			return i.map_iterator == map_iterator;
		}
		iterator &operator ++ () {
			++map_iterator;
			return *this;
		}
	};
	
	iterator begin() {
		return iterator(objects.begin());
	}
	iterator end() {
		return iterator(objects.end());
	}
};
