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
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			// Initialize each puyoUnit
			m_puyoUnits[i].puyos[j] = PuyoGame::GetSingleton().AllocPuyo();
			m_puyoUnits[i].puyos[j]->transform.SetParent(&transform);
			m_puyoUnits[i].puyos[j]->SetRandomColor();
		}

		// Add this unit to the queue
		m_unitQueue.push(&m_puyoUnits[i]);

		// Debug
		m_puyoUnits[i].SetPosition((float)i, 0.0f);
		//printf("Setting puyo position!");
	}

	// Maybe call a function here to position the puyos in the proper order?
}


PuyoQueue::~PuyoQueue()
{
}

PuyoUnit* PuyoQueue::GetNextUnit()
{
	PuyoUnit* next = m_unitQueue.front();
	PuyoUnit* back = m_unitQueue.back();

	// Generate Two New Puyos for the new last spot in the queue
	back->puyos[0] = PuyoGame::GetSingleton().AllocPuyo();
	back->puyos[1] = PuyoGame::GetSingleton().AllocPuyo();

	// Remove the current front of the queue and move it to the back
	m_unitQueue.pop();
	m_unitQueue.push(next);

	return next;
}
