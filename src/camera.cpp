#include "../include/camera.h"

OrthographicCamera::OrthographicCamera(glm::vec3 position, glm::vec3 direction, float x, float y, uint width, uint height, uint nSamples){
	mNSamples = nSamples;
	mPosition = position;
	mSizeX = x / width;
	mSizeY = y / height;
	mDirection = glm::normalize(direction);
	mHalfWidth = width / 2;
	mHalfHeight = height / 2;
	mUpVector = glm::vec3(0.0f, 1.0f, 0.0f);
	mRightVector = -glm::normalize(glm::cross(mUpVector, mDirection));
	mUpVector = glm::normalize(glm::cross(mRightVector, mDirection));
}

Ray OrthographicCamera::shootRay(uint x, uint y, uint s)const{
	uint sx = s % mNSamples;
	uint sy = s / mNSamples;
	glm::vec3 origin = mPosition + mUpVector * (((float)y - (float)mHalfHeight + (float)sy / mNSamples) * mSizeY) + mRightVector * (((float)x - (float)mHalfWidth + (float)sx / mNSamples) * mSizeX);
	Ray tempRay = Ray(origin, mDirection);
	return tempRay;
}

PinholeCamera::PinholeCamera(glm::vec3 position, glm::vec3 lookAt, float fov, float aspect, float zNear, uint width, uint height, uint nSamples){
	mNSamples = nSamples;
	mDirection = glm::normalize(lookAt - position);
	mPosition = position;
	mZNear = zNear;
	float yhalf = tan((M_PI * fov * 0.5f) / 180.0f) * mZNear;
	float xhalf = aspect * yhalf;
	mHalfWidth = width / 2;
	mHalfHeight = height / 2;
	mSizeX = xhalf / mHalfWidth;
	mSizeY = yhalf / mHalfHeight;
	mUpVector = glm::vec3(0.0f, 1.0f, 0.0f);
	mRightVector = -glm::normalize(glm::cross(mUpVector, mDirection));
	mUpVector = glm::normalize(glm::cross(mRightVector, mDirection));
	// std::cout << mDirection.x << "\t" << mDirection.y << "\t" << mDirection.z << std::endl;
	// std::cout << mUpVector.x << "\t" << mUpVector.y << "\t" << mUpVector.z << std::endl;
	// std::cout << mRightVector.x << "\t" << mRightVector.y << "\t" << mRightVector.z << std::endl;
}

Ray PinholeCamera::shootRay(uint x, uint y, uint s)const{
	uint sx = s % mNSamples;
	uint sy = s / mNSamples;
	glm::vec3 direction = mUpVector * (((float)y - (float)mHalfHeight + (float)sy / mNSamples) * mSizeY) + mRightVector * (((float)x - (float)mHalfWidth + (float)sx / mNSamples) * mSizeX) + mZNear * mDirection;
	Ray tempRay = Ray(mPosition, glm::normalize(direction));
	return tempRay;
}