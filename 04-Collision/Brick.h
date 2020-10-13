#pragma once
#include "GameObject.h"

#define BRICK_BBOX_WIDTH  20
#define BRICK_BBOX_HEIGHT 20

class CBrick : public CGameObject
{
public:
	virtual void Render();
	virtual void GetBoundingBox(float &l, float &t, float &r, float &b);
};