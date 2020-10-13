#pragma once
#include "GameObject.h"
class Brick : public CGameObject
{
public:
	Brick(float x, float y);
	void Update(DWORD dt);
	void Render();
};

