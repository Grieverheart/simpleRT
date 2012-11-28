#include "../include/raytracer.h"
#include <iostream>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_01.hpp>

RayTracer::RayTracer(uint width, uint height): mWidth(width), mHeight(height){
	mDepth = 3;
	mPhotonDepth = 6;
	mNPhotons = 1000000;
	mBuffer = new uchar[3*width*height]();
	randGen_.seed(0);
}

RayTracer::~RayTracer(void){
	delete[] mBuffer;
}

float RayTracer::mtRandf(float x, bool isSymmetric)const{
	static boost::random::uniform_01<boost::mt19937, float> mtUniReal(randGen_);
	return isSymmetric? x * (2.0f * mtUniReal() - 1.0f) : x * mtUniReal();
}

int RayTracer::mtRandi(int x){
	static boost::random::uniform_int_distribution<> mtUniInt(0);
	return mtUniInt(randGen_) % x;
}

glm::vec3 RayTracer::mtRandSphere(void)const{
	float x1 = mtRandf(1.0f, true);
	float x2 = mtRandf(1.0f, true);
	float s = x1 * x1 + x2 * x2;
	while(s > 1.0f){
		x1 = mtRandf(1.0f, true);
		x2 = mtRandf(1.0f, true);	
		s = x1 * x1 + x2 * x2;	
	}
	float z = 1.0f - 2.0f * s;
	s = sqrt(1.0f - s);
	float x = 2.0f * x1 * s;
	float y = 2.0f * x2 * s;
	return glm::vec3(x, y, z);
}

glm::vec3 RayTracer::mtRandCosine(glm::vec3 dir)const{
	glm::vec3 w = mtRandSphere() + dir;
	float a = glm::length(w);
	while(a < 0.0001f){
		w = mtRandSphere() + dir;
		a = glm::length(w);
	}
	return w / a;
}

glm::vec3 RayTracer::mtRandCone(float mincos)const{
	float phi = mtRandf(2.0f * M_PI, false);
	float z = mtRandf(1.0f - mincos, false);
	z += mincos;
	glm::vec3 retVec;
	retVec.z = z;
	z = sqrt(1.0f - z * z);
	retVec.x = z * cos(phi);
	retVec.y = z * sin(phi);
	
	return retVec;
}


static inline glm::vec3 reflect(glm::vec3 I, glm::vec3 N){
	return glm::normalize(I - 2.0f * glm::dot(N, I) * N);
}

static inline float minf(float a, float b){
	if(a < b) return a;
	else return b;
}

static inline float maxf(float a, float b){
	if(a > b) return a;
	else return b;
}

static uint photonCount = 0;
void RayTracer::genPhotonMap(void){

	//Get Scene BBox
	AABB aabb = mGrid.getAABB();
	glm::vec3 aabbPosition = 0.5f * (aabb.bounds[1] + aabb.bounds[0]);
	glm::vec3 aabbVertices[8] = {
		glm::vec3(aabb.bounds[0].x, aabb.bounds[0].y, aabb.bounds[0].z),
		glm::vec3(aabb.bounds[1].x, aabb.bounds[0].y, aabb.bounds[0].z),
		glm::vec3(aabb.bounds[0].x, aabb.bounds[1].y, aabb.bounds[0].z),
		glm::vec3(aabb.bounds[1].x, aabb.bounds[1].y, aabb.bounds[0].z),
		glm::vec3(aabb.bounds[0].x, aabb.bounds[0].y, aabb.bounds[1].z),
		glm::vec3(aabb.bounds[1].x, aabb.bounds[0].y, aabb.bounds[1].z),
		glm::vec3(aabb.bounds[0].x, aabb.bounds[1].y, aabb.bounds[1].z),
		glm::vec3(aabb.bounds[1].x, aabb.bounds[1].y, aabb.bounds[1].z)
	};

	float lightWeights[mNPointLights];
	float minCosines[mNPointLights];
	glm::mat3 rotMatrices[mNPointLights];
	float totalLight = 0.0f;
	
	//Calculate total light and precompute rotation matrices for random vectors for each light
	for(uint i = 0; i < mNPointLights; i++){
		totalLight += mScene->pointLight(i).mPower;
		glm::vec3 lightPosition = mScene->pointLight(i).mPosition;
		glm::vec3 planeNormal = glm::normalize(aabbPosition - lightPosition);
		float mincos = 1.0f;
		for(uint j = 0; j < 8; j++){
			glm::vec3 normDir = glm::normalize(aabbVertices[j] - lightPosition);
			float costheta = glm::dot(normDir, planeNormal);
			if(costheta < mincos) mincos = costheta;
		}
		minCosines[i] = mincos;
		
		
		//////////Rotation Matrix using Householder transform////////
		static const glm::vec3 p = glm::vec3(0.0f, 1.0f, 0.0f);
		static const glm::vec3 u = glm::vec3(0.0f, 1.0f, -1.0f);
		static const glm::mat3 unitMat(1.0f);
		glm::vec3 v = p - planeNormal;
		float vv = 2.0f / glm::dot(v, v);
		float uv = (u.y * v.y + u.z * v.z) * vv;
		
		for(uint k = 0; k < 3; k++){
			for(uint j = 0; j < 3; j++){
				(rotMatrices[i])[k][j] = unitMat[k][j] - u[k] * u[j] - vv * v[k] * v[j] + uv * v[j] * u[k];
			}
		}
	}
	//Calculate light weights
	for(uint i = 0; i < mNPointLights; i++){
		lightWeights[i] = mScene->pointLight(i).mPower / totalLight;
	}
	
	//////////////////////Generate Photons//////////////////////
	while(photonCount < mNPhotons){
		//distribute photons to the different lights
		float rnd = mtRandf(1.0f, false);
		uint lightID = 0;
		for(uint i = 0; i < mNPointLights; i++){
			rnd -= lightWeights[i];
			if(rnd < 0.0f){
				lightID = i;
				break;
			}
		}
		Photon photon;
		photon.rgb = mScene->pointLight(lightID).mColor;
		photon.p = mScene->pointLight(lightID).mPosition;
		// photon.dir = mtRandSphere(); /* This is slow for point light sources that are not enclosed in some volume! */
		// while(photon.dir.y > 0.0f) photon.dir = mtRandSphere();
		photon.dir = rotMatrices[lightID] * mtRandCone(minCosines[lightID]);
		tracePhoton(photon, 0);
		//To Add: Scale Photon
	}
	mPhotonMap.construct();
}

void RayTracer::tracePhoton(Photon &photon, uint level){
	if(level > mPhotonDepth) return;
	float t = 2000.0f;
	Ray ray(photon.p, photon.dir);
	bool isIntersect = false;
	uint currObject = 0;
	glm::vec3 normal;
	if(mGrid.intersect(ray, t, currObject, normal)) isIntersect = true;
	if(!isIntersect) return;
	Material objectMaterial = mScene->object(currObject)->mMaterial;
	photon.p = ray.r0 + ray.dir * t;
	if(level > 0){
		photon.rgb *= objectMaterial.color;
		mPhotonMap.storePhoton(photon); //First hit is direct light. We already calculate that with ray tracing
		photonCount++;
	}
	// else traceShadowPhoton(Ray(photon.p, ray.dir), currObject);
	
	//Reflect Photon Diffusely
	if(mtRandf(1.0f, false) < objectMaterial.diffusivity){
		//Reflect photon
		photon.dir = mtRandCosine(normal);
		// photon.dir = reflect(photon.dir, normal);
		tracePhoton(photon, level + 1);
	}
	return;
}

void RayTracer::traceShadowPhoton(Ray ray, uint objectID){
	Ray shadowRay = Ray(ray);
	float t = 2000.0f;
	bool isIntersect = false;
	uint currObject = objectID;
	glm::vec3 normal;
	while(objectID == currObject){
		t = 2000.0f;
		shadowRay.r0 += shadowRay.dir * 0.0001f;
		if(mGrid.intersect(shadowRay, t, currObject, normal)) isIntersect = true;
		if(!isIntersect) return;
	}
	// if(glm::dot(normal, -shadowRay.dir) < 0.0f) return;
	Photon shadowPhoton;
	shadowPhoton.p = shadowRay.r0 + shadowRay.dir * t;
	shadowPhoton.dir = shadowRay.dir;
	shadowPhoton.isShadow = true;
	mPhotonMap.storePhoton(shadowPhoton); //First hit is direct light. We already calculate that with ray tracing
	return;
}

static uint nRays = 0;
void RayTracer::traceRay(Ray &ray, colorRGBF &pixelColor, uint level, float Rcoef)const{
	if(level > mDepth || Rcoef < 0.01f) return;
	float t = 2000.0f;
	bool isIntersect = false;
	uint currObject = 0;
	glm::vec3 normal;
	if(mGrid.intersect(ray, t, currObject, normal)) isIntersect = true;
	
	if(!isIntersect){
		if(level == 0) pixelColor = colorRGBF(1.0f); //background color
		return;
	}
	nRays++;
	glm::vec3 intersection = ray.r0 + ray.dir * t;
	Material objectMaterial = mScene->object(currObject)->mMaterial;
	
	float nShadowPhotons;
	pixelColor += Rcoef * calcIndirect(intersection, normal, nShadowPhotons);
	
	pixelColor += Rcoef * calcDiffuse(intersection, ray.dir, normal, objectMaterial);
	
	colorRGBF reflColor;
	ray.r0 += glm::cross(normal, glm::cross(ray.dir, normal));
	Ray reflectedray = Ray(intersection, reflect(ray.dir, normal));
	traceRay(reflectedray, reflColor, level + 1, Rcoef * objectMaterial.reflectivity);
	pixelColor += Rcoef * reflColor * objectMaterial.color;
	return;
}

colorRGBF RayTracer::calcIndirect(glm::vec3 position, glm::vec3 N, float &nShadowPhotons)const{
	colorRGBF pixelColor;
	nShadowPhotons = 0.0f;
	std::vector<Photon const*> photons = mPhotonMap.locate(position, 0.2f);
	uint nPhotons = photons.size();
	static float normalization = mNPointLights * (200.0f / M_PI) / (mNPhotons * 0.2f * 0.2f);
	if(nPhotons > 8){
		colorRGBF indColor;
		for(uint i = 0; i < nPhotons; i++){
			// if(photons[i]->isShadow) nShadowPhotons++;
			// else{
				float diffuse = glm::dot(-photons[i]->dir, N);
				if(diffuse < 0) continue;
				colorRGBF photonColor = photons[i]->rgb;
				indColor += diffuse * photonColor;
			// }
		}
		pixelColor += indColor * normalization;
		// nShadowPhotons = nShadowPhotons / nPhotons;
	}
	return pixelColor;
}

colorRGBF RayTracer::calcDiffuse(glm::vec3 position, glm::vec3 I, glm::vec3 N, Material mat)const{
	colorRGBF pixelColor;
	Ray lightRay;
	lightRay.r0 = position;
	for(uint lightID = 0; lightID < mNPointLights; lightID++){
		colorRGBF lightColor = mScene->pointLight(lightID).mColor;
		lightRay.dir = mScene->pointLight(lightID).mPosition - lightRay.r0;
		float d = glm::length(lightRay.dir);
		if(glm::dot(lightRay.dir, N) <= 0.0f) continue;
		lightRay.dir = lightRay.dir / d;
		// lightRay.r0 += lightRay.dir * 1.0001f; //Bump Ray
		bool isInShadow = false;
		isInShadow = mGrid.shadowIntersect(lightRay, d);
		if(!isInShadow){
			// lambert
			float diffuse = glm::dot(lightRay.dir, N);
			pixelColor += diffuse * lightColor * mat.color;
			// blinn
			glm::vec3 halfVector = lightRay.dir - I;
			float temp = glm::length(halfVector);
			if(temp > 0.0f){
				halfVector = halfVector / temp;
				float spec = maxf(glm::dot(halfVector, N), 0.0f);
				spec = mat.Sv * pow(spec, mat.Sp);
				pixelColor += spec * lightColor;
			}
		}
	}
	return pixelColor;
}


void RayTracer::Init(Scene *scene){
	mScene = scene;
	mNObjects = scene->nObjects();
	mNPointLights = scene->nPointLights();
	mNPlanes = scene->nPlanes();
	mGrid.construct(mScene);
	genPhotonMap();
}

static inline uchar srgbEncode(float c){
	static const float gamma = 1.0f / 2.2f;
	if (c < 0.018) return uchar(1118.93f * c); 
	else return uchar(275.141f * pow(c, gamma) - 20.141f); // Inverse gamma 2.2
}

void RayTracer::Trace(CameraBase &camera){
	uint nSamples = camera.getSamples();
	nSamples *= nSamples;
	std::cout.precision(3);
	std::cout.width(3);
	int percentage = -1;
	#pragma omp parallel for schedule(dynamic)
	for(uint i = 0; i < mWidth; i++){
		for(uint j = 0; j < mHeight; j++){
			// int percentage_new = int(100.0f * (mHeight * i + j) / (mWidth * mHeight - 1.0f));
			// if(percentage_new != percentage){
				// std::cout << "\r" << std::flush;
				// std::cout << percentage_new << "\%";
				// percentage = percentage_new;
			// }
			colorRGBF pixelColor;
			for(uint sample = 0; sample < nSamples; sample++){
				float coef = 1.0f;
				colorRGBF sampleColor;
				Ray ray = camera.shootRay(i, j, sample);
				uint level = 0;
				traceRay(ray, sampleColor, level, coef);
				pixelColor += sampleColor;
			}
			mBuffer[3*i + 3*mWidth*j + 0] = srgbEncode(minf(pixelColor.r / nSamples, 1.0f));
			mBuffer[3*i + 3*mWidth*j + 1] = srgbEncode(minf(pixelColor.g / nSamples, 1.0f));
			mBuffer[3*i + 3*mWidth*j + 2] = srgbEncode(minf(pixelColor.b / nSamples, 1.0f));
		}
	}
	std::cout << std::endl;
	std::cout << nRays << std::endl;
}