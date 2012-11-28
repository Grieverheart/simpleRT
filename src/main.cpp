#include "../include/raytracer.h"
#include <windows.h>
#include <FreeImage.h>
#include <iostream>
#include <fstream>
#include <sstream>

double PCFreq = 0.0;
__int64 CounterStart = 0;

void StartCounter(){
	LARGE_INTEGER li;
	if(!QueryPerformanceFrequency(&li))
		std::cout << "QueryPerformanceFrequency failed!" << std::endl;
		
	PCFreq = double(li.QuadPart) / 1000.0;
	
	QueryPerformanceCounter(&li);
	CounterStart = li.QuadPart;
}

double GetCounter(){
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return double(li.QuadPart - CounterStart) / PCFreq;
}


int main(int argc, char *argv[]){

	uint width, height;
	width = 1300;
	height = 1300;
	
	Scene myScene;
	
	Material material0;
	material0.color = colorRGBF(1.0f);
	material0.reflectivity = 0.0f;
	material0.diffusivity = 0.5f;
	material0.Sv = 0.0f;
	material0.Sp = 0.0f;
	
	Material material1;
	material1.color = colorRGBF(0.04f, 0.3f, 0.6f);
	// material1.color = colorRGBF(30, 144, 255);
	material1.reflectivity = 0.0f;
	material1.diffusivity = 1.0f;
	material1.Sv = 0.2f;
	material1.Sp = 128.0f;
	
	Material material2;
	material2.color = colorRGBF(0.3f, 0.6f, 0.04f);
	// material2.color = colorRGBF(100, 232, 100);
	material2.reflectivity = 0.0f;
	material2.diffusivity = 1.0f;
	material2.Sv = 0.2f;
	material2.Sp = 128.0f;
	
	std::vector<Material> mats;
	mats.push_back(material1);
	mats.push_back(material2);
	
	glm::vec3 lightPosition = glm::vec3(-30.0f, 30.0f, 30.0f);
	myScene.addAreaLight(lightPosition, -glm::normalize(lightPosition), 15.0f, colorRGBF(255, 255, 255), 64);
	
	
	std::ifstream file;
	file.open(argv[1]);
	if(!file){
		std::cout << "Error parsing file \"" << argv[1] << "\" by function " << "'" << __FUNCTION__ << "'" << std::endl;
		return false;
	}
	std::cout << "Parsing " << argv[1] << "." << std::endl;
	std::string line;
	
	std::getline(file, line);
	std::istringstream s(line);
	uint nPart;
	uint nTypes;
	s >> nPart;
	
	std::getline(file, line);
	s.seekg(0);
	s.clear();
	s.str(line);
	s >> nTypes;
	
	std::vector<uint> typeIDs;
	std::vector<float> typeScales;
	
	for(uint i = 0; i < nPart + 1; i++) std::getline(file, line);
	for(uint i = 0; i < nTypes; i++){
		std::getline(file, line);
		std::istringstream ss(line);
		std::string typeName;
		ss >> typeName;
		float scale;
		if(ss >> scale) typeScales.push_back(scale);
		else typeScales.push_back(1.0f);
		uint typeID;
		typeID = myScene.addPolyhedronType("obj/" + typeName + ".obj");
		typeIDs.push_back(typeID);
	}
	
	file.seekg(0);
	std::getline(file, line);
	std::getline(file, line);
	std::getline(file, line);
	
	float box[9];
	s.seekg(0);
	s.clear();
	s.str(line);
	for(uint i = 0; i < 9; i++) s >> box[i];
	
	PinholeCamera camera(glm::vec3(0.0f, (box[4] + box[5]), 2.0f * box[8]), glm::vec3(0.0f, 4.0f, 0.0f), 60.0f, (float)width / height, 1.0f, width, height, 2);
	// myScene.addPlane(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, -0.5f * (box[0] + box[1] + box[2]), 0.0f), material0);
	// myScene.addPlane(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, -0.5f * box[8]), material0);
	
	glm::vec3 translation = glm::vec3(
		-0.5f * (box[0] + box[1] + box[2]),
		-0.5f * (box[4] + box[5]),
		-0.5f * box[8]
	);
	
	for(uint i = 0; i < nPart; i++){
		std::getline(file, line);
		std::istringstream ss(line);
		uint typeID;
		ss >> typeID;
		glm::vec3 pos;
		ss >> pos.x >> pos.y >> pos.z;
		glm::vec4 rot;
		ss >> rot.x >> rot.y >> rot.z >> rot.w;
		myScene.addPolyhedron(typeIDs[typeID], pos + translation, mats[typeID], rot, typeScales[typeID]);
	}
	
	
	file.close();
	
	
	
	RayTracer raytracer(width, height);
	
	StartCounter();
	raytracer.Init(&myScene);
	std::cout << "Initialization: " << GetCounter() / 1000.0 << "s" << std::endl;
	
	StartCounter();
	raytracer.Trace(camera);
	std::cout << "Ray Tracing: " << GetCounter() / 1000.0 << "s" << std::endl;
	
	FreeImage_Initialise();
	
	FIBITMAP *bitmap = FreeImage_Allocate(width, height, 24);
	const uchar* data = raytracer.readBuffer();
	RGBQUAD color;
	for(uint i = 0; i < width; i++){
		for(uint j = 0; j < height; j++){
			color.rgbRed = data[3*i + 3*width*j + 0];
		    color.rgbGreen = data[3*i + 3*width*j + 1];
		    color.rgbBlue = data[3*i + 3*width*j + 2];
			FreeImage_SetPixelColor(bitmap, i, j, &color);
		}
	}
	
	FreeImage_Save(FIF_PNG, bitmap, "test.png", 0);
	
	FreeImage_DeInitialise();
	
	return 1;
}