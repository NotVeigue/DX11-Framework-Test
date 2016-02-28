#include "PuyoPuyoGamePCH.h"
#include "PuyoQueue.h"
#include "PuyoGame.h"


PuyoQueue::PuyoQueue()
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			m_puyoUnits[i].puyos[j] = PuyoGame::GetSingleton().AllocPuyo();
			m_puyoUnits[i].puyos[j]->transform.SetParent(&transform);
			m_puyoUnits[i].puyos[j]->SetRandomColor();
		}
		// Debug
		m_puyoUnits[i].SetPosition((float)i, 0.0f);
		printf("Setting puyo position!");
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
