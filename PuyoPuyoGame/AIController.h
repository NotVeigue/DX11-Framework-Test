#pragma once
#include "PuyoController.h"
#include "LuaAIBridge.h"
#include <lua.hpp>

class AIController : public PuyoController
{
private:

	// TODO: Add lua instance and other stuff
	lua_State* m_luaState;

	enum CONTROL_FLAGS
	{
		MOVE_RIGHT,
		MOVE_LEFT,
		FLIP,
		FALL
	};

	int m_instanceID; // The number of the PuyoInstance this AI is tied to
	bool m_controlFlags[4] = { false, false, false, false };

	void ClearControlFlags() 
	{
		for (int i = 0; i < 4; i++) 
			m_controlFlags[i] = false;
	}

	// TODO: Add new functions for calling the LUA AI functions
	int m_nextMove[3]; // This is where the next move will be laid out (x, y, orientation)... Or I could just use the LUA stack... We will see when I get there.

	// IDEA: There is probably no need to actually make this two steps in the C++ code. I can probably do the whole update in LUA and just return which buttons are down.
	void DetermineMove();
	void DetermineInput();

public:
	AIController();
	~AIController();

	bool MoveLeft() const final;
	bool MoveRight() const final;
	bool Flip() const final;
	bool Fall() const final;

	// TODO: Add difficulty parameter to initialize
	void Initialize(int id);
	void Cleanup();
	void Update(double dt);
};

