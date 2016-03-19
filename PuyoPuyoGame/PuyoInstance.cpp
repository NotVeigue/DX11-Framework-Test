#include "PuyoPuyoGamePCH.h"
#include "PuyoInstance.h"
#include "PuyoGame.h"
#include "InputManager.h"
#include "PuyoValues.h"
#include <math.h>

using namespace DirectX;

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

bool CheckValidRotation(PuyoGrid& grid, PuyoUnit* unit)
{
	const XMFLOAT2& o = unit->GetOrientation();
	const XMFLOAT2 p = unit->GetPosition(0);
	XMFLOAT2 r(o.y, -o.x);

	return CheckValidSpace(p, grid) && CheckValidSpace(XMFLOAT2(p.x + r.x, p.y + r.y), grid);
}

bool PuyoInstance::Update(double dt)
{
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
			else if (m_controller->Flip() && CheckValidRotation(m_puyoGrid, m_currentUnit))
			{
				m_currentUnit->Rotate(false);
			}

			m_currentUnit->Translate(dx, dy);

			const XMFLOAT2& p = m_currentUnit->GetPosition(0);
			const XMFLOAT2& h = m_currentUnit->GetPosition(1);
			Puyo* contactPuyo = nullptr;
			const XMFLOAT2* contactPos = nullptr;
			if (!CheckValidSpace(p, m_puyoGrid))
			{
				contactPos = &p;
				contactPuyo = m_currentUnit->puyos[0];
			}
			else if (!CheckValidSpace(h, m_puyoGrid))
			{
				contactPos = &h;
				contactPuyo = m_currentUnit->puyos[1];
			}

			if (!contactPuyo)
				return true;

			m_puyoGrid.AddPuyo(contactPuyo, (int)contactPos->x, (int)GetGridY(contactPos->y) + 1);
			m_fallingPuyos.push_back(contactPuyo == m_currentUnit->puyos[0] ? m_currentUnit->puyos[1] :
				m_currentUnit->puyos[0]);

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

			if (m_fallingPuyos.size() == 0)
			{
				printf("Size was 0! \n");
				m_currentUnit = m_puyoQueue.GetNextUnit();
				m_currentUnit->SetParent(&transform);
				m_currentUnit->SetPosition(PUYO_SPAWN_X, PUYO_SPWAN_Y);
				m_gameState = PUYO_STATE::PLAYER_CONTROL;
				//m_gameState = PUYO_STATE::PAUSED;
			}

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
