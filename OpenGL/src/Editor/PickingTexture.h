#pragma once

#include <GL/glew.h>
#include <iostream>

#include "../Graphics/Shading/Texture.h"
#include "../Math/maths.h"

class PickingTexture {
public:
	PickingTexture(int WindowWidth, int WindowHeight);
	~PickingTexture();

	void EnableWriting();
	void DisableWriting();

	struct PixelInfo {
		unsigned int ObjectID = 0;
		unsigned int PrimID = 0;
		unsigned int DrawID = 0;

		void print() {
			std::cout << "Object; " << ObjectID << ", Draw: " << DrawID << ", Prim: " << PrimID << std::endl;
		}
	};

	PixelInfo ReadPixel(unsigned int x, unsigned int y);

	Vec3 ReadWorldPosition(unsigned int x, unsigned int y);

private:
	unsigned int m_fbo = 0;
	unsigned int m_pickingTexture = 0;
	unsigned int m_depthTexture = 0; 
	unsigned int m_worldPosTexture = 0;

	
};