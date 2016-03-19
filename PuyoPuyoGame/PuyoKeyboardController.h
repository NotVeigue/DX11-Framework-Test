#pragma once
#include "InputManager.h"
#include "PuyoController.h"

class PuyoKeyboardController : public PuyoController
{
private:
	KEY m_moveLeft;
	KEY m_moveRight;
	KEY m_flip;
	KEY m_fall;

public:
	PuyoKeyboardController(KEY left, KEY right, KEY flip, KEY fall);
	~PuyoKeyboardController();

	bool MoveLeft() const final;
	bool MoveRight() const final;
	bool Flip() const final;
	bool Fall() const final;
};

