#include "PuyoPuyoGamePCH.h"
#include "PuyoInstance.h"
#include "PuyoGame.h"
#include "InputManager.h"

using namespace DirectX;

PuyoInstance::PuyoInstance()
{
	Initialize();
}


PuyoInstance::~PuyoInstance()
{
	Shutdown();
}


void PuyoInstance::Initialize()
{
	// Add initialization code here!
	transform.SetScale(XMVectorSet(PUYO_SIZE, PUYO_SIZE, PUYO_SIZE, PUYO_SIZE));
	m_puyoQueue.transform.SetParent(&transform);
	m_puyoQueue.transform.SetPosition(XMVectorSet(10.0f, 13.0f, 0.0f, 0.0f));
}

void PuyoInstance::Shutdown()
{
	// Do we even need to do any shutdown here?
}

static double elapsed = 0.0;
static int x = 0, y = 0;
const double step = 0.5;
bool PuyoInstance::Update(double dt)
{
	elapsed += dt;
	if (elapsed > step)
	{
		elapsed -= step;

		// Add a new puyo to the grid or remove one if one is already there
		if (m_puyoGrid.GetPuyoAt(x % 6, y))
		{
			PuyoGame::GetSingleton().FreePuyo(m_puyoGrid.RemovePuyo(x % 6, y));
		}
		else
		{
			Puyo* newPuyo = PuyoGame::GetSingleton().AllocPuyo();
			if (!newPuyo)
				printf("NULL PUYO ALERT!!!");
			newPuyo->transform.SetParent(&transform);
			m_puyoGrid.AddPuyo(newPuyo, x % 6, y);
		}
		printf("%d \n", x);
		y = (++x / 6) % 13;
	}

	if (InputManager::GetSingleton().IsKeyDown(KEY::A))
	{
		PuyoGame::GetSingleton().AllocPuyo();
	}

	// Do a test here to start!
	return true;
}
