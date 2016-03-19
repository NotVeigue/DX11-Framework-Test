#pragma once
#include "Transform.h"
#include "PuyoGrid.h"
#include "PuyoQueue.h"
#include <forward_list>
#include "PuyoController.h"

class PuyoInstance
{
private:
	enum class PUYO_STATE
	{
		PLAYER_CONTROL,
		RESOLVING,
		GAME_OVER,
		PAUSED
	};
	PUYO_STATE m_gameState;

	PuyoGrid m_puyoGrid;
	PuyoQueue m_puyoQueue;
	PuyoUnit* m_currentUnit;
	std::list<Puyo*> m_fallingPuyos;
	std::list<Puyo*> m_disappearingPuyos;

	PuyoController* m_controller;

	// DEBUG
	double elapsed = 0.0;
	int x = 0, y = 0;
	double step = 0.5;

public:
	PuyoInstance(PuyoController* controller, bool rightSide);
	~PuyoInstance();

	Transform transform;

	// Updates the puyo state. Returns true if successful, returns false if game over.
	bool Update(double dt);

	// A pointer to a function in the outside game that allows this instance to send garbage puyos to a rival instance.
	void(*SendGarbage)(int instanceID, int garbageCount);
};

