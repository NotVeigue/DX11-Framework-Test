#include "PuyoPuyoGamePCH.h"
#include "PuyoGame.h"

GameEngine* g_gameEngine;
PuyoGame* g_puyoGame;

bool Update(double dt);

void main()
{
	printf("Did this work?? \n");

	g_gameEngine = new GameEngine(800, 600, false, true);
	g_puyoGame = new PuyoGame();

	g_gameEngine->Run(Update);
	
	delete g_puyoGame;
	delete g_gameEngine;
}

bool Update(double dt)
{
	if (InputManager::GetSingleton().IsKeyDown(KEY::Q))
		return false;

	g_puyoGame->Update(dt);

	return true;
}