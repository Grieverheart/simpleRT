#include "../include/object.h"
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

static inline float sqrf(float a){
	return a * a;
}

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

AABB::AABB(glm::vec3 min, glm::vec3 max){
	bounds[0] = min;
	bounds[1] = max;
}

void AABB::setExtends(glm::vec3 min, glm::vec3 max){
	bounds[0] = min;
	bounds[1] = max;
}

bool AABB::intersect(Ray ray, float &t)const{
	glm::vec3 invDir = 1.0f / ray.dir;
	float t1 = (bounds[0].x - ray.r0.x) * invDir.x;
	float t2 = (bounds[1].x - ray.r0.x) * invDir.x;
	float t3 = (bounds[0].y - ray.r0.y) * invDir.y;
	float t4 = (bounds[1].y - ray.r0.y) * invDir.y;
	float t5 = (bounds[0].z - ray.r0.z) * invDir.z;
	float t6 = (bounds[1].z - ray.r0.z) * invDir.z;
	
	float tmin = maxf(maxf(minf(t1, t2), minf(t3, t4)), minf(t5, t6));
	float tmax = minf(minf(maxf(t1, t2), maxf(t3, t4)), maxf(t5, t6));
	
	if(tmax < 0.0f){
		t = tmax;
		return false;
	}
	
	if(tmin > tmax){
		t = tmax;
		return false;
	}
	
	t = tmin;
	return true;
}

Sphere::Sphere(glm::vec3 position, float radius, Material& material){
	mPosition = position;
	mRadius = radius;
	mType = SPHERE;
	mMaterial = material;
	mAABB.setExtends(glm::vec3(-radius) + position, glm::vec3(radius) + position);
}


bool Sphere::intersect(Ray ray, float &t){
	glm::vec3 direction = mPosition - ray.r0;
	float B = glm::dot(ray.dir, direction);
	float det = sqrf(B) - glm::dot(direction, direction) + sqrf(mRadius);
	if(det < 0.0f) return false;
	float t0 = B + sqrt(det);
	float t1 = B - sqrt(det);
	bool retValue = false;
	if((t0 < t) && (t0 > 0.0001f)){
		t = t0;
		mIntersection = ray.r0 + t * ray.dir;
		retValue = true;
	}
	if((t1 < t) && (t1 > 0.0001f)){
		t = t1;
		mIntersection = ray.r0 + t * ray.dir;
		retValue = true;
	}
	return retValue;
}

Plane::Plane(glm::vec3 normal, glm::vec3 point, Material& material){
	mNormal = glm::normalize(normal);
	mPoint = point;
	mMaterial = material;
	mType = PLANE;
	mAABB.setExtends(glm::vec3(-10000.0f), glm::vec3(10000.0f)); //Does not really matter
}

bool Plane::intersect(Ray ray, float &t){
	float denominator = glm::dot(mNormal, ray.dir);
	if(fabs(denominator) < 0.0001f) return false;
	float numerator = glm::dot(mNormal, (mPoint - ray.r0));
	if(fabs(numerator) > 0.0001f){
		float t1 = numerator / denominator;
		if(t1 < t && t1 > 0.0001f){
			t = t1;
			return true;
		}
	}
	return false;
}


Triangle::Triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, Material& material){
	isAllocated = true;
	mVertices[0] = new glm::vec3(v0);
	mVertices[1] = new glm::vec3(v1);
	mVertices[2] = new glm::vec3(v2);
	mNormal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
	mMaterial = material;
	mType = TRIANGLE;
	
	glm::vec3 min(10000.0f);
	glm::vec3 max(-10000.0f);
	for(uint i = 0; i < 3; i++){
		for(uint j = 0; j < 3; j++){
			if((*mVertices[i])[j] < min[j]) min[j] = (*mVertices[i])[j];
			if((*mVertices[i])[j] > max[j]) max[j] = (*mVertices[i])[j];
		}
	}
	mAABB.setExtends(min, max);
}

Triangle::Triangle(glm::vec3 *v0, glm::vec3 *v1, glm::vec3 *v2, Material& material){
	isAllocated = false;
	mVertices[0] = v0;
	mVertices[1] = v1;
	mVertices[2] = v2;
	
	mNormal = glm::normalize(glm::cross(*v1 - *v0, *v2 - *v0));
	mMaterial = material;
	mType = TRIANGLE;
	
	glm::vec3 min(10000.0f);
	glm::vec3 max(-10000.0f);
	for(uint i = 0; i < 3; i++){
		for(uint j = 0; j < 3; j++){
			if((*mVertices[i])[j] < min[j]) min[j] = (*mVertices[i])[j];
			if((*mVertices[i])[j] > max[j]) max[j] = (*mVertices[i])[j];
		}
	}
	mAABB.setExtends(min, max);
}

Triangle::~Triangle(void){
	if(isAllocated){
		for(uint i = 0; i < 3; i++) delete mVertices[i];
	}
}

bool Triangle::intersect(Ray ray, float &t){
	glm::vec3 AC = *mVertices[2] - *mVertices[0];
	glm::vec3 AB = *mVertices[1] - *mVertices[0];
	glm::vec3 P = glm::cross(ray.dir, AC);
	float det = glm::dot(AB, P);
	if(det < 0.0f) return false;
	float invDet = 1.0f / det;
	glm::vec3 T = ray.r0 - *mVertices[0];
	glm::vec3 Q = glm::cross(T, AB);
	float t1 = glm::dot(AC, Q) * invDet;
	if(t1 > t || t1 < 0.0f) return false;
	float u = glm::dot(T, P) * invDet;
	if(u < 0.0f || u > 1.0f) return false;
	float v = glm::dot(ray.dir, Q) * invDet;
	if(v < 0.0f || u + v > 1.0f) return false;
	t = t1;
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////* Faster Ray Triangle Intersect with precomputed values. *///////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
// static inline int signf(float a){
	// if(a == 0) return -1;
	// return (a > 0.0f);
// }

// static inline int signf(float a){
	// if(((int&)a & 0x7FFFFFFF) == 0) return 0;
	// int r = 1;
	// r |= ((int&)a & 0x80000000);
	// return r;
// }

// Triangle::Triangle(glm::vec3 *v0, glm::vec3 *v1, glm::vec3 *v2, Material& material){
	// isAllocated = false;
	// mVertices[0] = v0;
	// mVertices[1] = v1;
	// mVertices[2] = v2;
	
	// glm::vec3 AC = *mVertices[2] - *mVertices[0];
	// glm::vec3 AB = *mVertices[1] - *mVertices[0];
	// N = glm::cross(AB, AC);
	// float area2 = glm::dot(N, N);
	// d = glm::dot((*mVertices[0]), N);
	// N1 = glm::cross(AC, N) / area2;
	// d1 = -glm::dot(N1, *mVertices[0]);
	// N2 = glm::cross(N, AB) / area2;
	// d2 = -glm::dot(N2, *mVertices[0]);
	
	// mNormal = (1.0f / (float)sqrt(area2)) * N;
	// mMaterial = material;
	// mType = TRIANGLE;
	
	// glm::vec3 min(10000.0f);
	// glm::vec3 max(-10000.0f);
	// for(uint i = 0; i < 3; i++){
		// for(uint j = 0; j < 3; j++){
			// if((*mVertices[i])[j] < min[j]) min[j] = (*mVertices[i])[j];
			// if((*mVertices[i])[j] > max[j]) max[j] = (*mVertices[i])[j];
		// }
	// }
	// mAABB.setExtends(min, max);
// }

// bool Triangle::intersect(Ray ray, float &t){

	// float det = glm::dot(ray.dir, N);
	// if(det == 0) return false;
	
	// float t1 = d - glm::dot(ray.r0, N);
	// if(signf(t1) != signf(det*t - t1)) return false;
	
	// glm::vec3 P = det * ray.r0 + t1 * ray.dir;
	
	// float u = glm::dot(P, N1) + det * d1;
	// if(signf(u) != signf(det - u)) return false;
	
	// float v = glm::dot(P, N2) + det * d2;
	// if(signf(v) != signf(det - u - v)) return false;
	// float ret = t1 / det;
	// if(ret < 0.0001f) return false;
	// t = ret;
	// return true;
// }
////////////////////////////////////////////////////////////////////////////////////////////////////////////

Polyhedron::Polyhedron(PolyhedronType const& polyType, glm::vec3 position, Material& material, glm::vec4 rotation, float scale){
	mType = POLYHEDRON;
	mMaterial = material;
	
	mVertices = polyType.mVertices;
	std::vector<glm::vec3>::iterator vIter;
	glm::vec3 axis = rotation.yzw();
	glm::mat3 rotMatrix = glm::mat3(glm::rotate(glm::mat4(1.0), rotation.x, axis));
	//Transform all vertices
	for(vIter = mVertices.begin(); vIter < mVertices.end(); vIter++){
		*vIter = scale * (*vIter);
		*vIter = rotMatrix * (*vIter);
		*vIter = *vIter + position;
	}
	
	// Assign triangles to vertices and find object AABB
	std::vector<glm::ivec3>::const_iterator fIter;
	mNTriangles = 0;
	glm::vec3 min(10000.0f);
	glm::vec3 max(-10000.0f);
	for(fIter = polyType.mTrVertIndices.begin(); fIter < polyType.mTrVertIndices.end(); fIter++){
		Triangle tempTriangle(&mVertices[fIter->x], &mVertices[fIter->y], &mVertices[fIter->z], mMaterial);
		mTriangles.push_back(tempTriangle);
		mNTriangles++;
		for(uint j = 0; j < 3; j++){
			if(tempTriangle.mAABB.bounds[0][j] < min[j]) min[j] = tempTriangle.mAABB.bounds[0][j];
			if(tempTriangle.mAABB.bounds[1][j] > max[j]) max[j] = tempTriangle.mAABB.bounds[1][j];
		}
	}
	mAABB.setExtends(min, max);
}

bool Polyhedron::intersect(Ray ray, float &t){
	bool retValue = false;
	for(uint i = 0; i < mNTriangles; i++){
		if(glm::dot(ray.dir, mTriangles[i].normal()) < 0.0f){
			if(mTriangles[i].intersect(ray, t)){
				retValue = true;
				mIntersTriangle = i;
				break;
			}
		}
	}
	return retValue;
}