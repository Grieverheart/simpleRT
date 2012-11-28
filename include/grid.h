#ifndef RT_GRID_H
#define RT_GRID_H

#include <vector>
#include <glm/glm.hpp>
#include "scene.h"
#include "object.h"
#include "ray.h"
#include "common.h"

class Grid{
public:
	Grid(void): mCells(NULL){};
	~Grid(void){
		uint nCells = mRes[0] * mRes[1] * mRes[2];
		for(uint i = 0; i < nCells; i++){
			if(mCells[i] != NULL) delete mCells[i];
		}
		delete[] mCells;
	};
	void construct(Scene *scene);
	bool intersect(Ray ray, float &t, uint &objectID, glm::vec3 &normal)const;
	bool shadowIntersect(Ray ray, float &t)const; // Returns as soon as it finds an intersection
	AABB getAABB(void){
		return mAABB;
	}
	
private:
	struct Cell{
		struct Item{
			Item(Object* objPtr, uint mID){
				object = objPtr;
				meshID = mID;
			};
			Object* object;
			uint meshID;
			// int rayID;
		};
		void insert(Object* objPtr, uint mID){
			list.push_back(Item(objPtr, mID));
		}
		bool intersect(Ray ray, float &t, uint &objectID, glm::vec3 &normal);
		bool shadowIntersect(Ray ray, float &t);
		std::vector<Item> list;
	};
	Cell **mCells;
	uint mRes[3];
	glm::vec3 mCellDim;
	AABB mAABB;
};

#endif