#include "PuyoPuyoGamePCH.h"
#include "AIController.h"


AIController::AIController()
{
}


AIController::~AIController()
{
}

void AIController::Initialize(int id)
{
	m_instanceID = id;

	// Initialize the LUA instance, register functions, etc.
	m_luaState = lua_open();
	luaL_openlibs(m_luaState);
}

void AIController::Cleanup()
{
	// Shut down the LUA instance and stuff...
	lua_close(m_luaState);
}


bool AIController::MoveLeft() const
{
	return m_controlFlags[MOVE_LEFT];
}

bool AIController::MoveRight() const
{
	return m_controlFlags[MOVE_RIGHT];
}

bool AIController::Flip() const
{
	return m_controlFlags[FLIP];
}

bool AIController::Fall() const
{
	return m_controlFlags[FALL];
}

void AIController::Update(double dt)
{
	ClearControlFlags();

	// Ask AI to calculate a move location
	// (Push ID to stack, call AI update, pull results off the stack)
	
	// Ask AI to tell us what button to press using move location
	// (Push destination/orientation and elapsed time since last move to stack, call AI move, pull result off the stack)
}