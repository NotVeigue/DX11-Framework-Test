#include "PuyoPuyoGamePCH.h"
#include "PlayerController.h"


PlayerController::PlayerController(KEY left, KEY right, KEY flip, KEY fall)
	: m_moveLeft(left)
	, m_moveRight(right)
	, m_flip(flip)
	, m_fall(fall)
{
}

PlayerController::~PlayerController()
{
}

bool PlayerController::MoveLeft() const
{
	return InputManager::GetSingleton().IsKeyDown(m_moveLeft);
}

bool PlayerController::MoveRight() const
{
	return InputManager::GetSingleton().IsKeyDown(m_moveRight);
}

bool PlayerController::Flip() const
{
	return InputManager::GetSingleton().IsKeyDown(m_flip);
}

bool PlayerController::Fall() const
{
	return InputManager::GetSingleton().IsKeyHeld(m_fall);
}
