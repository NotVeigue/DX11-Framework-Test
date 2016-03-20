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
	m_puyoQueue.transform.SetPosition(rightSide ? XMVectorSet(-QUEUE_WIDTH - QUEUE_PADDING, 13.0f, 0.0f, 0.0f) :
												  XMVectorSet(QUEUE_PADDING + 6.0f, 13.0f, 0.0f, 0.0f));
}


PuyoInstance::~PuyoInstance()
{

}

// ***************************************************************
// LOCAL HELPER FUNCTIONS
// ***************************************************************

int GetGridY(float y)
{
	return (int)floor(y);
}

bool CheckValidSpace(const XMFLOAT2& coords, const PuyoGrid& grid)
{
	int x = (int)coords.x;
	int y = GetGridY(coords.y);

	return grid.CheckOpenSpace(x, y);
}

bool CheckValidMove(int dx, int dy, const PuyoGrid& grid, PuyoUnit* unit)
{
	const XMFLOAT2& p = unit->GetPosition(0);
	const XMFLOAT2& h = unit->GetPosition(1);
	return CheckValidSpace(XMFLOAT2(p.x + dx, p.y + dy), grid) && CheckValidSpace(XMFLOAT2(h.x + dx, h.y + dy), grid);
}

void TryRotation(PuyoGrid& grid, PuyoUnit* unit)
{
	const XMFLOAT2& o = unit->GetOrientation();
	const XMFLOAT2& p = unit->GetPosition(0);
	XMFLOAT2 r(o.y, -o.x);

	while (r != o && !CheckValidSpace(p + r, grid))
	{
		if (r.x != 0 && CheckValidSpace(p - r, grid))
		{
			unit->Translate(-r.x, -r.y);
			unit->SetRotation(r);
			return;
		}

		float x = r.x;
		r.x = r.y;
		r.y = -x;
	}

	if (r == o)
		return;

	unit->SetRotation(r);
}

// ***************************************************************
// PRIVATE FUNCTIONS
// ***************************************************************

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

// ***************************************************************
// PUBLIC FUNCTIONS
// ***************************************************************

bool PuyoInstance::Update(double dt)
{
	// DEBUG
	if (InputManager::GetSingleton().IsKeyDown(KEY::SPACE))
	{
		m_gameState = PUYO_STATE::PAUSED;
		return true;
	}

	switch(m_gameState)
	{
		case PUYO_STATE::PLAYER_CONTROL:
		{
			// Move the current unit around
			float dx = 0.0f;
			float dy = dt * (m_controller->Fall() ? FALL_SPEED_FAST : FALL_SPEED);

			if (m_controller->MoveLeft() && CheckValidMove(-1, 0, m_puyoGrid, m_currentUnit))
			{
				dx = -1.0f;
			}
			else if (m_controller->MoveRight() && CheckValidMove(1, 0, m_puyoGrid, m_currentUnit))
			{
				dx = 1.0f;
			}
			else if (m_controller->Flip() /*&& CheckValidRotation(m_puyoGrid, m_currentUnit)*/)
			{
				TryRotation(m_puyoGrid, m_currentUnit);
			}

			m_currentUnit->Translate(dx, dy);

			const XMFLOAT2& p = m_currentUnit->GetPosition(0);
			const XMFLOAT2& h = m_currentUnit->GetPosition(1);

			Puyo* contactPuyo = nullptr;
			Puyo* otherPuyo = nullptr;
			const XMFLOAT2* contactPos = nullptr;
			const XMFLOAT2* otherPos = nullptr;
			if (!CheckValidSpace(p, m_puyoGrid))
			{
				contactPos = &p;
				otherPos = &h;
				contactPuyo = m_currentUnit->puyos[0];
				otherPuyo = m_currentUnit->puyos[1];
			}
			else if (!CheckValidSpace(h, m_puyoGrid))
			{
				contactPos = &h;
				otherPos = &p;
				contactPuyo = m_currentUnit->puyos[1];
				otherPuyo = m_currentUnit->puyos[0];
			}

			if (!contactPuyo)
				return true;

			m_puyoGrid.AddPuyo(contactPuyo, (int)contactPos->x, (int)GetGridY(contactPos->y) + 1);
			
			if (!CheckValidSpace(*otherPos, m_puyoGrid))
			{
				m_puyoGrid.AddPuyo(otherPuyo, (int)otherPos->x, (int)GetGridY(otherPos->y) + 1);
			}
			else
			{
				m_fallingPuyos.push_back(otherPuyo);
			}	

			m_gameState = PUYO_STATE::RESOLVING;
			break;
		}
		case PUYO_STATE::RESOLVING:
		{
			// Resolve falling puyos until none are left
			XMFLOAT2 pos;
			float dy = dt * FALL_SPEED;

			auto itr = m_fallingPuyos.begin();
			while (itr != m_fallingPuyos.end())
			{
				Puyo* p = *itr;
				XMStoreFloat2(&pos, p->transform.GetPosition());
				if (!CheckValidSpace(XMFLOAT2(pos.x, pos.y + dy), m_puyoGrid))
				{
					int x = (int)pos.x;
					int y = GetGridY(pos.y + dy) + 1;
					while (!m_puyoGrid.CheckOpenSpace(x, y))
						y++;

					m_puyoGrid.AddPuyo(p, x, y);
					m_fallingPuyos.erase(itr++);

					// Add check to go to game over if puyos are placed in game over zone
				}
				else
				{
					p->transform.Translate(XMVectorSet(0.0f, dy, 0.0f, 0.0f));
					++itr;
				}
			}

			if (m_fallingPuyos.size() > 0)
				return true;

			printf("Size was 0! \n");
			CheckForCombos();

			if (m_fallingPuyos.size() > 0)
				return true;

			m_currentUnit = m_puyoQueue.GetNextUnit();
			m_currentUnit->SetParent(&transform);
			m_currentUnit->SetPosition(PUYO_SPAWN_X, PUYO_SPWAN_Y);
			m_gameState = PUYO_STATE::PLAYER_CONTROL;

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
	return true;
}
