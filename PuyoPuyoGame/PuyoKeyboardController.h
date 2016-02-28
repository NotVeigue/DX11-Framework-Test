#pragma once
#include "InputManager.h"

class PuyoKeyboardController
{
private:
	KEY m_moveLeft;
	KEY m_moveRight;
	KEY m_flip;
	KEY m_fall;

public:
	PuyoKeyboardController(KEY left, KEY right, KEY flip, KEY fall);
	~PuyoKeyboardController();

	bool MoveLeft() const;
	bool MoveRight() const;
	bool Flip() const;
	bool Fall() const;
};

