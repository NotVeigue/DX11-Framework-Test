#include "PuyoPuyoGamePCH.h"
#include "PuyoInstance.h"
#include "PuyoGame.h"
#include "InputManager.h"
#include "XMExtensions.h"
#include <math.h>

using namespace DirectX;
using namespace XMExtensions;

PuyoInstance::PuyoInstance(PuyoController* controller, bool rightSide)
	: m_controller(controller)
	, m_gameState(PUYO_STATE::RESOLVING)
{
	// Add initialization code here!
	transform.SetScale(XMVectorSet(PUYO_SIZE, PUYO_SIZE, PUYO_SIZE, PUYO_SIZE));
	m_puyoQueue.transform.SetParent(&transform);
	if(rightSide)
		m_puyoQueue.transform.Rotate(XMQuaternionRotationAxis(XMVectorSet(0.0, 1.0, 0.0, 0.0), XM_PI));
	m_puyoQueue.transform.SetPosition(rightSide ? XMVectorSet(-QUEUE_PADDING, 13.0f, 0.0f, 0.0f) :
												  XMVectorSet(QUEUE_PADDING + 6.0f, 13.0f, 0.0f, 0.0f));
}


PuyoInstance::~PuyoInstance()
{

}

// ***************************************************************
// PRIVATE FUNCTIONS
// ***************************************************************

int PuyoInstance::GetGridY(float y) const
{
	return (int)floor(y);
}

bool PuyoInstance::CheckValidSpace(const XMFLOAT2& coords) const
{
	int x = (int)coords.x;
	int y = GetGridY(coords.y);

	return m_puyoGrid.CheckOpenSpace(x, y);
}

bool PuyoInstance::CheckValidMove(int dx, int dy) const
{
	const XMFLOAT2& p = m_currentUnit->GetPosition(0);
	const XMFLOAT2& h = m_currentUnit->GetPosition(1);
	return CheckValidSpace(XMFLOAT2(p.x + dx, p.y + dy)) && CheckValidSpace(XMFLOAT2(h.x + dx, h.y + dy));
}

void PuyoInstance::TryRotation()
{
	const XMFLOAT2& o = m_currentUnit->GetOrientation();
	const XMFLOAT2& p = m_currentUnit->GetPosition(0);
	XMFLOAT2 r(o.y, -o.x);

	while (r != o && !CheckValidSpace(p + r))
	{
		if (r.x != 0 && CheckValidSpace(p - r))
		{
			m_currentUnit->Translate(-r.x, -r.y);
			m_currentUnit->SetRotation(r);
			return;
		}

		float x = r.x;
		r.x = r.y;
		r.y = -x;
	}

	if (r == o)
		return;

	m_currentUnit->SetRotation(r);
}

// Locate puyos in the grid left floating after removing combo puyos and remove them, adding them to the falling puyos list
void PuyoInstance::HandleFloatingPuyos()
{
	for (int i = 0; i < GRID_WIDTH; i++)
	{
		// Checking from bottom up. We start at the row above the bottom-most. Thist ensures that effcts of lower removals
		// bubble up through the stacks.
		for (int j = 1; j < GRID_HEIGHT; j++)
		{
			if (m_puyoGrid.CheckOpenSpace(i, j))
				continue;

			if (m_puyoGrid.CheckOpenSpace(i, j - 1))
			{
				m_fallingPuyos.push_back(m_puyoGrid.RemovePuyo(i, j));
			}
		}
	}
}

// Removes puyos in the combo list from the grid and deallocates them
void PuyoInstance::RemoveComboPuyos(int comboSize)
{
	XMFLOAT2 pos;
	for (int i = 0; i < comboSize; i++)
	{
		XMStoreFloat2(&pos, m_comboStaging[i]->transform.GetPosition());
		PuyoGame::GetSingleton().FreePuyo(m_puyoGrid.RemovePuyo((int)pos.x, (int)pos.y));
	}

	HandleFloatingPuyos();
}

// Identify any puyos that are part of a combo and add them to our staging area to be removed
void PuyoInstance::CheckForCombos()
{
	// Clear the combo staging array
	memset(m_comboStaging, 0, sizeof(m_comboStaging));

	int comboSize = m_puyoGrid.FindCombos(m_comboStaging);
	
	if (comboSize == 0)
		return;

	// TODO: Ideally, this won't happen yet. We will enter some fading stage where the puyos disappear before we
	//		 call RemoveComboPuyos.
	RemoveComboPuyos(comboSize);
}

bool PuyoInstance::DoPlayerControl(double dt)
{
	// Move the current unit around
	float dx = 0.0f;
	float dy = dt * (m_controller->Fall() ? FALL_SPEED_FAST : FALL_SPEED);


	// Move Left
	if (m_controller->MoveLeft() && CheckValidMove(-1, 0))
	{
		dx = -1.0f;
	}
	// Move Right
	else if (m_controller->MoveRight() && CheckValidMove(1, 0))
	{
		dx = 1.0f;
	}
	// Flip clockwise
	else if (m_controller->Flip())
	{
		TryRotation();
	}

	// Move the unit downward
	m_currentUnit->Translate(dx, dy);

	// Check to see if either puyo is in contact with another puyo or the floor of the grid
	const XMFLOAT2& p = m_currentUnit->GetPosition(0);
	const XMFLOAT2& h = m_currentUnit->GetPosition(1);

	Puyo* contactPuyo = nullptr;
	Puyo* otherPuyo = nullptr;
	const XMFLOAT2* contactPos = nullptr;
	const XMFLOAT2* otherPos = nullptr;
	if (!CheckValidSpace(p))
	{
		contactPos = &p;
		otherPos = &h;
		contactPuyo = m_currentUnit->puyos[0];
		otherPuyo = m_currentUnit->puyos[1];
	}
	else if (!CheckValidSpace(h))
	{
		contactPos = &h;
		otherPos = &p;
		contactPuyo = m_currentUnit->puyos[1];
		otherPuyo = m_currentUnit->puyos[0];
	}

	// If neither was in contact, we are done!
	if (!contactPuyo)
		return true;

	// Add the touching puyo to the grid
	m_puyoGrid.AddPuyo(contactPuyo, (int)contactPos->x, (int)GetGridY(contactPos->y) + 1);

	// Check to see if the other puyo is touching anything and handle it accordingly
	if (!CheckValidSpace(*otherPos))
	{
		m_puyoGrid.AddPuyo(otherPuyo, (int)otherPos->x, (int)GetGridY(otherPos->y) + 1);
	}
	else
	{
		m_fallingPuyos.push_back(otherPuyo);
	}

	// If contact was made, we need to resolve!
	m_gameState = PUYO_STATE::RESOLVING;

	return true;
}

bool PuyoInstance::DoResolve(double dt)
{
	// Resolve falling puyos until none are left
	XMFLOAT2 pos;
	float dy = dt * FALL_SPEED_FAST;

	// Check each puyo in the falling puyos list to see if it has made contact with another puyo and make if fall if not
	auto itr = m_fallingPuyos.begin();
	while (itr != m_fallingPuyos.end())
	{
		Puyo* p = *itr;
		XMStoreFloat2(&pos, p->transform.GetPosition());
		if (!CheckValidSpace(XMFLOAT2(pos.x, pos.y + dy)))
		{
			int x = (int)pos.x;
			int y = GetGridY(pos.y + dy) + 1;
			while (!m_puyoGrid.CheckOpenSpace(x, y))
				y++;

			m_puyoGrid.AddPuyo(p, x, y);
			m_fallingPuyos.erase(itr++);

			// TODO: Add check to go to game over if puyos are placed in game over zone
		}
		else
		{
			p->transform.Translate(XMVectorSet(0.0f, dy, 0.0f, 0.0f));
			++itr;
		}
	}

	// If there were still puyos left in the list after that check, we'll have to continue for another frame
	if (m_fallingPuyos.size() > 0)
		return true;

	// If all of the falling puyos were added to the grid, we need to check for combos
	CheckForCombos();

	// If there were new falling puyos added after the combo check, we will have to continue for another frame
	if (m_fallingPuyos.size() > 0)
		return true;

	// If we are out of falling puyos, it is time to return control to the player
	m_currentUnit = m_puyoQueue.GetNextUnit();
	m_currentUnit->SetParent(&transform);
	m_currentUnit->SetPosition(PUYO_SPAWN_X, PUYO_SPWAN_Y);
	m_gameState = PUYO_STATE::PLAYER_CONTROL;

	return true;
}

// ***************************************************************
// PUBLIC FUNCTIONS
// ***************************************************************

bool PuyoInstance::Update(double dt)
{
	bool result = true;

	switch(m_gameState)
	{
		case PUYO_STATE::PLAYER_CONTROL:
		{
			// DEBUG
			if (InputManager::GetSingleton().IsKeyDown(KEY::SPACE))
			{
				m_gameState = PUYO_STATE::PAUSED;
				return true;
			}

			result = DoPlayerControl(dt);
			break;
		}
		case PUYO_STATE::RESOLVING:
		{
			result = DoResolve(dt);
			break;
		}
		case PUYO_STATE::GAME_OVER:
		{
			// Make all of the puyos fall of the stage
			break;
		}
		case PUYO_STATE::PAUSED:
		{
			// Wait for input to tell us to unpause

			// DEBUG
			if (InputManager::GetSingleton().IsKeyDown(KEY::SPACE))
			{
				m_gameState = PUYO_STATE::PLAYER_CONTROL;
			}
			break;
		}
	}

	//elapsed += dt;
	//if (elapsed > step)
	//{
	//	elapsed -= step;

	//	// Add a new puyo to the grid or remove one if one is already there
	//	if (m_puyoGrid.GetPuyoAt(x % 6, y))
	//	{
	//		PuyoGame::GetSingleton().FreePuyo(m_puyoGrid.RemovePuyo(x % 6, y));
	//	}
	//	else
	//	{
	//		Puyo* newPuyo = PuyoGame::GetSingleton().AllocPuyo();
	//		if (!newPuyo)
	//			printf("NULL PUYO ALERT!!!");
	//		newPuyo->transform.SetParent(&transform);
	//		m_puyoGrid.AddPuyo(newPuyo, x % 6, y);
	//	}
	//	printf("%d \n", x);
	//	y = (++x / 6) % 13;
	//}

	//if (InputManager::GetSingleton().IsKeyDown(KEY::A))
	//{
	//	PuyoGame::GetSingleton().AllocPuyo();
	//}

	// Do a test here to start!
	return result;
}
