#include "../include/grid.h"

static inline float maxf(float a, float b){
	float retVal = a;
	if(retVal < b) retVal = b;
	return retVal;
}

static inline float minf(float a, float b){
	float retVal = a;
	if(retVal > b) retVal = b;
	return retVal;
}

static inline int maxi(int a, int b){
	int retVal = a;
	if(retVal < b) retVal = b;
	return retVal;
}

static inline int mini(int a, int b){
	int retVal = a;
	if(retVal > b) retVal = b;
	return retVal;
}

void Grid::construct(Scene *scene){
	uint nObjects = scene->nObjects();
	uint nPrimitives = 0;
	glm::vec3 min(10000.0f);
	glm::vec3 max(-10000.0f);
	// Find Scene Extends
	for(uint  i = 0; i < nObjects; i++){
		if(scene->object(i)->mType == POLYHEDRON){
			Polyhedron* polyhedron = (Polyhedron*)scene->object(i);
			uint nTriangles = polyhedron->nTriangles();
			for(uint j = 0; j < 3; j++){
				if(polyhedron->mAABB.bounds[0][j] < min[j]) min[j] = polyhedron->mAABB.bounds[0][j];
				if(polyhedron->mAABB.bounds[1][j] > max[j]) max[j] = polyhedron->mAABB.bounds[1][j];
			}
			nPrimitives += nTriangles;
		}
		else{
			Sphere* sphere = (Sphere*)scene->object(i);
			for(uint j = 0; j < 3; j++){
				if(sphere->mAABB.bounds[0][j] < min[j]) min[j] = sphere->mAABB.bounds[0][j];
				if(sphere->mAABB.bounds[1][j] > max[j]) max[j] = sphere->mAABB.bounds[1][j];
			}
			nPrimitives++;
		}
	}
	// Add 0.1 so that we don't get out of bounds
	min = min - 0.1f;
	max = max + 0.1f;
	mAABB.setExtends(min, max);
	
	//Calculate grid properties
	glm::vec3 gridSize = max - min;
	float cubeRoot = pow((5.0f * nPrimitives) / (gridSize[0] * gridSize[1] * gridSize[2]), 0.333);
	for(uint i = 0; i < 3; i++){
		float temp = gridSize[i] * cubeRoot;
		temp = maxf(1.0f, minf(temp, 128.0f)); //Minimum of 1 cell and maximum of 128 cells in each dimension
		mRes[i] = (uint)temp;
	}
	mCellDim = gridSize / glm::vec3(mRes[0], mRes[1], mRes[2]);
	
	//Alocate memory
	mCells = new Cell* [mRes[0] * mRes[1] * mRes[2]] (); //Notice the parentheses. This initializes the pointers to NULL
	//Insert Objects in grid
	for(uint  i = 0; i < nObjects; i++){
		if(scene->object(i)->mType == POLYHEDRON){
			Polyhedron* polyhedron = (Polyhedron*)scene->object(i);
			uint nTriangles = polyhedron->nTriangles();
			for(uint j = 0; j < nTriangles; j++){
				Triangle *triangle = polyhedron->triangle(j);
				glm::vec3 minTri = triangle->mAABB.bounds[0];
				glm::vec3 maxTri = triangle->mAABB.bounds[1];
				//convert AABB to cell coordinates
				minTri = (minTri - mAABB.bounds[0]) / mCellDim;
				maxTri = (maxTri - mAABB.bounds[0]) / mCellDim;
				for(uint z = uint(minTri.z); z <= uint(maxTri.z); z++){
					for(uint y = uint(minTri.y); y <= uint(maxTri.y); y++){
						for(uint x = uint(minTri.x); x <= uint(maxTri.x); x++){
							uint index = x + y * mRes[0] + z * mRes[0] * mRes[1];
							if(mCells[index] == NULL) mCells[index] = new Cell;
							mCells[index]->insert(triangle, i);
						}
					}
				}
			}
		}
		else{
			Sphere* sphere = (Sphere*)scene->object(i);
			glm::vec3 minSph = sphere->mAABB.bounds[0];
			glm::vec3 maxSph = sphere->mAABB.bounds[1];
			//convert AABB to cell coordinates
			minSph = (minSph - mAABB.bounds[0]) / mCellDim;
			maxSph = (maxSph - mAABB.bounds[0]) / mCellDim;
			for(uint z = uint(minSph.z); z <= uint(maxSph.z); z++){
				for(uint y = uint(minSph.y); y <= uint(maxSph.y); y++){
					for(uint x = uint(minSph.x); x <= uint(maxSph.x); x++){
						uint index = x + y * mRes[0] + z * mRes[0] * mRes[1];
						if(mCells[index] == NULL) mCells[index] = new Cell;
						mCells[index]->insert(sphere, i);
					}
				}
			}
		}
	}
	
}

static inline int clampi(int input, int min, int max){
	return maxi(min, mini(input, max));
}

bool Grid::intersect(Ray ray, float &t, uint &objectID, glm::vec3 &normal)const{
	glm::vec3 invDir = 1.0f / ray.dir;
	glm::vec3 deltaT, nextCrossingT;
	glm::ivec3 exitCell, step;
	
	float tmin = 10000.0f;
	Ray r(ray);
	if(!mAABB.intersect(r, tmin)) return false;
	if(tmin > 0.0f) r.r0 = r.r0 + r.dir * tmin; //If origin outside box, set origin to hit point
	else tmin = 0.0f;
	//Convert ray origin to cell coordinates
	glm::vec3 rayOrigCell = r.r0 - mAABB.bounds[0];
	glm::ivec3 cell = glm::ivec3(rayOrigCell / mCellDim);
	for(uint i = 0; i < 3; i++){
		cell[i] = clampi(cell[i], 0, mRes[i] - 1);
		if(r.dir[i] < 0.0f){
			deltaT[i] = -mCellDim[i] * invDir[i];
			nextCrossingT[i] = tmin + (cell[i] * mCellDim[i] - rayOrigCell[i]) * invDir[i];
			exitCell[i] = -1;
			step[i] = -1;
		}
		else{
			deltaT[i] = mCellDim[i] * invDir[i];
			nextCrossingT[i] = tmin + ((cell[i] + 1) * mCellDim[i] - rayOrigCell[i]) * invDir[i];
			exitCell[i] = mRes[i];
			step[i] = 1;
		}
	}
	
	//Traverse the cells using 3d-DDA
	float retValue = false;
	while(1){
		uint index = cell[0] + cell[1] * mRes[0] + cell[2] * mRes[0] * mRes[1];
		if(mCells[index] != NULL){
			if(mCells[index]->intersect(ray, t, objectID, normal)) retValue = true;
		}
		uchar k = 
			((nextCrossingT[0] < nextCrossingT[1]) << 2) +
			((nextCrossingT[0] < nextCrossingT[2]) << 1) +
			((nextCrossingT[1] < nextCrossingT[2]));
		static const uchar map[8] = {2, 1, 2, 1, 2, 2, 0, 0};
		uchar axis = map[k];
		if(t < nextCrossingT[axis]) break;
		cell[axis] += step[axis];
		if(cell[axis] == exitCell[axis]) break;
		nextCrossingT[axis] += deltaT[axis];
	}
	return retValue;
}

bool Grid::Cell::intersect(Ray ray, float &t, uint &objectID, glm::vec3 &normal){
	std::vector<Item>::iterator itr;
	bool retValue = false;
	// Loop over all primitives in the cell
	for(itr = list.begin(); itr < list.end(); itr++){
		// if(itr->rayID == ray.ID) continue;
		if(itr->object->intersect(ray, t)){
			retValue = true;
			objectID = itr->meshID;
			normal = itr->object->normal();
			
			// itr->rayID = ray.ID;
		}
	}
	return retValue;
}


bool Grid::shadowIntersect(Ray ray, float &t)const{
	glm::vec3 invDir = 1.0f / ray.dir;
	glm::vec3 deltaT, nextCrossingT;
	glm::ivec3 exitCell, step;
	
	float tmin = 10000.0f;
	Ray r(ray);
	if(!mAABB.intersect(r, tmin)) return false;
	if(tmin > 0.0f) r.r0 = r.r0 + r.dir * tmin; //If origin outside box, set origin to hit point
	else tmin = 0.0f;
	//Convert ray origin to cell coordinates
	glm::vec3 rayOrigCell = r.r0 - mAABB.bounds[0];
	glm::ivec3 cell = glm::ivec3(rayOrigCell / mCellDim);
	for(uint i = 0; i < 3; i++){
		cell[i] = clampi(cell[i], 0, mRes[i] - 1);
		if(r.dir[i] < 0.0f){
			deltaT[i] = -mCellDim[i] * invDir[i];
			nextCrossingT[i] = tmin + (cell[i] * mCellDim[i] - rayOrigCell[i]) * invDir[i];
			exitCell[i] = -1;
			step[i] = -1;
		}
		else{
			deltaT[i] = mCellDim[i] * invDir[i];
			nextCrossingT[i] = tmin + ((cell[i] + 1) * mCellDim[i] - rayOrigCell[i]) * invDir[i];
			exitCell[i] = mRes[i];
			step[i] = 1;
		}
	}
	
	//Traverse the cells using 3d-DDA
	while(1){
		uint index = cell[0] + cell[1] * mRes[0] + cell[2] * mRes[0] * mRes[1];
		if(mCells[index] != NULL){
			if(mCells[index]->shadowIntersect(ray, t)) return true;
		}
		uchar k = 
			((nextCrossingT[0] < nextCrossingT[1]) << 2) +
			((nextCrossingT[0] < nextCrossingT[2]) << 1) +
			((nextCrossingT[1] < nextCrossingT[2]));
		static const uchar map[8] = {2, 1, 2, 1, 2, 2, 0, 0};
		uchar axis = map[k];
		cell[axis] += step[axis];
		if(cell[axis] == exitCell[axis]) return false;;
		nextCrossingT[axis] += deltaT[axis];
	}
}

bool Grid::Cell::shadowIntersect(Ray ray, float &t){
	std::vector<Item>::iterator itr;
	// Loop over all primitives in the cell
	for(itr = list.begin(); itr < list.end(); itr++){
		if(itr->object->intersect(ray, t)){
			return true;
		}
	}
	return false;
}