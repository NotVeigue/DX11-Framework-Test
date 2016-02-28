#include "PuyoPuyoGamePCH.h"
#include "PuyoKeyboardController.h"


PuyoKeyboardController::PuyoKeyboardController(KEY left, KEY right, KEY flip, KEY fall)
	: m_moveLeft(left)
	, m_moveRight(right)
	, m_flip(flip)
	, m_fall(fall)
{
}

PuyoKeyboardController::~PuyoKeyboardController()
{
}

bool PuyoKeyboardController::MoveLeft() const
{
	return InputManager::GetSingleton().IsKeyDown(m_moveLeft);
}

bool PuyoKeyboardController::MoveRight() const
{
	return InputManager::GetSingleton().IsKeyDown(m_moveRight);
}

bool PuyoKeyboardController::Flip() const
{
	return InputManager::GetSingleton().IsKeyDown(m_flip);
}

bool PuyoKeyboardController::Fall() const
{
	return InputManager::GetSingleton().IsKeyHeld(m_fall);
}
