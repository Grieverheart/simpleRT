#ifndef RTRAY_H
#define RTRAY_H

#include <glm/glm.hpp>

struct Ray{
	Ray(void){};
	Ray(glm::vec3 origin, glm::vec3 direction): r0(origin), dir(direction){};
	glm::vec3 r0;
	glm::vec3 dir;
};

#endif
