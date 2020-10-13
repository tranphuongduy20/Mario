#include "Brick.h"

Brick::Brick(float x, float y) :CGameObject(x, y)
{
};

void Brick::Update(DWORD dt)
{
}
void Brick::Render()
{
	LPANIMATION ani;

	ani = CAnimations::GetInstance()->Get(510);

	ani->Render(x, y);
}