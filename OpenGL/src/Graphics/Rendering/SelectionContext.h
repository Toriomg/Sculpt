#pragma once
#include "../../Scene/GameObject.h"

struct SelectionContext
{
	GameObject* pSelectedObject;
	unsigned int SelectedObjectID = 0;
	int SelectedTriangleID = -1;
	bool IsVertexSelected = false;
	Vec3 SelectedVertexWorldPos;
};