#include "GameEngine.h"

// ----------------------------------------------------------------------------------
// Singleton Stuff
// ----------------------------------------------------------------------------------
template<> GameEngine* Singleton<GameEngine>::msSingleton = 0;
GameEngine& GameEngine::GetSingleton(void)
{
	assert(msSingleton);
	return *msSingleton;
}

GameEngine* GameEngine::GetSingletonPtr(void)
{
	return msSingleton;
}

// ----------------------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------------------

// The idea behind this implementation is that it allows me to control the order in which
// each system is created and destroyed. When used correctly, the user should be able to
// simply initialize a single GameEngine instance at the start of their game and delete
// that object when execution is complete.
GameEngine::GameEngine(int windowWidth, int windowHeight, bool vSync, bool windowed)
{
	m_windowsManager = new WindowsManager(windowWidth, windowHeight, vSync, windowed);
	m_inputManager = new InputManager();
	m_renderManager = new RenderManager();
}


GameEngine::~GameEngine()
{
	delete m_renderManager;
	delete m_inputManager;
	delete m_windowsManager;
}

// ToDo: Build a GameTimer class and change the callback to accept the elapsed time
bool GameEngine::Run(bool(*UpdateGame)(double dt))
{
	assert(UpdateGame);

	MSG msg;
	bool result = true;
	ZeroMemory(&msg, sizeof(MSG));

	// Loop until we are explicitly told to stop from within the loop
	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
		{
			break;
		}

		// Call the main gameloop function
		if (!UpdateGame(m_gameTimer.Update()))
		{
			break;
		}

		InputManager::GetSingleton().Update();
	}

	return result;
}
