#define GLM_SWIZZLE
#include <glm/gtc/matrix_transform.hpp>
#include "../include/scene.h"
#include <sstream>
#include <fstream>
#include <iostream>

Scene::~Scene(void){
	for(uint i = 0; i < mNObjects; i++){
		delete mObjects[i];
	}
	for(uint i = 0; i < mNPlanes; i++){
		delete mPlanes[i];
	}
	for(uint i = 0; i < mNPointLights; i++){
		if(!mPointLights[i]->mIsAreaLight) delete mPointLights[i];
	}
	for(uint i = 0; i < mNAreaLights; i++){
		delete mAreaLights[i];
	}
}

void Scene::addSphere(glm::vec3 position, float radius, Material& material){
	Sphere* tempSphere = new Sphere(position, radius, material);
	mObjects.push_back(tempSphere);
	mNObjects++;
}

void Scene::addPlane(glm::vec3 normal, glm::vec3 point, Material& material){
	Plane* tempPlane = new Plane(normal, point, material);
	mPlanes.push_back(tempPlane);
	mNPlanes++;
}

void Scene::addTriangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, Material& material){
	Triangle* tempTriangle = new Triangle(v0, v1, v2, material);
	mObjects.push_back(tempTriangle);
	mNObjects++;
}

void Scene::addPointLight(glm::vec3 position, colorRGBF color){
	PointLight *tempLight = new PointLight(position, color);
	mPointLights.push_back(tempLight);
	mNPointLights++;
}

void Scene::addAreaLight(glm::vec3 position, glm::vec3 normal, float radius, colorRGBF color, uint nPoints){
	AreaLight *tempLight = new AreaLight(position, normal, radius, color, nPoints);
	mAreaLights.push_back(tempLight);
	for(uint i = 0; i < nPoints; i++) mPointLights.push_back(&(tempLight->mPointLights[i]));
	mNAreaLights++;
	mNPointLights += nPoints;
}

int Scene::addPolyhedronType(std::string objFile){
	PolyhedronType tempPolyType ;
	if(!parsePolyObj(objFile, tempPolyType)) return -1;
	mTypes.push_back(tempPolyType);
	mNTypes++;
	return mNTypes - 1;
}

void Scene::addPolyhedron(int objectID, glm::vec3 position, Material &material, glm::vec4 rotation, float scale){
	if(objectID < 0){
		std::cout << "Polyhedron type unknown." << std::endl;
		return;
	}
	Polyhedron* tempPolyhedron = new Polyhedron(mTypes[objectID], position, material, rotation, scale);
	mObjects.push_back(tempPolyhedron);
	mNObjects++;
}

void Scene::translate(glm::vec3 trVector){
	mModelMatrix = glm::translate(mModelMatrix, trVector);
}

void Scene::rotate(glm::vec4 rotVector){
	glm::vec3 axis = rotVector.yzw();
	mModelMatrix = glm::rotate(mModelMatrix, rotVector.x, axis);
}

Object* Scene::object(uint i)const{
	return mObjects[i];
}

Plane* Scene::plane(uint i)const{
	return mPlanes[i];
}

PointLight const& Scene::pointLight(uint i)const{
	return *mPointLights[i];
}

bool Scene::parsePolyObj(std::string objFile, PolyhedronType &pType){
	/* Parse File */
	const char *filepath = objFile.c_str();
	std::ifstream file(filepath);
	if(!file){
		std::cout << "Error parsing file \"" << objFile << "\" by function " << "'" << __FUNCTION__ << "'" << std::endl;
		return false;
	}
	std::cout << "Parsing " << objFile << "." << std::endl;
	
	std::string line;
	while(std::getline(file, line)){
		if(line[0] == 'v'){
			std::istringstream s(line.substr(2));
			glm::vec3 vertex;
			s >> vertex[0] >> vertex[1] >> vertex[2] >> std::ws;
			pType.mVertices.push_back(vertex);
		}
		else if(line[0] == 'f'){
			std::istringstream s(line.substr(2));
			std::vector<uint> face;
			uint faceVertexIndex;
			while(s >> faceVertexIndex) face.push_back(faceVertexIndex - 1);
			uint facesize = face.size();
			if(facesize > 3){
				for(uint i = 1; i < facesize - 1; i++){
					glm::ivec3 triangle(face[0], face[i], face[i + 1]);
					pType.mTrVertIndices.push_back(triangle);
				}
			}
			else{
				glm::ivec3 triangle(face[0], face[1], face[2]);
				pType.mTrVertIndices.push_back(triangle);
			}
		}
	}
	file.close();
	return true;
}