#ifndef RTCAMERA_H
#define RTCAMERA_H


#include "common.h"
#include <glm/glm.hpp>
#include "ray.h"

class CameraBase{
public:
	virtual Ray shootRay(uint x, uint y, uint s)const = 0;
	uint getSamples(void){
		return mNSamples;
	};
protected:
	glm::vec3 mPosition;
	glm::vec3 mDirection;
	uint mNSamples;
};

class OrthographicCamera: public CameraBase{
public:
	OrthographicCamera(glm::vec3 position, glm::vec3 direction, float x, float y, uint width, uint height, uint nSamples);
	Ray shootRay(uint x, uint y, uint s)const;
private:
	float mSizeX, mSizeY; //Dimensions of pixel in world space
	uint mHalfWidth, mHalfHeight; //Half width and height pixel resolution
	glm::vec3 mUpVector, mRightVector;
};

class PinholeCamera: public CameraBase{
public:
	PinholeCamera(glm::vec3 position, glm::vec3 lookAt, float fov, float aspect, float zNear, uint width, uint height, uint nSamples);
	Ray shootRay(uint x, uint y, uint s)const;
private:
	float mSizeX, mSizeY; //Dimensions of pixel in world space
	uint mHalfWidth, mHalfHeight; //Half width and height pixel resolution
	float mZNear;
	glm::vec3 mUpVector, mRightVector;
};

#endif
