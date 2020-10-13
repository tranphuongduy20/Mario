#include <algorithm>
#include <assert.h>
#include "Utils.h"

#include "Mario.h"
#include "Game.h"

#include "Goomba.h"
#include "Portal.h"
#include "Brick.h"

CMario::CMario(float x, float y) : CGameObject()
{
	level = MARIO_LEVEL_SMALL;
	untouchable = 0;
	SetState(MARIO_ANI_SMALL_IDLE);
	start_x = x; 
	start_y = y; 
	this->x = x; 
	this->y = y; 
}

void CMario::Update(DWORD dt, vector<LPGAMEOBJECT> *coObjects)
{
	

	// Calculate dx, dy 
	CGameObject::Update(dt);

	// Simple fall down
	vy += MARIO_GRAVITY*dt;
	//vx += 0.001 * dt;
	
	vector<LPCOLLISIONEVENT> coEvents;
	vector<LPCOLLISIONEVENT> coEventsResult;

	coEvents.clear();

	// reset untouchable timer if untouchable time has passed
	if ( GetTickCount() - untouchable_start > MARIO_UNTOUCHABLE_TIME) 
	{
		untouchable_start = 0;
		untouchable = 0;
	}
	if (isWaitingForAni && animation_set->at(state)->IsOver()) {
		isWaitingForAni = false;
	}

	// turn off collision when die 
	if (state!=MARIO_STATE_DIE)
		CalcPotentialCollisions(coObjects, coEvents);

	// No collision occured, proceed normally
	if (coEvents.size()==0)
	{
		x += dx; 
		y += dy;
	}
	else
	{
		float min_tx, min_ty, nx = 0, ny;
		float rdx = 0; 
		float rdy = 0;

		// TODO: This is a very ugly designed function!!!!
		FilterCollision(coEvents, coEventsResult, min_tx, min_ty, nx, ny, rdx, rdy);

		// how to push back Mario if collides with a moving objects, what if Mario is pushed this way into another object?
		//if (rdx != 0 && rdx!=dx)
		//	x += nx*abs(rdx); 
		
		// block every object first!
		x += min_tx*dx + nx*0.4f;
		y += min_ty*dy + ny*0.4f;

		if (nx!=0) vx = 0;
		if (ny!=0) vy = 0;


		//
		// Collision logic with other objects
		//
		for (UINT i = 0; i < coEventsResult.size(); i++)
		{
			LPCOLLISIONEVENT e = coEventsResult[i];

			if (dynamic_cast<CGoomba *>(e->obj)) // if e->obj is Goomba 
			{
				CGoomba *goomba = dynamic_cast<CGoomba *>(e->obj);

				// jump on top >> kill Goomba and deflect a bit 
				if (e->ny < 0)
				{
					if (goomba->GetState()!= GOOMBA_STATE_DIE)
					{
						goomba->SetState(GOOMBA_STATE_DIE);
						vy = -MARIO_JUMP_DEFLECT_SPEED;
					}
				}
				else if (e->nx != 0)
				{
					if (untouchable==0)
					{
						if (goomba->GetState()!=GOOMBA_STATE_DIE)
						{
							if (level > MARIO_LEVEL_SMALL)
							{
								level = MARIO_LEVEL_SMALL;
								StartUntouchable();
							}
							else 
								SetState(MARIO_STATE_DIE);
						}
					}
				}
			} // if Goomba
			else if (dynamic_cast<CBrick*>(e->obj)) // if e->obj is CBrick 
			{
				CBrick* brick = dynamic_cast<CBrick*>(e->obj);
			
				if (e->ny != 0)
				{
					isGround = true;
				}
			} // if Brick
			else if (dynamic_cast<CPortal *>(e->obj))
			{
				CPortal *p = dynamic_cast<CPortal *>(e->obj);
				CGame::GetInstance()->SwitchScene(p->GetSceneId());
			}
		}
	}

	// clean up collision events
	for (UINT i = 0; i < coEvents.size(); i++) delete coEvents[i];
	DebugOut(L" hghj %d \n", isWaitingForAni);
}

void CMario::Render()
{
	
	int alpha = 255;
	if (untouchable) alpha = 128;
	DebugOut(L"[xxxxxxxxxxxx] direction \n", nx);
	
		animation_set->at(state)->Render(nx, x, y, alpha);
	
	//RenderBoundingBox();
}

void CMario::SetState(int state)
{
	CGameObject::SetState(state);
	
	switch (state)
	{
	case MARIO_ANI_BIG_WALKING:
	case MARIO_ANI_SMALL_WALKING:
	case MARIO_ANI_RACCOON_WALKING:
		if (lastnx == -1) {
			SetState(MARIO_ANI_SMALL_SKID);
		}
		if (nx == 1) {
			vx = MARIO_WALKING_SPEED;
		}
		else if (nx == -1) {
			vx = -MARIO_WALKING_SPEED;
		}
		//Go();
		break;
	case MARIO_ANI_BIG_IDLE:
	case MARIO_ANI_SMALL_IDLE:
	case MARIO_ANI_RACCOON_IDLE:
		//DebugOut(L"dung im:%d \n", 1);
		vx = 0;
		if (vx * nx <= 0)
		{
			vx = 0;
			break;
		}vx -= MARIO_ACC * nx;
		break;

	/*case MARIO_STATE_JUMP:
		if (isGround) {
			vy = -MARIO_JUMP_SPEED_Y;
			isGround = false;
		}
		break;*/
	/*case MARIO_STATE_JUMP_MAX:
		vy = -MARIO_JUMP_SPEED_YMAX;
		break;*/
	case MARIO_ANI_BIG_JUMP:
	case MARIO_ANI_SMALL_JUMP:
	case MARIO_ANI_RACCOON_JUMP:
		if (isGround) {
			vy = -MARIO_JUMP_SPEED_Y;
			isGround = false;
		}
		//vy = -MARIO_JUMP_SPEED_YMAX;
		//speedUpY();
		break;
	case MARIO_ANI_BIG_RUN:
	case MARIO_ANI_SMALL_RUN:
	case MARIO_ANI_RACCOON_RUN:
		speedUp();
		break;
	case MARIO_ANI_BIG_SKID:
	case MARIO_ANI_SMALL_SKID:
	case MARIO_ANI_RACCOON_SKID:
		isWaitingForAni = true;	
		nx *= -1;
		break;
	case MARIO_ANI_BIG_CROUCH:
	case MARIO_ANI_RACCOON_CROUCH:
		vx = 0;
		break;
	case MARIO_ANI_RACCOON_SPIN:
		//vx = 0;
		break;
	}
	/*case MARIO_STATE_DIE:
		vy = -MARIO_DIE_DEFLECT_SPEED;
		break;*/
	
		
}

void CMario::GetBoundingBox(float &left, float &top, float &right, float &bottom)
{
	left = x;
	top = y; 

	if (level==MARIO_LEVEL_BIG)
	{
		right = x + MARIO_BIG_BBOX_WIDTH;
		bottom = y + MARIO_BIG_BBOX_HEIGHT;
	}
	else if (level == MARIO_LEVEL_SMALL)
	{
		right = x + MARIO_SMALL_BBOX_WIDTH;
		bottom = y + MARIO_SMALL_BBOX_HEIGHT;
	}
	else if (level == MARIO_LEVEL_RACCOON)
	{
		right = x + MARIO_RACCOON_BBOX_WIDTH;
		bottom = y + MARIO_RACCOON_BBOX_HEIGHT;
	}
}

/*
	Reset Mario status to the beginning state of a scene
*/
void CMario::ResetBIG()
{
	SetState(MARIO_STATE_IDLE);
	SetLevel(MARIO_LEVEL_BIG);
	SetPosition(start_x, start_y);
	SetSpeed(0, 0);
}

void CMario::ResetRACCOON()
{
	SetState(MARIO_ANI_RACCOON_IDLE);
	SetLevel(MARIO_LEVEL_RACCOON);
	SetPosition(start_x, start_y);
	SetSpeed(0, 0);
}

void CMario::speedUp()
{
	if (vx < MARIO_VMAX)
		vx += MARIO_SPEEEDUP*nx;
	else
		vx = MARIO_VMAX * nx;
	DebugOut(L"%.2f", vx);
}

void CMario::speedUpY(int state)
{
	if (vy < MARIO_JUMP_SPEED_YMAX)
		vy += MARIO_JUMP_Y * -1;
	else
		vy = MARIO_JUMP_SPEED_YMAX * -1;
	DebugOut(L"%.2f", vy);
}

//void CMario::Go()
//{
//	if (vx < MARIO_GO)
//		vx += MARIO_GOMAX * nx;
//	else
//		vx = MARIO_VMAX;
//	DebugOut(L"%.2f", vx);
//}

