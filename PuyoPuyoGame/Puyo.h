#pragma once
#include "Transform.h"

enum PUYO_COLOR
{
	RED,
	GREEN,
	BLUE,
	YELLOW,
	PURPLE,
	CLEAR,
	PUYO_COLOR_COUNT,
	NONE
};

class Puyo
{
private:
	bool checked = false;

public:
	Puyo();
	~Puyo();

	Transform transform;
	PUYO_COLOR puyoColor;

	void SetRandomColor();

	// Getters and setters just in case I ever need to do anything else when checked is changed...
	void SetChecked(bool value);
	bool IsChecked() const;
};