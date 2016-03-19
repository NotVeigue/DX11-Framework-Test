#pragma once

class PuyoController
{
public:
	virtual bool MoveLeft() const = 0;
	virtual bool MoveRight() const = 0;
	virtual bool Flip() const = 0;
	virtual bool Fall() const = 0;
};
