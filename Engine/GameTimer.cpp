#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "GameTimer.h"



GameTimer::GameTimer()
{
	LARGE_INTEGER li;

	QueryPerformanceCounter(&li);
	m_prevTime = m_startTime = li.QuadPart;

	QueryPerformanceFrequency(&li);
	m_frequency = li.QuadPart;

	m_deltaTime = 0.0;
}


GameTimer::~GameTimer()
{
}

double GameTimer::Update()
{
	LARGE_INTEGER li;

	QueryPerformanceCounter(&li);
	__int64 curTime = li.QuadPart;
	m_deltaTime = static_cast<double>(curTime - m_prevTime) / static_cast<double>(m_frequency);
	m_prevTime = curTime;

	return m_deltaTime;
}

double GameTimer::GetDeltaTime()
{
	return m_deltaTime;
}
