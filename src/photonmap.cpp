#include "../include/photonmap.h"
#include <algorithm>
#include <iostream>

typedef bool (*compFunc) (Photon, Photon);

bool compX(Photon a, Photon b){
	return (a.p.x < b.p.x);
}

bool compY(Photon a, Photon b){
	return (a.p.y < b.p.y);
}

bool compZ(Photon a, Photon b){
	return (a.p.z < b.p.z);
}

static compFunc compFunctions[] = {compX, compY, compZ};

PhotonMap::~PhotonMap(void){
	if(mRoot != NULL) kdClear(mRoot);
}

void PhotonMap::kdClear(kdNode* node){
	if(node->isLeaf){
		delete node;
		return;
	}
	kdClear(node->left);
	kdClear(node->right);
	delete node;
}

void PhotonMap::construct(void){
	// Find Photon Map Extends
	glm::vec3 min(10000.0f);
	glm::vec3 max(-10000.0f);
	std::vector<Photon>::const_iterator itr;
	for(itr = mPhotons.begin(); itr < mPhotons.end(); itr++){
		for(uint j = 0; j < 3; j++){
			if(itr->p[j] < min[j]) min[j] = itr->p[j];
			if(itr->p[j] > max[j]) max[j] = itr->p[j];
		}
	}
	// Add 0.1 so that we don't get out of bounds
	min = min - 0.1f;
	max = max + 0.1f;
	mAABB.setExtends(min, max);
	mRoot = balance(0, mPhotons.size() - 1);
}

PhotonMap::kdNode* PhotonMap::balance(uint start, uint end){
	kdNode* node = new kdNode();
	if(start == end){
		node->isLeaf = true;
		node->photon = &mPhotons[start];
		return node;
	}
	
	//Find split axis
	uchar axis = 2;
	if(
		(mAABB.bounds[1].x - mAABB.bounds[0].x) > (mAABB.bounds[1].y - mAABB.bounds[0].y) &&
		(mAABB.bounds[1].x - mAABB.bounds[0].x) > (mAABB.bounds[1].z - mAABB.bounds[0].z)
	) axis = 0;
	else if(
		(mAABB.bounds[1].y - mAABB.bounds[0].y) > (mAABB.bounds[1].z - mAABB.bounds[0].z)
	) axis = 1;
	node->splitAxis = axis;
	
	//Sort with respect to axis and find split position
	std::sort(mPhotons.begin() + start, mPhotons.begin() + end + 1, compFunctions[axis]);
	uint median = start + (end - start) / 2;
	node->splitPosition = mPhotons[median].p[axis];
	
	//Balance left node
	float temp = mAABB.bounds[1][axis];
	mAABB.bounds[1][axis] = node->splitPosition;
	node->left = balance(start, median);
	mAABB.bounds[1][axis] = temp;
	
	//Balance right node
	temp = mAABB.bounds[0][axis];
	mAABB.bounds[0][axis] = node->splitPosition;
	node->right = balance(median + 1, end);
	mAABB.bounds[0][axis] = temp;
	
	return node;
}


std::vector<Photon const*> PhotonMap::locate(glm::vec3 position, float radius)const{
	std::vector<Photon const*> retPhotons;
	recLocate(mRoot, retPhotons, position, radius);
	return retPhotons;
}

void PhotonMap::recLocate(kdNode* node, std::vector<Photon const*> &retPhotons, glm::vec3 position, float radius)const{
	if(node->isLeaf){
		Photon const* photon = node->photon;
		glm::vec3 distance = photon->p - position;
		if(glm::length(distance) < radius) retPhotons.push_back(photon);
		return;
	}
	float dl, dr;
	uchar axis = node->splitAxis;
	dl = position[axis] - radius;
	dr = position[axis] + radius;
	
	float splitPos = node->splitPosition;
	
	if(dl < splitPos){
		recLocate(node->left, retPhotons, position, radius);
	}
	if(dr > splitPos){
		recLocate(node->right, retPhotons, position, radius);
	}
}