#pragma once
#include "Transform.h"
#include "PuyoGrid.h"
#include "PuyoQueue.h"
#include <forward_list>

class PuyoInstance
{
private:
	enum class PUYO_STATE
	{
		PLAYER_CONTROL,
		RESOLVING,
		GAME_OVER
	};
	PUYO_STATE m_gameState;

	PuyoGrid m_puyoGrid;

	PuyoQueue m_puyoQueue;
	PuyoUnit* m_currentUnit;
	std::forward_list<Puyo*> m_fallingPuyos;
	std::forward_list<Puyo*> m_disappearingPuyos;

public:
	PuyoInstance();
	~PuyoInstance();

	Transform transform;

	void Initialize();
	void Shutdown();

	// Updates the puyo state. Returns true if successful, returns false if game over.
	bool Update(double dt);

	// A pointer to a function in the outside game that allows this instance to send garbage puyos to a rival instance.
	void(*SendGarbage)(int instanceID, int garbageCount);
};

