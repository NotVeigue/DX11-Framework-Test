#pragma once
#include "Transform.h"
#include "PuyoGrid.h"
#include "PuyoQueue.h"
#include "PuyoController.h"
#include "PuyoValues.h"
#include <forward_list>

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
	Puyo* m_comboStaging[GRID_WIDTH * GRID_HEIGHT] = {};
	PuyoQueue m_puyoQueue;
	PuyoUnit* m_currentUnit;
	std::list<Puyo*> m_fallingPuyos;
	std::list<Puyo*> m_disappearingPuyos;

	PuyoController* m_controller;

	// Helper Functions
	int GetGridY(float y) const;
	bool CheckValidSpace(const DirectX::XMFLOAT2& coords) const;
	bool CheckValidMove(int dx, int dy) const;
	void TryRotation();
	void CheckForCombos();
	void RemoveComboPuyos(int count);
	void HandleFloatingPuyos();

	// State functions
	bool DoPlayerControl(double dt);
	bool DoResolve(double dt);

	// DEBUG
	/*double elapsed = 0.0;
	int x = 0, y = 0;
	double step = 0.5;*/

public:
	PuyoInstance(bool rightSide);
	~PuyoInstance();

	Transform transform;
	
	// Used for starting and ending play instances
	void Initialize(PuyoController* controller);
	void Cleanup();

	// Updates the puyo state. Returns true if successful, returns false if game over.
	bool Update(double dt);

	const PuyoGrid& GetGrid() const;

	// A pointer to a function in the outside game that allows this instance to send garbage puyos to a rival instance.
	void(*SendGarbage)(int instanceID, int garbageCount);
};

