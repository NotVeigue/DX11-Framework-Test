#include "PuyoPuyoGamePCH.h"
#include "PuyoQueue.h"
#include "PuyoGame.h"
#include <math.h>

// --------------------------------------------------------------------
// PUYO UNIT
// --------------------------------------------------------------------
PuyoUnit::PuyoUnit()
	: m_orientation(0.0f, 1.0f)
{

}

PuyoUnit::~PuyoUnit()
{

}

void PuyoUnit::UpdateHangingPuyo()
{
	m_positions[1].x = m_positions[0].x + m_orientation.x;
	m_positions[1].y = m_positions[0].y + m_orientation.y;
}

void PuyoUnit::SetTransforms()
{
	puyos[0]->transform.SetPosition(DirectX::XMVectorSet(m_positions[0].x, m_positions[0].y, 0.0f, 0.0f));
	puyos[1]->transform.SetPosition(DirectX::XMVectorSet(m_positions[1].x, m_positions[1].y, 0.0f, 0.0f));
}

void PuyoUnit::Initialize(Transform* parent, Puyo* pivot, Puyo* hanging)
{
	puyos[0] = pivot;
	puyos[1] = hanging;

	for (int i = 0; i < 2; i++)
	{
		puyos[i]->transform.SetParent(parent);
		puyos[i]->SetRandomColor();
	}

	SetRotation(0, 1);
}

const DirectX::XMFLOAT2& PuyoUnit::GetPosition(int index) const
{
	int i = min(max(index, 0), 1);
	return m_positions[i];
}

const DirectX::XMFLOAT2& PuyoUnit::GetOrientation() const
{
	return m_orientation;
}

void PuyoUnit::Translate(float x, float y)
{
	m_positions[0].x += x; m_positions[0].y += y;
	UpdateHangingPuyo();
	SetTransforms();
}

void PuyoUnit::SetParent(Transform* parent)
{
	for (int i = 0; i < 2; i++)
	{
		puyos[i]->transform.SetParent(parent);
	}
}

void PuyoUnit::SetPosition(float x, float y)
{
	m_positions[0].x = x; m_positions[0].y = y;
	UpdateHangingPuyo();
	SetTransforms();
}

void PuyoUnit::SetPosition(DirectX::XMFLOAT2& a)
{
	m_positions[0] = a;
	UpdateHangingPuyo();
	SetTransforms();
}

void PuyoUnit::SetRotation(float x, float y)
{
	m_orientation.x = x; m_orientation.y = y;
	UpdateHangingPuyo();
	SetTransforms();
}

void PuyoUnit::SetRotation(DirectX::XMFLOAT2& a)
{
	m_orientation = a;
	UpdateHangingPuyo();
	SetTransforms();
}

void PuyoUnit::Rotate(bool counterClockwise)
{
	float x = m_orientation.x;
	float y = m_orientation.y;
	m_orientation.x = counterClockwise ? -y : y;
	m_orientation.y = counterClockwise ? x : -x;

	UpdateHangingPuyo();
	SetTransforms();
}

// --------------------------------------------------------------------
// PUYO QUEUE
// --------------------------------------------------------------------

PuyoQueue::PuyoQueue()
{
	
}


PuyoQueue::~PuyoQueue()
{
}

void PuyoQueue::Initialize()
{
	for (int i = 0; i < 4; i++)
	{
		InitializeUnit(m_puyoUnits[i]);

		// Debug
		m_puyoUnits[i].SetPosition((float)i, 0.0f);
		//printf("Setting puyo position! %d \n", i);
	}

	// Maybe call a function here to position the puyos in the proper order?
}

void PuyoQueue::InitializeUnit(PuyoUnit& unit)
{
	unit.Initialize(&transform, 
		PuyoGame::GetSingleton().AllocPuyo(), 
		PuyoGame::GetSingleton().AllocPuyo());
}

PuyoUnit* PuyoQueue::GetNextUnit()
{
	PuyoUnit& next = m_puyoUnits[head];
	PuyoUnit& end = m_puyoUnits[head - 1 < 0 ? 3 : head - 1];

	// Generate Two New Puyos for the new last spot in the queue
	InitializeUnit(end);

	// Remove the current front of the queue and move it to the back
	head = (++head) % 4;

	// DEBUG
	for (int i = 0; i < 4; i++)
	{
		float offset = (float)((i + (4 - head)) % 4);
		m_puyoUnits[i].SetPosition(offset, 0.0f);
	}

	return &next;
}
