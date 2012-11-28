#ifndef RTSCENE_H
#define RTSCENE_H

#include <vector>
#include <string>
#include "common.h"
#include <glm/glm.hpp>
#include "object.h"

struct PointLight{
	PointLight(void){};
	PointLight(glm::vec3 position, colorRGBF color): mIsAreaLight(false){
		mPosition = position;
		mColor = color;
		mPower = color.power();
	}
	glm::vec3 mPosition;
	colorRGBF mColor;
	float mPower;
	bool mIsAreaLight;
};

struct AreaLight{
	AreaLight(glm::vec3 position, glm::vec3 normal, float radius, colorRGBF color, uint nPoints){
		mPosition = position;
		mNormal = normal;
		mNPoints = nPoints;
		mColor = color;
		mRadius = radius;
		mPower = color.power();
		mPointLights = new PointLight[nPoints];
		//////////Rotation Matrix using Householder transform////////
		glm::mat3 R;
		static const glm::vec3 p = glm::vec3(0.0f, 1.0f, 0.0f);
		static const glm::vec3 u = glm::vec3(0.0f, 1.0f, -1.0f);
		static const glm::mat3 unitMat(1.0f);
		glm::vec3 v = p - normal;
		float vv = 2.0f / glm::dot(v, v);
		float uv = (u.y * v.y + u.z * v.z) * vv;
		
		for(uint k = 0; k < 3; k++){
			for(uint j = 0; j < 3; j++){
				R[k][j] = unitMat[k][j] - u[k] * u[j] - vv * v[k] * v[j] + uv * v[j] * u[k];
			}
		}
		//////////////////////////////////////////////////////////////
		for(uint i = 0; i < nPoints; i++){
			float theta = i * GOLDEN_ANGLE;
			////Distribute on Sphere
			// float z = (2.0f * i - nPoints + 1.0f) / nPoints;
			// float r = radius * sqrt(1.0f - z * z);
			// mPointLights[i].mPosition = R * (r * glm::vec3(cos(theta), sin(theta), z)) + position;
			float r = radius * sqrt(i) / sqrt(nPoints);
			mPointLights[i].mPosition = R * (r * glm::vec3(cos(theta), sin(theta), 0.0f)) + position;
			mPointLights[i].mColor = color * (1.0f / nPoints);
			mPointLights[i].mPower = mPointLights[i].mColor.power();
			mPointLights[i].mIsAreaLight = true;
		}
	}
	~AreaLight(void){
		delete[] mPointLights;
	};
	glm::vec3 mPosition;
	glm::vec3 mNormal;
	uint mNPoints;
	colorRGBF mColor;
	float mPower;
	float mRadius;
	PointLight *mPointLights;
};

class Scene{
public:
	Scene(void) : mNObjects(0), mNPlanes(0), mNPointLights(0), mNAreaLights(0), mNTypes(0), mModelMatrix(glm::mat4(1.0)){};
	~Scene(void);
	void addSphere(glm::vec3 position, float radius, Material& material);
	void addPlane(glm::vec3 normal, glm::vec3 point, Material& material);
	void addTriangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, Material& material);
	void addPolyhedron(int objectID, glm::vec3 position, Material &material, glm::vec4 rotation, float scale);
	int addPolyhedronType(std::string objFile);
	void addPointLight(glm::vec3 position, colorRGBF color); //Add support for more light types
	void addAreaLight(glm::vec3 position, glm::vec3 normal, float radius, colorRGBF color, uint nPoints);
	uint nObjects(void)const{ return mNObjects;};
	uint nPointLights(void)const{ return mNPointLights;};
	uint nPlanes(void)const{ return mNPlanes;};
	Object* object(uint i)const;
	Plane* plane(uint i)const;
	PointLight const& pointLight(uint i)const;
	void translate(glm::vec3 trVector);
	void rotate(glm::vec4 rotVector);
	
private:
	bool parsePolyObj(std::string, PolyhedronType &pType); //Helper function
	std::vector<PolyhedronType> mTypes;
	std::vector<Object*> mObjects;
	std::vector<Plane*> mPlanes; //Keep planes separate for grid
	std::vector<PointLight*> mPointLights;
	std::vector<AreaLight*> mAreaLights;
	uint mNObjects;
	uint mNPlanes;
	uint mNPointLights;
	uint mNAreaLights;
	uint mNTypes;
	glm::mat4 mModelMatrix;

};


#endif
