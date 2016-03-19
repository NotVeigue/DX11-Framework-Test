#pragma once
#include <queue>
#include "Puyo.h"

// Represents the falling pair of puyos the player manipulates. 
class PuyoUnit
{
private:
	DirectX::XMFLOAT2 m_orientation;

	// This may seem useless, but it makes querying positions much faster than using the puyos's transforms
	DirectX::XMFLOAT2 m_positions[2];

	// Updates the hanging puyo -- the puyo that does not serve as the pivot -- after the unit has translated or rotated.
	void UpdateHangingPuyo();

	void SetTransforms();

public:
	PuyoUnit();
	~PuyoUnit();

	Puyo* puyos[2];

	const DirectX::XMFLOAT2& GetPosition(int index) const;
	const DirectX::XMFLOAT2& GetOrientation() const;

	void SetParent(Transform* parent);

	void SetPosition(float x, float y);
	
	void Translate(float x, float y);

	void Rotate(bool counterClockwise);
};


// Represents the queue of puyos hanging to the side of a puyo grid. Handles dispensing 
class PuyoQueue
{
private:
	PuyoUnit m_puyoUnits[4];
	std::queue<PuyoUnit*> m_unitQueue;

	// TODO: Add functionality for animating the queued puyos.

public:
	PuyoQueue();
	~PuyoQueue();

	Transform transform;

	// TODO: Implement this. It should perform any animations that are necessary.
	void Update(double dt);

	PuyoUnit* GetNextUnit();
};

