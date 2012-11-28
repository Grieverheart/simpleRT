#ifndef RT_PHOTONMAP_H
#define RT_PHOTONMAP_H

#include <vector>
#include <glm/glm.hpp>
#include "object.h"
#include "common.h"

struct Photon{
	Photon(void): isShadow(false){};
	glm::vec3 p;
	glm::vec3 dir;
	colorRGBF rgb;
	bool isShadow;
};

class PhotonMap{
public:
	PhotonMap(void): mRoot(NULL){};
	~PhotonMap(void);
	void storePhoton(Photon ph){
		mPhotons.push_back(ph);
	};
	void construct(void);
	std::vector<Photon const*> locate(glm::vec3 position, float radius)const;
private:
	std::vector<Photon> mPhotons;
	struct kdNode{
		kdNode(void): right(NULL), left(NULL), isLeaf(false), photon(NULL){};
		kdNode* right;
		kdNode* left;
		bool isLeaf;
		uchar splitAxis;
		float splitPosition;
		Photon const *photon;
	};
	kdNode *mRoot;
	AABB mAABB;
	kdNode* balance(uint start, uint end);
	void kdClear(kdNode *node);
	void recLocate(kdNode* node, std::vector<Photon const*> &retPhotons, glm::vec3 position, float radius)const;
};

#endif