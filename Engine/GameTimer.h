#pragma once
class GameTimer
{
private:
	_int64 m_startTime;
	_int64 m_prevTime;
	_int64 m_frequency;
	double m_deltaTime;

public:
	GameTimer();
	~GameTimer();

	double Update();
	double GetDeltaTime();
};

