#pragma once
#include <queue>
#include "Puyo.h"

class PuyoUnit
{
private:
	DirectX::XMFLOAT2 m_orientation;
	DirectX::XMFLOAT2 m_positions[2];

	void UpdateRotatingPuyo()
	{
		m_positions[1].x = m_positions[0].x + m_orientation.x;
		m_positions[1].y = m_positions[0].y + m_orientation.y;
	}

	void SetTransforms()
	{
		puyos[0]->transform.SetPosition(DirectX::XMVectorSet(m_positions[0].x, m_positions[0].y, 0.0f, 0.0f));
		puyos[1]->transform.SetPosition(DirectX::XMVectorSet(m_positions[1].x, m_positions[1].y, 0.0f, 0.0f));
	}

public:
	PuyoUnit()
		: m_orientation(0.0f, 1.0f)
	{}
	~PuyoUnit() {}

	Puyo* puyos[2];

	void SetPosition(float x, float y)
	{
		m_positions[0].x = x; m_positions[0].y = y;
		UpdateRotatingPuyo();
		SetTransforms();
	}
	
	void Translate(float x, float y)
	{
		m_positions[0].x += x; m_positions[0].y += y;
		UpdateRotatingPuyo();
		SetTransforms();
	}

	void Rotate(bool counterClockwise)
	{
		float x = m_orientation.x;
		float y = m_orientation.y;
		m_orientation.x = counterClockwise ? -y :  y;
		m_orientation.y = counterClockwise ?  x : -x;

		UpdateRotatingPuyo();
		SetTransforms();
	}
};

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

