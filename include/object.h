#ifndef RTOBJECT_H
#define RTOBJECT_H

#include "common.h"
#include <glm/glm.hpp>
#include <vector>
#include "ray.h"

enum eObjectType{
	SPHERE,
	PLANE,
	TRIANGLE,
	POLYHEDRON
};

struct PolyhedronType{
	std::vector<glm::ivec3> mTrVertIndices;
	std::vector<glm::vec3> mVertices;
};

struct Material{
	colorRGBF color;
	float reflectivity;
	float Sv, Sp;
	float diffusivity;
};


//AABB
class AABB{
public:
	AABB(void){};
	AABB(glm::vec3 min, glm::vec3 max);
	void setExtends(glm::vec3 min, glm::vec3 max);
	bool intersect(Ray ray, float &t)const;
	glm::vec3 bounds[2];
};


//Base Object class
class Object{
public:
	virtual bool intersect(Ray ray, float &t) = 0;
	virtual glm::vec3 normal(void)const = 0;
	eObjectType mType;
	Material mMaterial;
	AABB mAABB;
};

//Sphere derivative
class Sphere: public Object{
public:
	Sphere(glm::vec3 position, float radius, Material& material);
	bool intersect(Ray ray, float &t);
	glm::vec3 normal(void)const{
		return glm::normalize(mIntersection - mPosition);
	};
private:
	float mRadius;
	glm::vec3 mPosition;
	glm::vec3 mIntersection; //Holds the point of intersection on the sphere for returning the normal
};

//Plane derivative
class Plane: public Object{
public:
	Plane(glm::vec3 normal, glm::vec3 point, Material& material);
	bool intersect(Ray ray, float &t);
	glm::vec3 normal(void)const{
		return mNormal;
	};
private:
	glm::vec3 mNormal;
	glm::vec3 mPoint;
};

//Triangle derivative
class Triangle: public Object{
public:
	Triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, Material& material); // CCW
	Triangle(glm::vec3 *v0, glm::vec3 *v1, glm::vec3 *v2, Material& material); // CCW
	~Triangle(void);
	bool intersect(Ray ray, float &t);
	glm::vec3 normal(void)const{
		return mNormal;
	};
private:
	bool isAllocated;
	glm::vec3 mNormal;
	glm::vec3 *mVertices[3]; // We use a pointer so that we can use triangles inside meshes without having duplicate vertices
	// glm::vec3 N, N1, N2;
	// float d, d1, d2;
};

//Polyhedron derivative
class Polyhedron: public Object{
public:
	Polyhedron(PolyhedronType const& polyType, glm::vec3 position, Material& material, glm::vec4 rotation, float scale = 1.0f);
	~Polyhedron(void);
	bool intersect(Ray ray, float &t);
	glm::vec3 normal(void)const{
		return mTriangles[mIntersTriangle].normal();
	};
	Triangle *triangle(uint i){
		return &mTriangles[i];
	};
	uint nTriangles(void){
		return mNTriangles;
	};
private:
	uint mNTriangles;
	std::vector<Triangle> mTriangles;
	std::vector<glm::vec3> mVertices;
	uint mIntersTriangle; //Holds the intersected face id for returning the correct normal
};

#endif
