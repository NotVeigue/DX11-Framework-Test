#pragma once
#include "Singleton.h"
#include "InputManager.h"
#include "WindowsManager.h"
#include "RenderManager.h"
#include "GameTimer.h"

class GameEngine : public Singleton<GameEngine>
{
private:
	WindowsManager* m_windowsManager;
	InputManager*	m_inputManager;
	RenderManager*	m_renderManager;

	GameTimer		m_gameTimer;

public:
	GameEngine(int windowWidth, int windowHeight, bool vSync, bool windowed);
	~GameEngine();

	bool Run(bool(*UpdateGame)(double dt));

	static GameEngine& GetSingleton(void);
	static GameEngine* GetSingletonPtr(void);
};

