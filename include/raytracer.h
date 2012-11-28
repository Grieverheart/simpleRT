#ifndef RAYTRACER_H
#define RAYTRACER_H

#include "common.h"
#include "scene.h"
#include "camera.h"
#include "grid.h"
#include "photonmap.h"
#include <boost/random/mersenne_twister.hpp>

class RayTracer{
public:
	RayTracer(uint width, uint height);
	~RayTracer(void);
	void Trace(CameraBase &camera);
	void Init(Scene *scene);
	uchar const* readBuffer(void){return mBuffer;};
	
private:
	void traceRay(Ray &ray, colorRGBF &pixelColor, uint level, float Rcoef)const;
	float mtRandf(float x, bool isSymmetric)const;
	int mtRandi(int x);
	glm::vec3 mtRandSphere(void)const;
	glm::vec3 mtRandCosine(glm::vec3 dir)const;
	glm::vec3 mtRandCone(float mincos)const;
	void genPhotonMap(void);
	void tracePhoton(Photon &photon, uint level);
	void traceShadowPhoton(Ray ray, uint objectID);
	colorRGBF calcDiffuse(glm::vec3 position, glm::vec3 I, glm::vec3 N, Material mat)const;
	colorRGBF calcIndirect(glm::vec3 position, glm::vec3 N, float &nShadowPhotons)const;
	uint mWidth, mHeight;
	uint mDepth;
	uint mPhotonDepth;
	uint mNPhotons;
	uint mNObjects, mNPointLights, mNPlanes;
	Scene *mScene;
	Grid mGrid;
	uchar *mBuffer;
	boost::random::mt19937 randGen_;
	PhotonMap mPhotonMap;
};


#endif
